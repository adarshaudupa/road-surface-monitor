/*
 * tim3.c
 *
 *  Created on: May 14, 2026
 *      Author: Adarsha Udupa
 */


/*
 * tim2.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Adarsha Udupa
 */

#include "stm32f446xx.h"
#include "tim5.h"

volatile uint8_t tim5_flag = 0;


void timer_start(void) {
    TIM5->CR1 |= (1 << 0);
}

void timer_stop(void) {
    TIM5->CR1 &= ~(1 << 0);
    TIM5->CNT = 0;
}

static uint32_t tim5_get_timer_clock_hz(void) {
    uint32_t cfgr = RCC->CFGR;
    uint32_t ppre1 = (cfgr >> 10) & 0x7U;
    uint32_t hclk = SystemCoreClock;
    uint32_t apb1_clk;
    uint32_t tim_clk;

    if (ppre1 < 4U) {
        apb1_clk = hclk;
        tim_clk = apb1_clk;
    } else {
        apb1_clk = hclk / (1U << (ppre1 - 3U));
        tim_clk = apb1_clk * 2U;
    }

    return tim_clk;
}

uint32_t TIM5_GetMicros(void)
{
    uint32_t tim_clk = 84000000;
    uint32_t psc = TIM5->PSC + 1U;
    uint32_t cnt = TIM5->CNT;
    uint64_t timer_cnt_hz = (uint64_t)tim_clk / psc;

    if (timer_cnt_hz == 0U) {
        return 0U;
    }

    return (uint32_t)(((uint64_t)cnt * 1000000ULL) / timer_cnt_hz);
}
void TIM5_Init(void)
{
    // Enable TIM2 clock (bit 0 of APB1ENR)
    RCC->APB1ENR |= (1 << 3); // TIM5 is bit 3 in APB1ENR

    // Set prescaler
    TIM5->PSC = 7; // 84 MHz / (7 + 1) = ~10.5 MHz timer clock

    // Set auto-reload
    TIM5->ARR = 0xFFFFFFFF; // Max ARR for 32-bit timer, gives ~409.6 seconds overflow at 10.5 MHz

    TIM5->CCMR1 &= ~(3 << 0);
    TIM5->CCMR1 |= (1 << 0); // Capture/Compare 1 selection: 01 = CC1 channel is configured as input, IC1 is mapped on TI1

    TIM5->CCMR1 &= ~(0xF << 4);
    TIM5->CCMR1 |= (10 << 4); // Input capture 1 filter: 1010 = fSAMPLING = fCK_INT, N=8 (8 consecutive samples must be 1 to validate the capture)

    TIM5->CCER &= ~(1 << 1);
    TIM5->CCER |= (1 << 0); // Capture/Compare 1 output enable: 1 = Capture enabled on rising edge (CC1E=1, CC1P=0)

    // Enable capture interrupt (bit 1 of DIER)
    TIM5->DIER |= (1 << 1);

    // Enable TIM2 interrupt in NVIC
    NVIC_EnableIRQ(TIM5_IRQn);  // This one stays as-is (CMSIS function)
    NVIC_SetPriority(TIM5_IRQn, 2);  // Lower priority number = higher priority
}

uint32_t TIM5_GetMicros(void)
{
    uint32_t tim_clk = 84000000;
    uint32_t psc = TIM5->PSC + 1U;
    uint32_t cnt = TIM5->CNT;
    uint64_t timer_cnt_hz = (uint64_t)tim_clk / psc;

    if (timer_cnt_hz == 0U) {
        return 0U;
    }

    return (uint32_t)(((uint64_t)cnt * 1000000ULL) / timer_cnt_hz);
}

void TIM5_IRQHandler(void)
{
    if (TIM5->SR & (1 << 1)) {  // CC1IF flag
        TIM5->SR &= ~(1 << 1);  // Clear flag

        if (edge_count == 0) // Rising edge
        {
            // Rising edge
            echo_start = TIM5->CCR1; // Capture the timer value at rising edge
            TIM5->CCER |= (1 << 1);  // Set CC1P for falling edge next
            edge_count = 1; // Move to next state
        } else {
            // Falling edge
            echo_width = TIM->CCR1 - echo_start; // Calculate pulse width (handle timer overflow)
            TIM5->CCER &= ~(1 << 1);  // Clear CC1P for rising edge next
            edge_count = 0; // Reset state for next measurement
        }
    }
}
