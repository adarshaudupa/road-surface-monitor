/*
 * NEO_6M.h
 *
 *  Created on: Jun 4, 2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_NEO_6M_H_
#define INC_NEO_6M_H_

extern static volatile uint8_t  rx_buffer[GPS_RX_BUFFER_SIZE];
extern static volatile uint16_t rx_head = 0;
extern static volatile uint16_t rx_tail = 0;

extern static char     line_buffer[GPS_LINE_BUFFER_SIZE];
extern static uint16_t line_index = 0;

extern static GPS_Data_t        gps_data;
extern static volatile uint8_t  gps_data_updated  = 0;
extern static volatile uint32_t gps_overflow_count = 0;

#include <stdint.h>
void GPS_Init(void);
void USART1_IRQHandler(void);
void GPS_Process(void);
static void GPS_ParseLine(char *line);
void GPS_GetDataCopy(GPS_Data_t *dest);
uint8_t GPS_IsDataUpdated(void);
void GPS_ClearDataUpdatedFlag(void);
uint32_t GPS_GetOverflowCount(void);
uint32_t GPS_GetRxByteCount(void);


#endif /* INC_NEO_6M_H_ */
