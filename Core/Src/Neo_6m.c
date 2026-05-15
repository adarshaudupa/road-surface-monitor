#include "Neo_6m.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static UART_HandleTypeDef *gps_uart;

static uint8_t rx_byte;

static volatile uint8_t rx_buffer[GPS_RX_BUFFER_SIZE];

static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

static char line_buffer[GPS_LINE_BUFFER_SIZE];

static uint16_t line_index = 0;

static GPS_Data_t gps_data;

static void GPS_ParseLine(char *line);

static int32_t GPS_ConvertToMicroDegrees(char *raw,
                                         char direction);

void GPS_Init(UART_HandleTypeDef *huart)
{
    gps_uart = huart;

    HAL_UART_Receive_IT(gps_uart,
                        &rx_byte,
                        1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        uint16_t next_head;

        next_head =
            (rx_head + 1) % GPS_RX_BUFFER_SIZE;

        if(next_head != rx_tail)
        {
            rx_buffer[rx_head] = rx_byte;

            rx_head = next_head;
        }

        HAL_UART_Receive_IT(gps_uart,
                            &rx_byte,
                            1);
    }
}

void GPS_Process(void)
{
    while(rx_tail != rx_head)
    {
        char c;

        c = rx_buffer[rx_tail];

        rx_tail =
            (rx_tail + 1) % GPS_RX_BUFFER_SIZE;

        if(c == '\n')
        {
            line_buffer[line_index] = '\0';

            GPS_ParseLine(line_buffer);

            line_index = 0;
        }
        else
        {
            if(line_index <
               (GPS_LINE_BUFFER_SIZE - 1))
            {
                line_buffer[line_index++] = c;
            }
            else
            {
                line_index = 0;
            }
        }
    }
}

static void GPS_ParseLine(char *line)
{
    if(strncmp(line, "$GPGGA", 6) == 0)
    {
        char *token;

        uint8_t field = 0;

        char latitude_raw[16] = {0};

        char longitude_raw[16] = {0};

        char lat_dir = 0;

        char lon_dir = 0;

        token = strtok(line, ",");

        while(token != NULL)
        {
            switch(field)
            {
                case 2:

                    strcpy(latitude_raw, token);

                    break;

                case 3:

                    lat_dir = token[0];

                    break;

                case 4:

                    strcpy(longitude_raw, token);

                    break;

                case 5:

                    lon_dir = token[0];

                    break;

                case 6:

                    gps_data.fix_valid =
                        (atoi(token) > 0);

                    break;

                case 7:

                    gps_data.satellites =
                        atoi(token);

                    break;

                case 9:

                    gps_data.altitude_cm =
                        (int32_t)(atof(token) * 100.0f);

                    break;
            }

            token = strtok(NULL, ",");

            field++;
        }

        gps_data.latitude =
            GPS_ConvertToMicroDegrees(latitude_raw,
                                      lat_dir);

        gps_data.longitude =
            GPS_ConvertToMicroDegrees(longitude_raw,
                                      lon_dir);
    }
}

static int32_t GPS_ConvertToMicroDegrees(char *raw,
                                         char direction)
{
    float value;

    int degrees;

    float minutes;

    float result;

    int32_t scaled;

    value = atof(raw);

    degrees = (int)(value / 100);

    minutes = value - (degrees * 100);

    result = degrees + (minutes / 60.0f);

    scaled =
        (int32_t)(result * 1000000.0f);

    if(direction == 'S' ||
       direction == 'W')
    {
        scaled *= -1;
    }

    return scaled;
}

GPS_Data_t* GPS_GetData(void)
{
    return &gps_data;
}


