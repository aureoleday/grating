/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#define AMUX_A0_Pin GPIO_PIN_0
#define AMUX_A0_GPIO_Port GPIOC
#define AMUX_B0_Pin GPIO_PIN_1
#define AMUX_B0_GPIO_Port GPIOC
#define AMUX_C0_Pin GPIO_PIN_2
#define AMUX_C0_GPIO_Port GPIOC
#define AMUX_A1_Pin GPIO_PIN_3
#define AMUX_A1_GPIO_Port GPIOC
#define H_SYNC_INT_O_Pin GPIO_PIN_3
#define H_SYNC_INT_O_GPIO_Port GPIOA
#define H_RST_INT_O_Pin GPIO_PIN_5
#define H_RST_INT_O_GPIO_Port GPIOA
#define H_D_INT_O_Pin GPIO_PIN_6
#define H_D_INT_O_GPIO_Port GPIOA
#define H_SYNC_IB_Pin GPIO_PIN_7
#define H_SYNC_IB_GPIO_Port GPIOA
#define AMUX_B1_Pin GPIO_PIN_4
#define AMUX_B1_GPIO_Port GPIOC
#define AMUX_C1_Pin GPIO_PIN_5
#define AMUX_C1_GPIO_Port GPIOC
#define DIG_A_Pin GPIO_PIN_0
#define DIG_A_GPIO_Port GPIOB
#define DIG_B_Pin GPIO_PIN_1
#define DIG_B_GPIO_Port GPIOB
#define DIG_C_Pin GPIO_PIN_2
#define DIG_C_GPIO_Port GPIOB
#define TE_485_Pin GPIO_PIN_12
#define TE_485_GPIO_Port GPIOB
#define ADDR_RDN1_Pin GPIO_PIN_13
#define ADDR_RDN1_GPIO_Port GPIOB
#define AMUX_A2_Pin GPIO_PIN_6
#define AMUX_A2_GPIO_Port GPIOC
#define AMUX_B2_Pin GPIO_PIN_7
#define AMUX_B2_GPIO_Port GPIOC
#define AMUX_C2_Pin GPIO_PIN_8
#define AMUX_C2_GPIO_Port GPIOC
#define AMUX_A3_Pin GPIO_PIN_9
#define AMUX_A3_GPIO_Port GPIOC
#define H_RST_IB_Pin GPIO_PIN_8
#define H_RST_IB_GPIO_Port GPIOA
#define H_D_IB_Pin GPIO_PIN_15
#define H_D_IB_GPIO_Port GPIOA
#define AMUX_B3_Pin GPIO_PIN_10
#define AMUX_B3_GPIO_Port GPIOC
#define AMUX_C3_Pin GPIO_PIN_11
#define AMUX_C3_GPIO_Port GPIOC
#define AMUX_EN_Pin GPIO_PIN_12
#define AMUX_EN_GPIO_Port GPIOC
#define RESV_Pin GPIO_PIN_2
#define RESV_GPIO_Port GPIOD
#define ADDR_CUP0_Pin GPIO_PIN_3
#define ADDR_CUP0_GPIO_Port GPIOB
#define ADDR_CUP1_Pin GPIO_PIN_4
#define ADDR_CUP1_GPIO_Port GPIOB
#define ADDR_CDN0_Pin GPIO_PIN_5
#define ADDR_CDN0_GPIO_Port GPIOB
#define ADDR_CDN1_Pin GPIO_PIN_6
#define ADDR_CDN1_GPIO_Port GPIOB
#define ADDR_RUP0_Pin GPIO_PIN_7
#define ADDR_RUP0_GPIO_Port GPIOB
#define ADDR_RUP1_Pin GPIO_PIN_8
#define ADDR_RUP1_GPIO_Port GPIOB
#define ADDR_RDN0_Pin GPIO_PIN_9
#define ADDR_RDN0_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
