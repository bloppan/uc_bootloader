/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#define button2_Pin GPIO_PIN_0
#define button2_GPIO_Port GPIOA
#define CrrentSens_Pin GPIO_PIN_1
#define CrrentSens_GPIO_Port GPIOA
#define Temp1Sens_Pin GPIO_PIN_2
#define Temp1Sens_GPIO_Port GPIOA
#define Temp2Sens_Pin GPIO_PIN_3
#define Temp2Sens_GPIO_Port GPIOA
#define nCS_Pin GPIO_PIN_4
#define nCS_GPIO_Port GPIOA
#define LD_Green_Pin GPIO_PIN_0
#define LD_Green_GPIO_Port GPIOB
#define LD_Orange_Pin GPIO_PIN_1
#define LD_Orange_GPIO_Port GPIOB
#define LD_Red_Pin GPIO_PIN_2
#define LD_Red_GPIO_Port GPIOB
#define nCNVST_Pin GPIO_PIN_9
#define nCNVST_GPIO_Port GPIOA
#define powerSwitch_Pin GPIO_PIN_4
#define powerSwitch_GPIO_Port GPIOB
#define nPowerDown_Pin GPIO_PIN_8
#define nPowerDown_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
