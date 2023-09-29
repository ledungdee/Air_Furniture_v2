/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_G_Pin GPIO_PIN_0
#define LED_G_GPIO_Port GPIOA
#define LED_R_Pin GPIO_PIN_1
#define LED_R_GPIO_Port GPIOA
#define Buzzer_Pin GPIO_PIN_0
#define Buzzer_GPIO_Port GPIOB
#define EN_Valve_Pin GPIO_PIN_13
#define EN_Valve_GPIO_Port GPIOB
#define EN_DC_Pin GPIO_PIN_14
#define EN_DC_GPIO_Port GPIOB
#define PWM_Pin GPIO_PIN_8
#define PWM_GPIO_Port GPIOA
#define EN_AC_Pin GPIO_PIN_11
#define EN_AC_GPIO_Port GPIOA
#define HALL_1_EXTI_Pin GPIO_PIN_12
#define HALL_1_EXTI_GPIO_Port GPIOA
#define HALL_1_EXTI_EXTI_IRQn EXTI15_10_IRQn
#define HALL_2_Pin GPIO_PIN_15
#define HALL_2_GPIO_Port GPIOA
#define Btn_1_Pin GPIO_PIN_3
#define Btn_1_GPIO_Port GPIOB
#define Btn_1_EXTI_IRQn EXTI3_IRQn
#define Btn_2_Pin GPIO_PIN_4
#define Btn_2_GPIO_Port GPIOB
#define Btn_2_EXTI_IRQn EXTI4_IRQn
#define LED_Water_Alert_Pin GPIO_PIN_5
#define LED_Water_Alert_GPIO_Port GPIOB
#define Btn_3_Pin GPIO_PIN_6
#define Btn_3_GPIO_Port GPIOB
#define Btn_3_EXTI_IRQn EXTI9_5_IRQn
#define Btn_4_Pin GPIO_PIN_7
#define Btn_4_GPIO_Port GPIOB
#define Btn_4_EXTI_IRQn EXTI9_5_IRQn
#define Btn_5_Pin GPIO_PIN_8
#define Btn_5_GPIO_Port GPIOB
#define Btn_5_EXTI_IRQn EXTI9_5_IRQn
#define DHT22_Pin GPIO_PIN_9
#define DHT22_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
