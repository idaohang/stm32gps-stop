
delay_ms�����Ķ�ʱ��׼��delay_ms(1000)���ֻ��0.6s���ҡ��ú���ʹ�������ʱʵ�֡�
delay_10ms������ʱ׼��delay_10ms(100)��1s���ú�������systickʵ�֡�

�� D:\GitHub\stm32gps\Project\stm32gps\system_stm32f10x.c ���޸���ʱ��Ƶ��Ϊ24MHz�����ʱ��Ϊ72MHz��TIM2�����ʱʱ��Ϊ59s���ң����ܲ�����Ҫ��
��stm32gps_config.c�е�RCC_Configuration����������APHB1ΪRCC_HCLK_Div4���������ʱ�Ӷ�ʱΪ3min��������Ҫ�趨��ֵ��

1. ��project��options��ѡ��USE_STM32_GPS_BOARD_VB��USE_STM32_GPS_BOARD_VA
2. ��עmain�����е�#if 0��ȥ�����޸�Ϊ#if 1��Ŀǰ���޸�ΪMACRO_FOR_TEST
3. DBG_ENABLE_MACRO����ʽ�汾��ȥ��
4. ��Ҫ����MACRO_FOR_TEST

//////////////////////////////
��ʽ�汾ʹ��realse
�������԰汾ʹ��factory
���԰汾ʹ��debug




1. LEDָʾ��
����ʽ�汾����������ӣ���ô����ʽ����汾�н���ʹ�á�
2. 

��Ҫȷ���Ĳ���
SLEEP_NORMAL_SEC �͹���ģʽ��˯��ʱ�� ��λsec ��������Ϊ24hour=86400
SLEEP_TIM2_RATIO ��ʱ����ʱ��˯��ʱ�� =  SLEEP_NORMAL_SEC/SLEEP_TIM2_RATIO����������Ϊ6
GPS_RETERY_TIMES    4  // GPS ���Դ�����ÿ��10s
GSM_RETERY_TIMES    10 // GSM �����ط�����
GSM_SUCCESS_TIMES   1  // GSM �ɹ����ʹ��������յ�SEND OK������ʾ�ѽ����ݰ��ɹ����͵���վ��

