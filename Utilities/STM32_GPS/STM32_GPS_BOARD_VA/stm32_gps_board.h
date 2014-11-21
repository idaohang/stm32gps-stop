/**
  ******************************************************************************
  * @file    stm32_gps_board.h
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file contains definitions for STM3210C_EVAL's Leds, push-buttons
  *          COM ports, SD Card on SPI and sEE on I2C hardware resources.
  ******************************************************************************
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_GPS_BOARD_H
#define __STM32_GPS_BOARD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32gps_board.h"

#define GPS_PWR_CTRL_PORT	GPIOC	
#define GPS_PWR_CTRL_PIN	GPIO_Pin_11

#define GSM_PWR_CTRL_PORT	GPIOC
#define GSM_PWR_CTRL_PIN	GPIO_Pin_9

#define GSM_PWRKEY_PORT		GPIOC
#define GSM_PWRKEY_PIN		GPIO_Pin_10

#define LEDn                             4

#define LED1_PIN                         GPIO_Pin_3
#define LED1_GPIO_PORT                   GPIOB
#define LED1_GPIO_CLK                    RCC_APB2Periph_GPIOB
  
#define LED2_PIN                         GPIO_Pin_5
#define LED2_GPIO_PORT                   GPIOB
#define LED2_GPIO_CLK                    RCC_APB2Periph_GPIOB

#define LED3_PIN                         GPIO_Pin_14
#define LED3_GPIO_PORT                   GPIOB
#define LED3_GPIO_CLK                    RCC_APB2Periph_GPIOB

#define LED4_PIN                         GPIO_Pin_13
#define LED4_GPIO_PORT                   GPIOB
#define LED4_GPIO_CLK                    RCC_APB2Periph_GPIOB


/** @addtogroup STM3210C_EVAL_LOW_LEVEL_BUTTON
  * @{
  */  

/** @addtogroup STM3210C_EVAL_LOW_LEVEL_COM
  * @{
  */
#define COMn                             3

/**
 * @brief Definition for COM port1, connected to USART2 (USART2 pins remapped on GPIOD)
 */ 
#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCC_APB2Periph_USART1
#define EVAL_COM1_TX_PIN                 GPIO_Pin_9
#define EVAL_COM1_TX_GPIO_PORT           GPIOA
#define EVAL_COM1_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM1_RX_PIN                 GPIO_Pin_10
#define EVAL_COM1_RX_GPIO_PORT           GPIOA
#define EVAL_COM1_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM1_IRQn                   USART1_IRQn

/**
* @brief Definition for COM port1, connected to USART2 (USART2 pins remapped on GPIOD)
*/
#define EVAL_COM2                        USART2
#define EVAL_COM2_CLK                    RCC_APB1Periph_USART2
#define EVAL_COM2_TX_PIN                 GPIO_Pin_2
#define EVAL_COM2_TX_GPIO_PORT           GPIOA
#define EVAL_COM2_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM2_RX_PIN                 GPIO_Pin_3
#define EVAL_COM2_RX_GPIO_PORT           GPIOA
#define EVAL_COM2_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM2_IRQn                   USART2_IRQn

/**
* @brief Definition for COM port1, connected to USART2 (USART2 pins remapped on GPIOD)
*/
#define EVAL_COM3                        USART3
#define EVAL_COM3_CLK                    RCC_APB1Periph_USART3
#define EVAL_COM3_TX_PIN                 GPIO_Pin_10
#define EVAL_COM3_TX_GPIO_PORT           GPIOB
#define EVAL_COM3_TX_GPIO_CLK            RCC_APB2Periph_GPIOB
#define EVAL_COM3_RX_PIN                 GPIO_Pin_11
#define EVAL_COM3_RX_GPIO_PORT           GPIOB
#define EVAL_COM3_RX_GPIO_CLK            RCC_APB2Periph_GPIOB
#define EVAL_COM3_IRQn                   USART3_IRQn
 
  
/** @defgroup STM3210C_EVAL_LOW_LEVEL_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM3210C_EVAL_LOW_LEVEL_Exported_Functions
  * @{
  */ 
void STM_EVAL_LEDInit_Test(Led_TypeDef Led);
void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct); 
/**
  * @}
  */
  
#ifdef __cplusplus
}
#endif

#endif /* __STM32_GPS_BOARD_H */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */
    
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
