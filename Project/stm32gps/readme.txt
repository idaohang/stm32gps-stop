
delay_ms函数的定时不准，delay_ms(1000)大概只有0.6s左右。该函数使用语句延时实现。
delay_10ms函数定时准，delay_10ms(100)是1s，该函数是用systick实现。

在 D:\GitHub\stm32gps\Project\stm32gps\system_stm32f10x.c 中修改了时钟频率为24MHz，如果时钟为72MHz，TIM2的最长定时时间为59s左右，可能不符合要求。
在stm32gps_config.c中的RCC_Configuration函数中设置APHB1为RCC_HCLK_Div4，这样最大时钟定时为3min，根据需要设定该值。

1. 在project的options中选择USE_STM32_GPS_BOARD_VB或USE_STM32_GPS_BOARD_VA
2. 关注main函数中的#if 0都去掉或修改为#if 1。目前已修改为MACRO_FOR_TEST
3. DBG_ENABLE_MACRO在正式版本中去掉
4. 不要定义MACRO_FOR_TEST

//////////////////////////////
正式版本使用realse
工厂测试版本使用factory
调试版本使用debug




1. LED指示灯
在正式版本中如果不焊接，那么在正式软件版本中将不使用。
2. 

需要确定的参数
SLEEP_NORMAL_SEC 低功耗模式的睡眠时间 单位sec 建议设置为24hour=86400
SLEEP_TIM2_RATIO 定时器超时的睡眠时间 =  SLEEP_NORMAL_SEC/SLEEP_TIM2_RATIO，建议设置为6
GPS_RETERY_TIMES    4  // GPS 重试次数，每次10s
GSM_RETERY_TIMES    10 // GSM 错误重发次数
GSM_SUCCESS_TIMES   1  // GSM 成功发送次数，即收到SEND OK，它表示已将数据包成功发送到基站。

