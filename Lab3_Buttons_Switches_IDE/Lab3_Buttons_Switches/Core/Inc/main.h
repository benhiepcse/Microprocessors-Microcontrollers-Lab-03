/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_RED_1_Pin GPIO_PIN_1
#define LED_RED_1_GPIO_Port GPIOA
#define LED_YELLOW_1_Pin GPIO_PIN_2
#define LED_YELLOW_1_GPIO_Port GPIOA
#define LED_GREEN_1_Pin GPIO_PIN_3
#define LED_GREEN_1_GPIO_Port GPIOA
#define LED_RED_2_Pin GPIO_PIN_4
#define LED_RED_2_GPIO_Port GPIOA
#define LED_YELLOW_2_Pin GPIO_PIN_5
#define LED_YELLOW_2_GPIO_Port GPIOA
#define LED_GREEN_2_Pin GPIO_PIN_6
#define LED_GREEN_2_GPIO_Port GPIOA
#define Button1_Pin GPIO_PIN_7
#define Button1_GPIO_Port GPIOA
#define a_Pin GPIO_PIN_0
#define a_GPIO_Port GPIOB
#define b_Pin GPIO_PIN_1
#define b_GPIO_Port GPIOB
#define c_Pin GPIO_PIN_2
#define c_GPIO_Port GPIOB
#define k_Pin GPIO_PIN_10
#define k_GPIO_Port GPIOB
#define l_Pin GPIO_PIN_11
#define l_GPIO_Port GPIOB
#define m_Pin GPIO_PIN_12
#define m_GPIO_Port GPIOB
#define n_Pin GPIO_PIN_13
#define n_GPIO_Port GPIOB
#define EN3_Pin GPIO_PIN_14
#define EN3_GPIO_Port GPIOB
#define Button2_Pin GPIO_PIN_8
#define Button2_GPIO_Port GPIOA
#define Button3_Pin GPIO_PIN_9
#define Button3_GPIO_Port GPIOA
#define EN0_Pin GPIO_PIN_10
#define EN0_GPIO_Port GPIOA
#define EN1_Pin GPIO_PIN_11
#define EN1_GPIO_Port GPIOA
#define EN2_Pin GPIO_PIN_12
#define EN2_GPIO_Port GPIOA
#define d_Pin GPIO_PIN_3
#define d_GPIO_Port GPIOB
#define e_Pin GPIO_PIN_4
#define e_GPIO_Port GPIOB
#define f_Pin GPIO_PIN_5
#define f_GPIO_Port GPIOB
#define g_Pin GPIO_PIN_6
#define g_GPIO_Port GPIOB
#define h_Pin GPIO_PIN_7
#define h_GPIO_Port GPIOB
#define i_Pin GPIO_PIN_8
#define i_GPIO_Port GPIOB
#define j_Pin GPIO_PIN_9
#define j_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
