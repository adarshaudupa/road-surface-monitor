/*
 * lsm6ds3.c
 *
 *  Created on: May 14, 2026
 *      Author: Adarsha Udupa
 */

#include "lsm6ds3.h"
#include "i2c1.h"
#include "uart2.h"
#include "stm32f4xx_hal.h"

void LSM6DS3_Init(void)
{
 // Accel: 104Hz ODR, ±2g full scale
 I2C1_WriteByte(0x6A, 0x10, 0x40);  // CTRL1_XL

 // Gyro: 104Hz ODR, 245dps full scale
 I2C1_WriteByte(0x6A, 0x11, 0x40);  // CTRL2_G
}

void LSM6DS3_WHOAMI(void)
{
 uint8_t id = I2C1_ReadRegister(0x6A, 0x0F); // Read WHO_AM_I register
 uart_print_uint("LSM6DS3 WHO_AM_I = ", id); // Should print 106 for LSM6DS3
}

int16_t imu_calibrate_z_baseline(uint8_t samples)
{
 UART2_SendString("Calibrating...\r\n"); // Print calibration message
 int16_t accel_z_baseline = 0; // Reset baseline accumulator
 for (int i = 0; i < (samples + 1); i++) // Take specified number of samples to average for baseline
 {
  uint8_t low = I2C1_ReadRegister(0x6A, 0x2C); // Read low byte of Z-axis accel
  uint8_t high = I2C1_ReadRegister(0x6A, 0x2D); // Read high byte of Z-axis accel
  int16_t az = (int16_t)((high << 8) | low); // Combine bytes to form signed 16-bit value
  accel_z_baseline += az; // Accumulate for averaging
  HAL_Delay(10); // Short delay between samples
  }
 accel_z_baseline /= 10; // Average to get baseline
 uart_print_int("Baseline Z = ", accel_z_baseline); // Print baseline value
 return accel_z_baseline;
}

int16_t imu_read_accel_z(void)
{

 uint8_t low = I2C1_ReadRegister(0x6A, 0x2C); // Read low byte of Z-axis accel uint8_t high = I2C1_ReadRegister(0x6A, 0x2D); // Read high byte of Z-axis accel int16_t accel_z = (int16_t)((high << 8) | low); // Combine bytes to form signed 16-bit value return accel_z;}

