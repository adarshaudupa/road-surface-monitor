/*
 * hc-sr04.h
 *
 *  Created on: Jun 5, 2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_HC_SR04_H_
#define INC_HC_SR04_H_

volatile uint32_t echo_start;;
volatile uint32_t echo_width;
volatile uint8_t  edge_count;

void HCSR04_Init(void);
static void delay_us_spin(uint32_t us);
uint16_t HCSR04_ReadDistance_cm(uint32_t *t_echo_us);


#endif /* INC_HC_SR04_H_ */
