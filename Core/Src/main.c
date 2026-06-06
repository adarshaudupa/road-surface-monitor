#include "stm32f4xx.h"
#include "uart2.h"
#include "uart3.h"
#include "NEO_6M.h"
#include "tim5.h"
#include "i2c1.h"
#include "lsm6ds3.h"
#include "hc-sr04.h"
#include "ESP32_comms.h"

#define SHOCK_DT_US         10000     // 100Hz sample
#define SHOCK_THRESH        3000      // LSM6DS3 shock threshold for pothole
#define EVENT_COOLDOWN_US  300000     // 300ms

static GPS_Data_t gps_data;

// Pothole event state
static uint32_t last_shock_us   = 0;
static uint32_t last_event_us   = 0;
static uint8_t  send_pending    = 0;  // 1 if need to send depth after US read

// Holds last event for ESP32 packet
static ESP32_Packet_t pkt;

// ──────────────────────────────────────────────────────────────────────────────
int main(void)
{
    // --- Clocks ---
    RCC->CR |= (1 << 0); while (!(RCC->CR & (1 << 1)));     // HSI on
    RCC->CFGR &= ~(3 << 0);  while ((RCC->CFGR & (3 << 2))); // SYSCLK = HSI

    // --- Peripheral init ---
    UART2_Init();
    UART3_Init();
    GPS_Init();           // wires up UART1/ISR/buffer
    TIM5_Init();          // for micros/hc-sr04
    I2C1_Init();
    HCSR04_Init();
    LSM6DS3_Init();

    UART2_SendString("==== POTHOLE DETECTION SYSTEM (INT-DRIVEN) ====\r\n");
    LSM6DS3_WHOAMI();
    imu_settle_gravity(200, 5); // 1s at 5ms/sample

    // --- Main loop ---
    while (1)
    {
        uint32_t now = TIM5_GetMicros();

        // --- GPS parser: Must run every loop ---
        GPS_Process();

        // --- IMU at 100Hz (~SHOCK_DT_US) ---
        if ((uint32_t)(now - last_shock_us) >= SHOCK_DT_US)
        {
            last_shock_us = now;

            int16_t shock = imu_vertical_shock();
            int16_t shock_abs = (shock < 0) ? -shock : shock;

            // Detection/trigger logic
            uint8_t road_flag = (shock_abs > SHOCK_THRESH) ? 1 : 0;  // 0=GOOD, 1=NOT GOOD

            // Only declare pothole if shock high and cooldown passed
            if (road_flag == 1 &&
                (uint32_t)(now - last_event_us) >= EVENT_COOLDOWN_US)
            {
                // Mark pothole event timestamp
                last_event_us = now;
                send_pending = 1;  // Wait for distance read

                // Copy GPS at event time
                GPS_GetDataCopy(&gps_data);

                // Prepare ESP32 packet
                memset(&pkt, 0, sizeof(pkt));
                pkt.timestamp_us = now;
                pkt.road_flag    = 1; // 1 = NOT GOOD (pothole)
                pkt.shock_lsb    = shock;
                pkt.dist_cm      = 0;            // updated after US read
                strncpy(pkt.lat_raw, gps_data.latitude_raw, sizeof(pkt.lat_raw) - 1);
                pkt.lat_dir      = gps_data.latitude_direction;
                strncpy(pkt.lon_raw, gps_data.longitude_raw, sizeof(pkt.lon_raw) - 1);
                pkt.lon_dir      = gps_data.longitude_direction;
                pkt.fix_quality  = gps_data.fix_quality;
                pkt.satellites   = gps_data.satellites;

                // Send event to ESP32 (depth=0 for now)
                ESP32_SendPacket(&pkt);

                UART2_SendString("*** POTHOLE EVENT: NOT GOOD ROAD ***\r\n");
                uart_print_int("shock = ", shock);

                // --- Immediately schedule an HC-SR04 distance sample ---
                // This line triggers ultrasonic; will handle asynchronously
                HCSR04_Start();    // (see below)
            }
            else if (road_flag == 0)
            {
                // Optionally, can throttle GOOD reports (send less often)
                // pkt ready for non-event report, if needed:
                // Set pkt fields, road_flag = 0, etc, and ESP32_SendPacket
                // (Here: only ever send GOOD packets if you want periodic status packets)
            }
        }

        // --- HC-SR04 Distance After Event (NON - polling, event driven)
        //
        // The best way is to use event/IRQ (no blocking), but your HCSR04 driver is currently polling!
        // Let's design an event-based version (see "HCSR04_Start" explanation below).
        //
        // If send_pending set, and HCSR04_IsDone() returns true -> get result and send.
        if (send_pending && HCSR04_IsDone()) // HCSR04 driver must set a done flag in ISR
        {
            uint16_t d_cm = HCSR04_GetLastCm(); // HC-SR04 driver provides last result

            pkt.dist_cm = d_cm;  // update event packet with measured depth (cm)
            ESP32_SendPacket(&pkt);

            // Optionally, print locally
            uart_print_uint("DISTANCE_CM = ", d_cm);

            send_pending = 0; // clear pending
        }

        // --- GPS print when updated ---
        if (GPS_IsDataUpdated())
        {
            GPS_GetDataCopy(&gps_data);
            UART2_SendString("UTC: "); UART2_SendString(gps_data.utc_time); UART2_SendString("\r\n");
            UART2_SendString("LAT: "); UART2_SendString(gps_data.latitude_raw); UART2_SendChar(gps_data.latitude_direction); UART2_SendString("\r\n");
            UART2_SendString("LON: "); UART2_SendString(gps_data.longitude_raw); UART2_SendChar(gps_data.longitude_direction); UART2_SendString("\r\n");
            UART2_SendString("ALT: "); UART2_SendString(gps_data.altitude_raw); UART2_SendString("\r\n\r\n");
            GPS_ClearDataUpdatedFlag();
        }
    }
}
