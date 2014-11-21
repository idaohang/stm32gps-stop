#include "stm32f10x.h"
#include "stm32f10x_it_api.h"
#include "stm32gps_board.h"
#include "stm32gps_config.h"
#include <stdio.h>
#include <string.h>

ErrorStatus HSEStartUpStatus;


/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;


/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay_10ms(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/**
  * @brief  Delay Timer ms.
  * @param  Timer: specifies the delay time length, in milliseconds.
  * @retval None
  */
void delay_ms(uint32_t Timer)
{
    volatile uint32_t i=0;
    uint32_t tickPerMs = SystemCoreClock/1000;

    while(Timer)
    {
        i=tickPerMs/6-1;
        while(i--);
        Timer--;
    }
}

/**
  * @brief  Configures the SysTick to generate an interrupt each 10 ms.
  * @param  None
  * @retval None
  */
void stm32gps_sys_tick_cfg(void)
{
	/* SysTick interrupt each 10 ms */
	if (SysTick_Config(SystemCoreClock / SYS_TICK_PER_SEC))
    {
      /* Capture error */
      while (1);
    }

	/* Set SysTick Priority to 3 */
  NVIC_SetPriority(SysTick_IRQn, 0x0C);
}

/**
  * @brief  Configures the RCC.
  * TIM_Prescaler = 1KHz; TIM_Period = 60s
  * @param  None
  * @retval None
  */
void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
	//RCC_DeInit();    //RCC system reset 
    RCC_HSEConfig(RCC_HSE_ON);  // Enable HSE 
    HSEStartUpStatus = RCC_WaitForHSEStartUp();  // Wait till HSE is ready 
    if(HSEStartUpStatus == SUCCESS)
   {
        /*设置低速AHB时钟（PCLK1）*/    
		RCC_PCLK1Config(RCC_HCLK_Div4);   //RCC_HCLK_Div4——APB1时钟= HCLK / 4
    }
}

/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SYSCLKConfig_STOP(void)
{
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  HSEStartUpStatus = RCC_WaitForHSEStartUp();

  if(HSEStartUpStatus == SUCCESS)
  {

#ifdef STM32F10X_CL
    /* Enable PLL2 */ 
    RCC_PLL2Cmd(ENABLE);

    /* Wait till PLL2 is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET)
    {
    }

#endif

    /* Enable PLL */ 
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
    {
    }

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while(RCC_GetSYSCLKSource() != 0x08)
    {
    }
  }
}


/**
  * @brief  Configures RTC clock source and prescaler.
  * @param  None
  * @retval None
  */
void RTC_Configuration(void)
{
    /* RTC clock source configuration ----------------------------------------*/
	/* Allow access to BKP Domain */
  	PWR_BackupAccessCmd(ENABLE);
	
    /* Reset Backup Domain */
    BKP_DeInit();
  
    /* Enable LSE OSC */
	RCC_LSICmd(ENABLE);
    /* Wait till LSE is ready */
	while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {
    }

    /* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* RTC configuration -----------------------------------------------------*/
    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* Set the RTC time base to 1s */
    RTC_SetPrescaler(32767);  
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
	RTC_ITConfig(RTC_IT_ALR, ENABLE);
	RTC_WaitForLastTask();
  
}

/**
  * @brief  Configures EXTI Lines.
  * @param  None
  * @retval None
  */
void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;

  /* Configure EXTI Line17(RTC Alarm) to generate an interrupt on rising edge */
  EXTI_ClearITPendingBit(EXTI_Line17);
  EXTI_InitStructure.EXTI_Line = EXTI_Line17;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}

/**
  * @brief  Configures the TIM2.
  * TIM_Prescaler = 1KHz; TIM_Period = 60s
  * @param  None
  * @retval None
  */
void TIM2_Configuration(void)
{
	//uint16_t PrescalerValue = 0;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//PrescalerValue = (uint16_t) (SystemCoreClock / TIM2_PRESCALER_HZ) - 1;
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period= (uint16_t)TIM2_PERIOD_TIMER;
    // 累计 TIM_Period个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t)TIM2_PRESCALER_TIMER;
    //TIM_TimeBaseStructure.TIM_Prescaler= PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2, ENABLE);				// 开启时钟
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE); // 先关闭等待使用   
}

/**
  * @brief  Configures the TIM4.
  * TIM_Prescaler = 1KHz; TIM_Period = 60s
  * @param  None
  * @retval None
  */
void TIM4_Configuration(void)
{
//	uint16_t PrescalerValue = 0;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//	PrescalerValue = (uint16_t) (SystemCoreClock / TIM4_PRESCALER_HZ) - 1;
	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
    TIM_DeInit(TIM4);
    TIM_TimeBaseStructure.TIM_Period= (uint16_t)TIM4_PERIOD_TIMER;
    // 累计 TIM_Period个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Prescaler= (uint16_t)TIM4_PRESCALER_TIMER;
    //TIM_TimeBaseStructure.TIM_Prescaler= PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM4, ENABLE);																		/* 开启时钟 */
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , DISABLE); // 先关闭等待使用   
}

/**
  * @brief  Configures the NVIC.
  * @param  None
  * @retval None
  */
void RTC_NVIC_Configuration(void)
{  
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 2 bits for Preemption Priority and 2 bits for Sub Priority */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel =RTCAlarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

}

/**
  * @brief  Configures the NVIC.
  * @param  None
  * @retval None
  */
void USART_NVIC_Configuration(void)
{  
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd =ENABLE;  
	NVIC_Init(&NVIC_InitStructure);

}

/*
 * 函数名：TIM2_NVIC_Configuration
 * 描述  ：TIM2中断优先级配置
 * 输入  ：无
 * 输出  ：无	
 */
void TIM2_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
 * 函数名：TIM4_NVIC_Configuration
 * 描述  ：TIM4中断优先级配置
 * 输入  ：无
 * 输出  ：无	
 */
void TIM4_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Start timer2
  * @param  None
  * @retval None
  */
void TIM2_Start(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  Stop timer2
  * @param  None
  * @retval None
  */
void TIM2_Stop(void)
{
	TIM_Cmd(TIM2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);
}

/**
  * @brief  Start timer4
  * @param  None
  * @retval None
  */
void TIM4_Start(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

/**
  * @brief  Stop timer4
  * @param  None
  * @retval None
  */
void TIM4_Stop(void)
{
	TIM_Cmd(TIM4, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , DISABLE);
}

/**
  * @brief  Configures the WatchDog.
  * @param  None
  * @retval None
  */
void IWDG_Configuration(void)
{
	/* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
	     dispersion) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: LSI/32 */
	IWDG_SetPrescaler(IWDG_Prescaler_256);

	/* Set counter reload value to obtain 10s IWDG TimeOut.
	 Counter Reload Value = 10s/6.4ms = 1562
	*/
	IWDG_SetReload(1562);
}

/**
  * @brief  Configures the SysTick to generate an interrupt each 250 ms.
  * @param  None
  * @retval None
  */
void stm32gps_led_cfg(void)
{
    STM_EVAL_LEDInit(LED1);
    //STM_EVAL_LEDInit(LED2);
    //STM_EVAL_LEDInit(LED3);
    //STM_EVAL_LEDInit(LED4);

    //STM_EVAL_LEDOn(LED1);
    //STM_EVAL_LEDOff(LED2);
    //STM_EVAL_LEDOff(LED3);
    //STM_EVAL_LEDOn(LED4);
}

void stm32gps_com_debug_cfg(void)
{
	USART_InitTypeDef USART_InitStructure;
    /* USARTx configured as follow:
     - BaudRate = 9600 baud
     - Word Length = 8 Bits
     - One Stop Bit
     - No parity
     - Hardware flow control disabled (RTS and CTS signals)
     - Receive and transmit enabled
     */
    USART_InitStructure.USART_BaudRate = USART_DBG_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    STM_EVAL_COMInit(COM3, &USART_InitStructure);
}

#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/**
 * @brief  Retargets the C library printf function to the USART.
 * @param  None
 * @retval None
 */
PUTCHAR_PROTOTYPE 
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    USART_SendData(EVAL_COM3, (uint8_t) ch);

    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(EVAL_COM3, USART_FLAG_TC) == RESET) {
    }

    return ch;
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}

#endif

void stm32gps_com_gps_cfg(void)
{
	USART_InitTypeDef USART_InitStructure;
	
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    STM_EVAL_COMInit(COM1, &USART_InitStructure);
}

void stm32gps_com_gsm_cfg(void)
{
	USART_InitTypeDef USART_InitStructure;
	
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    STM_EVAL_COMInit(COM2, &USART_InitStructure);
}
