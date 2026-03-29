/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#include "stm32h7xx_hal.h"

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
#define DO_H_DI1_Pin GPIO_PIN_2
#define DO_H_DI1_GPIO_Port GPIOE
#define DO_H_COD_Pin GPIO_PIN_3
#define DO_H_COD_GPIO_Port GPIOE
#define PWM_H_IN1_Pin GPIO_PIN_4
#define PWM_H_IN1_GPIO_Port GPIOE
#define PWM_H_IN2_Pin GPIO_PIN_5
#define PWM_H_IN2_GPIO_Port GPIOE
#define DO_H_DI2_Pin GPIO_PIN_6
#define DO_H_DI2_GPIO_Port GPIOE
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define AN_T_AR_Pin GPIO_PIN_6
#define AN_T_AR_GPIO_Port GPIOF
#define DI_H_SF_Pin GPIO_PIN_7
#define DI_H_SF_GPIO_Port GPIOF
#define DI_LINHA15_Pin GPIO_PIN_8
#define DI_LINHA15_GPIO_Port GPIOF
#define AN_AC_PRES_Pin GPIO_PIN_9
#define AN_AC_PRES_GPIO_Port GPIOF
#define AN_KNOCK_SIGN_Pin GPIO_PIN_10
#define AN_KNOCK_SIGN_GPIO_Port GPIOF
#define AN_PEDAL2_Pin GPIO_PIN_0
#define AN_PEDAL2_GPIO_Port GPIOC
#define AN_T_AGUA_Pin GPIO_PIN_2
#define AN_T_AGUA_GPIO_Port GPIOC
#define AN_LAMBDA1_Pin GPIO_PIN_3
#define AN_LAMBDA1_GPIO_Port GPIOC
#define DO_LED1_Pin GPIO_PIN_0
#define DO_LED1_GPIO_Port GPIOA
#define AN_PEDAL1_Pin GPIO_PIN_3
#define AN_PEDAL1_GPIO_Port GPIOA
#define AN_ALTERN_Pin GPIO_PIN_4
#define AN_ALTERN_GPIO_Port GPIOA
#define AN_TPS2_Pin GPIO_PIN_5
#define AN_TPS2_GPIO_Port GPIOA
#define AN_TPS1_Pin GPIO_PIN_6
#define AN_TPS1_GPIO_Port GPIOA
#define AN_BATTERY_Pin GPIO_PIN_0
#define AN_BATTERY_GPIO_Port GPIOB
#define AN_LAMBDA2_Pin GPIO_PIN_1
#define AN_LAMBDA2_GPIO_Port GPIOB
#define DI_ROT_Pin GPIO_PIN_2
#define DI_ROT_GPIO_Port GPIOB
#define AN_MAP_Pin GPIO_PIN_11
#define AN_MAP_GPIO_Port GPIOF
#define DI_NOMI_Pin GPIO_PIN_14
#define DI_NOMI_GPIO_Port GPIOF
#define DI_MAXI_Pin GPIO_PIN_15
#define DI_MAXI_GPIO_Port GPIOF
#define DO_OUTEN_Pin GPIO_PIN_7
#define DO_OUTEN_GPIO_Port GPIOE
#define DO_DIN3_Pin GPIO_PIN_8
#define DO_DIN3_GPIO_Port GPIOE
#define DI_BUTTON3_Pin GPIO_PIN_10
#define DI_BUTTON3_GPIO_Port GPIOE
#define DO_GIN1_Pin GPIO_PIN_11
#define DO_GIN1_GPIO_Port GPIOE
#define DO_KNOCK_HOLD_Pin GPIO_PIN_12
#define DO_KNOCK_HOLD_GPIO_Port GPIOE
#define DO_GIN3_Pin GPIO_PIN_13
#define DO_GIN3_GPIO_Port GPIOE
#define DO_GIN2_Pin GPIO_PIN_14
#define DO_GIN2_GPIO_Port GPIOE
#define DO_SPD2_VEL_Pin GPIO_PIN_10
#define DO_SPD2_VEL_GPIO_Port GPIOB
#define DO_SPD1_VEL_Pin GPIO_PIN_11
#define DO_SPD1_VEL_GPIO_Port GPIOB
#define SPI_CS_KNOCK_Pin GPIO_PIN_12
#define SPI_CS_KNOCK_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define STLINK_RX_Pin GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD
#define DO_LED2_Pin GPIO_PIN_11
#define DO_LED2_GPIO_Port GPIOD
#define DO_LED4_Pin GPIO_PIN_12
#define DO_LED4_GPIO_Port GPIOD
#define DI_SPARK_Pin GPIO_PIN_13
#define DI_SPARK_GPIO_Port GPIOD
#define DI_SPEED_Pin GPIO_PIN_14
#define DI_SPEED_GPIO_Port GPIOD
#define DO_AQUEC_LAMBDA1_Pin GPIO_PIN_15
#define DO_AQUEC_LAMBDA1_GPIO_Port GPIOD
#define DI_BRAKE2_Pin GPIO_PIN_8
#define DI_BRAKE2_GPIO_Port GPIOG
#define SPI_CS_INJECTION_Pin GPIO_PIN_6
#define SPI_CS_INJECTION_GPIO_Port GPIOC
#define DO_BOB_IGN1_Pin GPIO_PIN_8
#define DO_BOB_IGN1_GPIO_Port GPIOC
#define DO_BOB_IGN2_Pin GPIO_PIN_9
#define DO_BOB_IGN2_GPIO_Port GPIOC
#define DO_BOMB_INJ_Pin GPIO_PIN_10
#define DO_BOMB_INJ_GPIO_Port GPIOC
#define DO_AC_RELE_Pin GPIO_PIN_11
#define DO_AC_RELE_GPIO_Port GPIOC
#define DI_AC_BUTTON_Pin GPIO_PIN_12
#define DI_AC_BUTTON_GPIO_Port GPIOC
#define DO_BOMB_FRIO_Pin GPIO_PIN_2
#define DO_BOMB_FRIO_GPIO_Port GPIOD
#define DI_FASE_Pin GPIO_PIN_3
#define DI_FASE_GPIO_Port GPIOD
#define DI_CLUTCH_Pin GPIO_PIN_4
#define DI_CLUTCH_GPIO_Port GPIOD
#define DO_AQUEC_LAMBDA2_Pin GPIO_PIN_5
#define DO_AQUEC_LAMBDA2_GPIO_Port GPIOD
#define DO_RELE_FRIO_Pin GPIO_PIN_6
#define DO_RELE_FRIO_GPIO_Port GPIOD
#define DI_BRAKE1_Pin GPIO_PIN_10
#define DI_BRAKE1_GPIO_Port GPIOG
#define DO_GIN0_Pin GPIO_PIN_12
#define DO_GIN0_GPIO_Port GPIOG
#define DO_DIN0_Pin GPIO_PIN_14
#define DO_DIN0_GPIO_Port GPIOG
#define PWM_COAL_Pin GPIO_PIN_5
#define PWM_COAL_GPIO_Port GPIOB
#define DO_DIN1_Pin GPIO_PIN_6
#define DO_DIN1_GPIO_Port GPIOB
#define DO_DIN2_Pin GPIO_PIN_7
#define DO_DIN2_GPIO_Port GPIOB
#define CAN_RX_Pin GPIO_PIN_8
#define CAN_RX_GPIO_Port GPIOB
#define CAN_TX_Pin GPIO_PIN_9
#define CAN_TX_GPIO_Port GPIOB
#define DI_BUTTON2_Pin GPIO_PIN_0
#define DI_BUTTON2_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
