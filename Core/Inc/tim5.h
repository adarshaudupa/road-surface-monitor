/*
 * tim5.h
 *
 *  Created on: May 14, 2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_TIM5_H_
#define INC_TIM5_H_

#include <stdint.h>

void TIM5_Init(void);
uint32_t TIM5_GetMicros(void);
void TIM5_IRQHandler(void);

#endif /* INC_TIM5_H_ */
