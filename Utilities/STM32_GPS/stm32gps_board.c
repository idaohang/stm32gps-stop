/**
  ******************************************************************************
  * @file    stm32_eval.c
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   STM32xx-EVAL abstraction layer. 
  *          This file should be added to the main application to use the provided
  *          functions that manage Leds, push-buttons, COM ports and low level 
  *          HW resources initialization of the different modules available on
  *          STM32 evaluation boards from STMicroelectronics.
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
  
/* Includes ------------------------------------------------------------------*/
#include "stm32gps_board.h"

/** @addtogroup Utilities
  * @{
  */ 
  
/** @defgroup STM32_EVAL 
  * @brief This file provides firmware functions to manage Leds, push-buttons, 
  *        COM ports and low level HW resources initialization of the different 
  *        modules available on STM32 Evaluation Boards from STMicroelectronics.
  * @{
  */ 

/** @defgroup STM32_EVAL_Abstraction_Layer
  * @{
  */ 
  
#ifdef USE_STM32_GPS_BOARD_VA
 #include "STM32_GPS_BOARD_VA/stm32_gps_board.c"
#elif defined USE_STM32_GPS_BOARD_VB
 #include "STM32_GPS_BOARD_VB/stm32_gps_board.c" 
#else 
 #error "Please select first the STM32 GPS board to be used (in stm32_eval.h)"
#endif

/** @defgroup STM32_EVAL_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_Private_Variables
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_Private_FunctionPrototypes
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM32_EVAL_Private_Functions
  * @{
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

/**
  * @}
  */ 
    
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/