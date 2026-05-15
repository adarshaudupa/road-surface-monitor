/*
 * hc-sr04.c
 *
 *  Created on: 17-Mar-2026
 *      Author: Adarsha Udupa
 */

#include "stm32f4xx.h"
#include "hc-sr04.h"
#include "tim2.h"
#include "uart2.h"


void HCSR04_Init(void)
{
	// PA6 - TRIG - output
	    RCC->AHB1ENR |= (1 << 0); 		 // enable GPIOA clock
	    GPIOA->MODER &= ~(3 << 12); 	 // clear mode bits for PA6
	    GPIOA->MODER |=  (1 << 12);      // output
	    GPIOA->OTYPER &= ~(1 << 6);      // push-pull
	    GPIOA->OSPEEDR |= (3 << 12);     // high speed
	    GPIOA->ODR &= ~(1 << 6);         // TRIG low initially

	    // PB6 - ECHO - input
	    // In HCSR04_Init(), change PB6 to PA0:
	    GPIOA->MODER &= ~(3 << 0);      // PA0
	    GPIOA->MODER |= (2 << 0);       // AF mode
	    GPIOA->AFR[0] &= ~(0xF << 0);
	    GPIOA->AFR[0] |= (1 << 0);      // AF1 for TIM5_CH1
	    GPIOA->PUPDR &= ~(3 << 0);      // no pull
}

void HCSR04_ReadDistance_cm(void)
{
    // 1. Send 10µs trigger pulse
    GPIOA->ODR |=  (1 << 6);         // TRIG high
    delay_us(10);
    GPIOA->ODR &= ~(1 << 6);         // TRIG low

    // 2. Wait for ECHO to go high
    uint32_t timeout = 100000; // 100ms timeout
    while (!(GPIOA->IDR & (1 << 0)) && timeout--); // wait for ECHO high
    if (timeout == 0) return;       // no echo — object out of range

    // 3. Measure pulse width
    uint32_t start = TIM5_GetMicros(); // record start time
    timeout = 100000; // 100ms timeout for ECHO to go low
    while ((GPIOA->IDR & (1 << 0)) && timeout--); // wait for ECHO low
    uint32_t duration = TIM5_GetMicros() - start; // calculate pulse duration

    // 4. Convert to cm
    if(duration>0)
    {
    uint32_t dist = duration / 58; // distance in cm (speed of sound: 343 m/s)
    uart_print_uint("Distance = ", dist); // Print distance over UART
    }
}

void delay_us(uint32_t us)
{
    for (volatile uint32_t i = 0; i < us * 14; i++); // ~1µs per iteration at 84MHz
}
