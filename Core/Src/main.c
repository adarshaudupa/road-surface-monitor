/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f4xx.h"
#include "clock.h"
#include "gpio.h"
#include "tim2.h"
#include "Neo_6m.h"
#include "uart2.h"
#include "hc-sr04.h"
#include "i2c1.h"
#include "lsm6ds3.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define POTHOLE_THRESHOLD 3000 //LSB drop from baseline
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint8_t tim2_flag = 0; // Flag set in TIM2 interrupt for LSM6DS3 timing
volatile uint8_t tim5_flag = 0; // Flag set in TIM5 interrupt for HC-SR04 timing
volatile uint32_t echo_start = 0, echo_width = 0; // Variables for HC-SR04 echo timing
volatile uint8_t edge_count = 0; // Count edges for HC-SR04
int16_t accel_z_baseline = 0; // Baseline Z-axis acceleration for pothole detection
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void); // System clock configuration function
static void MX_GPIO_Init(void); // GPIO initialization function
static void MX_USART1_UART_Init(void); // USART1 initialization function
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
typedef enum {
    STATE_ROAD_GOOD = 0,
    STATE_POTHOLE_DETECTED,
    STATE_BUMPY
} road_state_t;

road_state_t road_state = STATE_ROAD_GOOD;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init(); // Initialize the HAL Library

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  I2C1_Init(); // Initialize I2C1 for LSM6DS3
  UART2_Init(); // Initialize UART2
  TIM2_Init(); // Initialize TIM2 for timing for LSM6DS3
  TIM5_Init(); // Initialize TIM5 for timing for HC-SR04
  GPS_Init(&huart1); // Initialize GPS module with UART1
  HCSR04_Init(); // Initialize HC-SR04 ultrasonic sensors
  LSM6DS3_Init(); // Initialize LSM6DS3 IMU
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  UART2_SendString("=== POTHOLE DETECTION SYSTEM ===\r\n");
  LSM6DS3_WHOAMI();// LSM6DS3 WHOAMI check to verify communication

  accel_z_baseline = imu_calibrate_z_baseline(10); // Calibrate baseline Z-axis acceleration with 10 samples

      // Main loop
  uint32_t last_sample_us = 0; // Timestamp of last IMU sample
  uint32_t last_ultra_us    = 0; // Timestamp of last ultrasonic reading after pothole detection
  const uint32_t SAMPLE_DT  = 10000;   // desired sampling interval for IMU (100 Hz)
  const uint32_t ULTRA_DT   = 500000;  // desired interval between ultrasonic readings after pothole detection (500 ms)
  float accel_z_filt = 0.0f;   // Filtered Z-axis acceleration for smoothing
  const float alpha  = 0.2f;   // 0 < alpha < 1 for low-pass filter (0.2 gives good smoothing without too much lag)

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  uint32_t now = TIM5_GetMicros(); // Get current time in microseconds
	  if ((now - last_sample_us) >= SAMPLE_DT) // Check if it's time to take a new sample based on desired sampling interval (SAMPLE_DT)
	  {
	      // do accel_z_filt, z_diff, state machine as above
	      last_sample_us = now; // Update last sample time to current time after processing
	  }
	  if (road_state == STATE_POTHOLE_DETECTED && (now - last_ultra_us) >= ULTRA_DT) // If we detected a pothole and it's time to take a new ultrasonic reading based on desired interval (ULTRA_DT)
	  {
	      uint16_t d_cm = HCSR04_ReadDistance_cm(); // Read distance from HC-SR04 in cm
	      uart_print_uint("Distance_cm = ", d_cm); // Print distance reading to UART
	      last_ultra_us = now; // Update last ultrasonic reading time
	  }

	          // Read accel every 10ms
	          if ((now - last_sample_us) > 10000) // If more than 10ms has passed since last sample
	          {
	              // Accel Z
	        	  int16_t accel_z_raw = imu_read_accel_z(); // Combine high and low bytes to form signed 16-bit value
	              accel_z_filt = accel_z_filt + alpha * ((float)accel_z - accel_z_filt); // Apply low-pass filter to raw accel_z for smoothing
	              accel_z = (int16_t)accel_z_filt; // Use filtered value for pothole detection
	              int16_t z_diff  = accel_z_baseline - accel_z; // Calculate difference from baseline

	              // Pothole detection
	              int16_t z_diff = accel_z_baseline - accel_z; // Calculate difference from baseline
	              if (z_diff > POTHOLE_THRESHOLD) // If the drop exceeds the threshold, we have a pothole
	              {
	            	  road_state = STATE_POTHOLE_DETECTED;
	            	  UART2_SendString("*** POTHOLE DETECTED ***\r\n"); // Print pothole detection message
	                  uart_print_int("Z_diff = ", z_diff); // Print the difference from baseline

	                  // Trigger HC-SR04
	                  HCSR04_ReadDistance_cm(); // Read distance from HC-SR04 and print it
	              }
	              else if (z_diff > (POTHOLE_THRESHOLD / 2)) // If the drop is significant but not a pothole, we have a bumpy road
	              {
	            	  road_state = STATE_BUMPY;
	            	  UART2_SendString("*** BUMPY ROAD DETECTED *** \r\n"); // Print bumpy road message
	                  uart_print_int("Z_diff = ", z_diff); // Print the difference from baseline
	              }
	              else
	              {
	            	  road_state = STATE_ROAD_GOOD; // Road is good if difference is within acceptable range
	              }
	              last_sample_us = now; // Update last trigger time
	          }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
