# Road Surface Health Monitoring System — STM32 Bare-Metal Firmware

**Deterministic edge anomaly detection on ARM Cortex-M4 with multi-sensor fusion and ESP32 telemetry**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

---

## Overview

Bare-metal firmware for the STM32F446RE that autonomously detects road surface anomalies — potholes, bumps, uneven surfaces — in real time. No HAL, no RTOS, no `sprintf`. Pure register-level C running a 100Hz sensor fusion loop that never blocks.

When an anomaly is detected, the system captures IMU shock magnitude, ultrasonic depth, GPS coordinates, and a microsecond timestamp, then serializes and forwards the event packet to an ESP32 WiFi gateway over UART3.

**Core constraint:** The 10ms IMU sampling deadline must never be violated — not by UART transmission, not by ultrasonic measurement, not by GPS parsing.

---

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│                  STM32F446RE                        │
│                                                     │
│  ┌──────────────┐    ┌──────────────┐               │
│  │  100Hz IMU   │    │  GPS Parser  │               │
│  │  Loop (TIM5) │    │  (USART1 IRQ)│               │
│  └──────┬───────┘    └──────┬───────┘               │
│         │                   │                       │
│         ▼                   ▼                       │
│  ┌──────────────────────────────────┐               │
│  │        Event State Machine       │               │
│  │  shock > threshold + cooldown?   │               │
│  └──────────────┬───────────────────┘               │
│                 │ YES                               │
│                 ▼                                   │
│  ┌──────────────────────────┐                       │
│  │  HC-SR04 Trigger (async) │                       │
│  │  TIM5 Input Capture      │                       │
│  └──────────────┬───────────┘                       │
│                 │ done flag                         │
│                 ▼                                   │
│  ┌──────────────────────────┐                       │
│  │  ESP32_SendPacket()      │  ──► UART3 ──► ESP32  │
│  │  Custom serializer       │                       │
│  └──────────────────────────┘                       │
└─────────────────────────────────────────────────────┘
```

---

## Hardware

| Component | Interface | Pins | Notes |
|-----------|-----------|------|-------|
| STM32F446RE Nucleo | — | — | ARM Cortex-M4, 16MHz HSI |
| LSM6DS3 IMU | I2C1 (bare-metal) | PB8 (SCL), PB9 (SDA) | 104Hz ODR, ±2g |
| HC-SR04 Ultrasonic | TIM5 Input Capture | PA6 (TRIG), PA0 (ECHO/CH1) | AF2 |
| NEO-6M GPS | USART1 (IRQ RX) | PA9 (TX), PA10 (RX) | 9600 baud, NMEA GPGGA |
| ESP32 Gateway | USART3 | PC10 (TX), PC11 (RX) | 115200 baud |
| USART2 | Debug output | PA2 (TX), PA3 (RX) | 9600 baud |

---

## Firmware Modules

```
firmware-stm32-baremetal/Core/
├── Inc/
│   ├── lsm6ds3.h       # IMU driver — gravity LPF, vertical shock extraction
│   ├── hc-sr04.h       # Ultrasonic — non-blocking, event-driven
│   ├── NEO_6M.h        # GPS — USART1 IRQ ring buffer + NMEA parser
│   ├── esp32_comms.h   # Telemetry packet — custom CSV serializer
│   ├── i2c1.h          # Bare-metal I2C driver
│   ├── tim5.h          # TIM5 — 1µs free-run counter + input capture
│   ├── uart1.h         # USART1 (GPS RX, 9600 baud)
│   ├── uart2.h         # USART2 (debug TX/RX)
│   └── uart3.h         # USART3 (ESP32 TX, 115200 baud)
└── Src/
    ├── main.c          # 100Hz event loop, anomaly state machine
    ├── lsm6ds3.c       # IMU init, gravity estimation, shock detection
    ├── hc-sr04.c       # TRIG/ECHO driver, ISR callback
    ├── NEO_6M.c        # USART1 IRQ handler, GPGGA parser
    ├── esp32_comms.c   # Packet build + UART3 transmit
    ├── i2c1.c          # I2C1 init, read/write, multi-byte burst
    ├── tim5.c          # TIM5 init, GetMicros(), input capture IRQ
    ├── uart1/2/3.c     # UART peripheral drivers
    └── clock.c         # RCC clock tree read utilities
```

---

## Technical Deep Dive

### IMU: Gravity-Vector LPF + Vertical Shock Extraction

The core detection problem: the sensor can be mounted at any orientation, and `az_raw` alone is not a reliable shock signal. The solution is projecting dynamic acceleration along the gravity vector.

```c
// Gravity baseline tracked with a slow LPF (alpha=0.02 @ 100Hz → ~0.5s time constant)
g_x = g_x + alpha_g * ((float)ax_raw - g_x);
g_y = g_y + alpha_g * ((float)ay_raw - g_y);
g_z = g_z + alpha_g * ((float)az_raw - g_z);

// Dynamic accel = raw - gravity estimate
float dx = (float)ax_raw - g_x;
float dy = (float)ay_raw - g_y;
float dz = (float)az_raw - g_z;

// Project onto unit gravity vector → orientation-independent vertical shock
float gmag = sqrtf(g_x*g_x + g_y*g_y + g_z*g_z);
float a_vert = dx*(g_x/gmag) + dy*(g_y/gmag) + dz*(g_z/gmag);
```

At boot, `imu_settle_gravity()` runs 200 samples at 5ms intervals to converge the baseline before the main loop starts.

---

### HC-SR04: Non-Blocking Input Capture

The ultrasonic measurement is fully asynchronous — it never holds up the IMU loop.

```
HCSR04_Start()
  → PA6 TRIG high for 10µs (busy-wait on TIM5->CNT)
  → PA6 TRIG low
  → TIM5 CH1 input capture armed on rising edge

TIM5_IRQHandler()
  → Rising edge: record echo_start, flip to falling edge
  → Falling edge: echo_width = captured - echo_start
  → Call HCSR04_ISR_CaptureDone()
  → hcsr04_done = 1

Main loop:
  if (send_pending && HCSR04_IsDone())
    pkt.dist_cm = HCSR04_GetLastCm();  // echo_width / 58
    ESP32_SendPacket(&pkt);
```

TIM5 runs at 1µs/tick (PSC=15, ARR=0xFFFFFFFF), giving a 32-bit free-running microsecond counter for both timestamps and echo measurement.

---

### GPS: USART1 IRQ Ring Buffer + NMEA Parser

GPS bytes arrive at 9600 baud and are stored in a 256-byte ring buffer by `USART1_IRQHandler`. `GPS_Process()` is called each main loop iteration — it drains the ring buffer character by character, assembles lines, and parses `$GPGGA`/`$GNGGA` sentences using `strtok`.

The parsed result is stored in `GPS_Data_t` and copied into the event packet at the moment of anomaly detection — GPS state is always available, never blocking.

---

### Telemetry: Custom CSV Serializer

No `sprintf`, no `printf`, no standard library formatting. Custom integer-to-string converters avoid flash overhead and nondeterministic execution time.

```c
// Packet format transmitted to ESP32 over UART3:
// EVT,<timestamp_us>,<road_flag>,<shock_lsb>,<dist_cm>,<lat>,<lat_dir>,<lon>,<lon_dir>,<fix>,<sats>\r\n

// Example:
EVT,4823910,1,-3412,28,1234.5678,N,07734.2345,E,1,6
```

`road_flag=1` means anomaly detected. The ESP32 receives the CSV, parses it, and forwards to a cloud dashboard. The STM32 has no knowledge of WiFi or cloud — clean separation.

---

### I2C Bus-Lock Recovery

At init, a software reset is triggered via `CR1 SWRST` before configuring I2C1. This clears any bus-locked state left by an incomplete transaction from a prior reset — without it, the first `I2C1_ReadRegister` call after a dirty reset hangs indefinitely waiting for `SB` (start bit).

```c
RCC->APB1RSTR |= (1 << 21);   // Assert I2C1 reset
RCC->APB1RSTR &= ~(1 << 21);  // Release reset
```

---

### Event State Machine (main.c)

```c
// 1. Sample IMU at 100Hz
if ((now - last_shock_us) >= 10000)
{
    last_shock_us = now;
    int16_t shock = imu_vertical_shock();

    if (abs(shock) > SHOCK_THRESH &&
        (now - last_event_us) >= EVENT_COOLDOWN_US)  // 300ms debounce
    {
        last_event_us = now;
        send_pending = 1;

        // Snapshot GPS, build packet, start ultrasonic
        GPS_GetDataCopy(&gps_data);
        pkt.timestamp_us = now;
        pkt.shock_lsb    = shock;
        HCSR04_Start();
    }
}

// 2. Async: send packet once ultrasonic result is ready
if (send_pending && HCSR04_IsDone())
{
    pkt.dist_cm = HCSR04_GetLastCm();
    ESP32_SendPacket(&pkt);
    send_pending = 0;
}

// 3. GPS parsing runs every iteration (non-blocking)
GPS_Process();
```

The IMU sampling deadline is never broken — `HCSR04_Start()` only fires a 10µs pulse and arms the capture interrupt. The actual wait for the echo result happens in the background.

---

## Build and Flash

### Prerequisites
- STM32CubeIDE or ARM GCC toolchain
- ST-LINK (integrated on Nucleo board)

### STM32CubeIDE
```
1. File → Import → Existing Projects into Workspace
2. Select firmware-stm32-baremetal/
3. Project → Build All (Ctrl+B)
4. Run → Debug (F11)
```

### Command Line
```bash
make clean && make
st-flash write build/firmware.bin 0x8000000
```

### Debug Output

Connect a serial terminal to USART2 (PA2/PA3) at **9600 baud** to see:

```
==== POTHOLE DETECTION SYSTEM ====
LSM6DS3 WHO_AM_I = 106
IMU settling gravity...
g_x = -142
g_y = 31
g_z = 16320
Main Loop Start
*** POTHOLE EVENT DETECTED ***
EVENT SENT, DISTANCE_CM = 28
```

---

## Tuning Parameters

| Parameter | Location | Default | Effect |
|-----------|----------|---------|--------|
| `SHOCK_THRESH` | `main.c` | `3000` | LSB threshold for anomaly. Lower → more sensitive |
| `EVENT_COOLDOWN_US` | `main.c` | `300000` | Min µs between events. Prevents burst false positives |
| `SHOCK_DT_US` | `main.c` | `10000` | IMU sample interval (100Hz) |
| `alpha_g` | `lsm6ds3.c` | `0.02f` | Gravity LPF speed. Lower → slower baseline tracking, better pothole isolation |
| `alpha_dyn` | `lsm6ds3.c` | `0.25f` | Dynamic accel smoothing |

---

## Known Limitations

| Issue | Impact |
|-------|--------|
| UART3 TX is blocking | ESP32 packet transmission holds up the main loop briefly. Acceptable at 115200 baud for short CSV packets; DMA TX would eliminate this |
| No GPS fix validation | Packets are sent even with `fix_quality=0`. Add a fix check before `send_pending=1` if GPS reliability is critical |
| Single-threshold detection | A fixed LSB threshold doesn't adapt to road speed or sensor noise floor. A dynamic threshold or RMS window would reduce false positives |

---

## References

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [LSM6DS3 Datasheet](https://www.st.com/resource/en/datasheet/lsm6ds3.pdf)
- [HC-SR04 Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
- [NEO-6M GPS Module Datasheet](https://content.u-blox.com/sites/default/files/products/documents/NEO-6_DataSheet_%28GPS.G6-HW-09005%29.pdf)

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

## Author

**Adarsha Udupa Baikady**  
Undergraduate | Electronics & Instrumentation Engineering  
Focus: Bare-Metal Embedded Firmware

- GitHub: [@adarshaudupa](https://github.com/adarshaudupa)
- LinkedIn: [adarsha-udupa-baikady](https://www.linkedin.com/in/adarsha-udupa-baikady-327a54219)

---

**No HAL. No RTOS. No sprintf. Just registers, reference manuals, and a 10ms deadline.**
