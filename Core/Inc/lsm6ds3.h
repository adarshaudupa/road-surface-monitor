/*
 * lsm6ds3.h
 *
 *  Created on: May 14, 2026
 *      Author: Adarsha Udupa
 */

#ifndef INC_LSM6DS3_H_
#define INC_LSM6DS3_H_

void LSM6DS3_Init(void);
void LSM6DS3_WHOAMI(void);
int16_t imu_calibrate_z_baseline(uint8_t samples);
int16_t imu_read_accel_z(void);

#endif /* INC_LSM6DS3_H_ */
