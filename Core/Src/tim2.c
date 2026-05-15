/*
 * tim2.c
 *
 *  Created on: Feb 13, 2026
 *      Author: Adarsha Udupa
 */

#include "stm32f446xx.h"
#include "tim2.h"
#include "clock.h"
#include "gpio.h"

volatile uint8_t tim2_flag = 0;


void timer_start(void) {
    TIM2->CR1 |= (1 << 0);
}

void timer_stop(void) {
    TIM2->CR1 &= ~(1 << 0);
    TIM2->CNT = 0;
}


void TIM2_Init(void) {
    // Enable TIM2 clock (bit 0 of APB1ENR)
    RCC->APB1ENR |= (1 << 0);
    // Set prescaler
    TIM2->PSC = 15;  // 16 MHz / (15 + 1) = 1 MHz timer clock → 1µs per tick
    // Set auto-reload
    TIM2->ARR = 0xFFFFFFFF;    // Max ARR for free-running counter

    TIM2->CR1 |= (1 << 0); // CEN = 1 (start counter)
}

uint32_t TIM2_GetMicros(void)
{
    return TIM2->CNT; // Return current timer count (microseconds since start)
}

void delay_us(uint32_t us)
{
    uint32_t start = TIM2->CNT; // Capture start time
    while ((TIM2->CNT - start) < us); // Wait until desired time has passed
}







