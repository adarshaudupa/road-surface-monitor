#ifndef INC_NEO_6M_H_
#define INC_NEO_6M_H_

#include "main.h"

#include <stdint.h>
#include <stdbool.h>

#define GPS_RX_BUFFER_SIZE     512
#define GPS_LINE_BUFFER_SIZE   128

typedef struct
{
    int32_t latitude;
    int32_t longitude;

    int32_t altitude_cm;

    uint8_t satellites;

    bool fix_valid;

} GPS_Data_t;

void GPS_Init(UART_HandleTypeDef *huart);

void GPS_Process(void);

GPS_Data_t* GPS_GetData(void);

#endif
