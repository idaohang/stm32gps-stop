/**
  ******************************************************************************
  * @file    USART/Printf/stm32f10x_it.h
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F10x_IT_API_H
#define __STM32F10x_IT_API_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

uint32_t sysTickGet(void);
uint32_t sysTickPerSec(void);
void sysTickDelay(uint32_t tickCnt);

typedef void (*UART_INT_HANDLER)(uint32_t com, uint32_t arg);
void USART_IRQHandler_register(uint32_t com, UART_INT_HANDLER handler, uint32_t arg);

#endif /* __STM32F10x_IT_API_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
