/**
 ******************************************************************************
 * @file    USART/Printf/main.c
 * @author  MCD Application Team
 * @version V3.5.0
 * @date    08-April-2011
 * @brief   Main program body
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
#include "stm32f10x.h"
#include "stm32f10x_it_api.h"
#include "stm32gps_board.h"
#include "stm32gps_config.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "GSM_App.h"
#include "GPS_App.h"
#include "eelink.h"
#include "gpio.h"


/* Private typedef -----------------------------------------------------------*/

typedef struct
{
    unsigned char status[2];    // dveice status
    unsigned char analog1[2]; 	// analog input 1
    unsigned char analog2[2];		// analog input 2
} ST_DEVICEDATA, *pST_DEVICEDATA;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define BKP_DR_NUMBER              42
/* Private variables ---------------------------------------------------------*/
__IO uint32_t LsiFreq = 40000;

// imei information buffer
uint8_t imeiBuf[IMEI_INFO_LEN];
uint8_t g_IMSIBuf[IMSI_INFO_LEN];
uint8_t g_phoneNum[PHONE_NUMBER_LEN];

// packet message
EELINK_SIM_PACKET_LOGIN loginMsg;
char loginBuf[PROTO_LOGIN_BUF];
EELINK_PACKET_GPS gpsMsg;
static char gpsBuf[PROTO_GPS_BUF];

ST_SIMDATA g_simData;
ST_GPSDATA g_gpsData;
ST_DEVICEDATA g_deviceData;
ST_IMSIINFO g_imsiInfo;

uint16_t g_sequenceNum;  // GPRS packet's sequence number
uint16_t g_successNum;   // GPRS Send Success number

uint8_t g_gpsStatus;    // GPS status 0x55 - error; 0xaa - ok

uint32_t g_setSleepSec;  // Server setting sleep seconds

uint32_t g_alarmFlag;  // 报警标志 SET-valid; RESET-invalid
uint32_t g_alarmPacketFlag; // 报警数据包标志 SET - alarm packet; RESET - not alarm packet


/* Private function prototypes -----------------------------------------------*/
uint8_t ProcessIMEI(uint8_t *pImei, uint8_t *pRst, int32_t imeilen, int32_t rstlen);
void loadLoginMsg(uint8_t *imei, uint16_t sequence);
void PackLoginMsg(void);
void LoadGpsMsg(uint16_t sequence);
void PackGpsMsg(void);
void PackAlarmMsg(void);
void PackFactoryMsg(void);
/* Private functions ---------------------------------------------------------*/

#ifdef DBG_ENABLE_MACRO
void ShowLoginMsg(void)
{
    uint32_t i;
    // imei
    DEBUG("IMEI:");
    for(i = 0; i < IMEI_INFO_LEN; i++)
    {
        DEBUG("%c-", imeiBuf[i]);
    }
    DEBUG("\r\n");
    // login msg
    DEBUG("LOGIN MSG:");
    for(i = 0; i < PROTO_LOGIN_BUF; i++)
    {
        DEBUG("0x%x-", loginBuf[i]);
    }
    DEBUG("\r\n");
}

void ShowGpsMsg(void)
{
    uint32_t i;
    // gps msg
    DEBUG("GPS MSG:");
    for(i = 0; i < PROTO_GPS_BUF; i++)
    {
        DEBUG("0x%x-", gpsBuf[i]);
    }
    DEBUG("\r\n");
}
#endif // DBG_ENABLE_MACRO

/**
  * @brief  Init Global Variables
  * @param  None
  * @retval None
  */
void InitVariables(void)
{
    uint32_t i;

    g_sequenceNum = 1;
    g_successNum = 0;
    g_gpsStatus = 0x55;
    g_setSleepSec = SLEEP_NORMAL_SEC;
    g_alarmFlag = RESET;
	g_alarmPacketFlag = RESET;

    memset(imeiBuf, IMEI_INFO_LEN, 0);
    for(i = 0; i < PROTO_LOGIN_BUF; i++)
    {
        loginBuf[i] = 0;
    }
    for(i = 0; i < PROTO_GPS_BUF; i++)
    {
        gpsBuf[i] = 0;
    }

    memset(g_IMSIBuf, IMSI_INFO_LEN, 0);

    for(i = 0; i < PHONE_NUMBER_LEN; i++)
    {
        g_phoneNum[i] = 0x30;
    }

    memset(&loginMsg, sizeof(loginMsg), 0);
    memset(&gpsMsg, sizeof(gpsMsg), 0);

    memset(&g_simData, sizeof(g_simData), 0);
    memset(&g_gpsData, sizeof(g_gpsData), 0);
    memset(&g_deviceData, sizeof(g_deviceData), 0);

}



/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
//    int i = 0;
	uint32_t errNum = 0;
    uint8_t gpsRecvTimes = 0;  // GPS Information Received times
    uint8_t rst = GPS_FAIL;    // GPS function return result
    ST_GPSRMCINFO rmc;         // GPS RMC packet information
    unsigned char gprsSendErrNum = 0;  // GPRS send error number
//    uint16_t removeNum = 0;    // Remove detect counter
    uint16_t sendLen = 0;      // GPRS send length (used for GPS and ALARM Msg)
    uint32_t sleepSec = 0;

    // Used for parse GPRS Received Data
    char *pRecvBuf = NULL;     // GPRS Received buffer
    uint32_t recvLen = 0;      // GPRS Received length
    char *pfeed = NULL;        // Used for parse

    /////////////////////////////////////////////////////////////////
    // Configure the GPIO ports and Power OFF GPS and GSM
    /////////////////////////////////////////////////////////////////
    MX_GPIO_Init();
    GPSPowerOff();
    GSM_PowerOff();
    /////////////////////////////////////////////////////////////////
    // Configure the SysTick
    /////////////////////////////////////////////////////////////////
    stm32gps_sys_tick_cfg();

    /////////////////////////////////////////////////////////////////
    // Configure PWR and BKP
    /////////////////////////////////////////////////////////////////
    /* Enable PWR and BKP clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    /* Enable WKUP pin */
//    PWR_WakeUpPinCmd(ENABLE);


    /////////////////////////////////////////////////////////////////
    // Configure RCC
    /////////////////////////////////////////////////////////////////
    //RCC_Configuration();

    /////////////////////////////////////////////////////////////////
    // Configure EXTI
    /////////////////////////////////////////////////////////////////
    EXTI_Configuration();

    /////////////////////////////////////////////////////////////////
    // Configure RTC
    /////////////////////////////////////////////////////////////////
    RTC_Configuration();
    RTC_NVIC_Configuration();

    /////////////////////////////////////////////////////////////////
    // Configure TIMER
    /////////////////////////////////////////////////////////////////
//    TIM2_Configuration();
//    TIM2_NVIC_Configuration();

    /////////////////////////////////////////////////////////////////
    // Configure LED, BUTTON and USART(GPS + GSM + DEBUG)
    /////////////////////////////////////////////////////////////////

    stm32gps_led_cfg();
    STM_EVAL_LEDOff(LED1);
#ifdef DBG_ENABLE_MACRO
    stm32gps_com_debug_cfg();
#endif // DBG_ENABLE_MACRO

    STM_EVAL_PBInit(BUTTON_KEY, BUTTON_MODE_EXTI);

    usart_init(STM32_SIM908_GPS_COM);
    stm32gps_com_gps_cfg();
	/* Enable the EVAL_COM1 Receive interrupt: this interrupt is generated when the 
     EVAL_COM1 receive data register is not empty */
  	USART_ITConfig(EVAL_COM1, USART_IT_RXNE, ENABLE);

    usart_init(STM32_SIM908_GSM_COM);
    stm32gps_com_gsm_cfg();
	USART_ITConfig(EVAL_COM2, USART_IT_RXNE, ENABLE);

	USART_NVIC_Configuration();

	/////////////////////////////////////////////////////////////////
    // Turn On TIM2
    /////////////////////////////////////////////////////////////////
    //TIM2_Start();

    /////////////////////////////////////////////////////////////////
    // Init Variables
    /////////////////////////////////////////////////////////////////
    InitVariables();

    /////////////////////////////////////////////////////////////////
    // Main LOOP
    /////////////////////////////////////////////////////////////////
    while(1)
    {
		//MX_GPIO_Init();
	    //GPSPowerOff();
	    //GSM_PowerOff();
	    delay_10ms(STICK_ON_SEC);

        // If Stick On Car or sth
        if(((uint32_t)Bit_RESET == STM_EVAL_PBGetState(BUTTON_KEY))
                || (SET == g_alarmFlag))
        {
            /////////////////////////////////////////////////////////////////
            // First Power ON GPS
            /////////////////////////////////////////////////////////////////
            GPSPowerOn();
//			GPIO_ResetBits(GPIOB, GPIO_Pin_5);
//			delay_ms(500);
//			GPIO_SetBits(GPIOB, GPIO_Pin_5);
			DEBUG("GPSPowerOn\n");
            while(1)
            {
				//GPSClearBuffer();
                // delay 2 sec
                delay_10ms(200);
				
                /////////////////////////////////////////////////////////////////
                // Receive GPS Data and Analyze, If Recv Success then break
                /////////////////////////////////////////////////////////////////
                rst = GPSInfoAnalyze(&rmc);
                if( GPS_SUCCESS == rst)
                {
                    g_gpsStatus = 0xAA;
#ifdef DBG_ENABLE_MACRO
                    STM_EVAL_LEDOff(LED1);
                    DEBUG("GPS Recv Success!\n");
#endif // DBG_ENABLE_MACRO
                    break;
                }
                else if(GPS_INVALID == rst)
                {
                    g_gpsStatus = 0xAA;
#ifdef DBG_ENABLE_MACRO
                    STM_EVAL_LEDOn(LED1);
                    DEBUG("GPS Recv Invalid\n");
#endif // DBG_ENABLE_MACRO
                }
                else
                {
                    g_gpsStatus = 0x55;
#ifdef DBG_ENABLE_MACRO
                    STM_EVAL_LEDOn(LED1);
                    DEBUG("GPS Recv Error\n");
#endif // DBG_ENABLE_MACRO
                }

                /////////////////////////////////////////////////////////////////
                // If GPS Receive Times Over then break, ~30sec
                /////////////////////////////////////////////////////////////////
                gpsRecvTimes++;
                if(gpsRecvTimes > GPS_RETERY_TIMES)
                {
                    gpsRecvTimes = 0;
                    break;
                }

                /////////////////////////////////////////////////////////////////
                // Set RTC Alarm to wake from STOP mode
                /////////////////////////////////////////////////////////////////
                /* Wait till RTC Second event occurs */
                RTC_ClearFlag(RTC_FLAG_SEC);
                while(RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET);

                /* Alarm in 10 second */
                RTC_SetAlarm(RTC_GetCounter() + STOP_GPS_SEC);
                /* Wait until last write operation on RTC registers has finished */
                RTC_WaitForLastTask();

                /////////////////////////////////////////////////////////////////
                // Go Into STOP Mode
                /////////////////////////////////////////////////////////////////
                /* Request to enter STOP mode with regulator in low power mode*/
                PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

                /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
                	PLL as system clock source (HSE and PLL are disabled in STOP mode) */
                SYSCLKConfig_STOP();
            }

            /////////////////////////////////////////////////////////////////
            // Second Power ON GSM
            /////////////////////////////////////////////////////////////////
            GSM_PowerOn();
			//GSM_ClearBuffer();
			// delay 2 sec
            //delay_10ms(800);

            g_sequenceNum = 1; // Init packet sequence to 1
            g_successNum = 0;
            gprsSendErrNum = 0;

            /////////////////////////////////////////////////////////////////
            // (While loop) to handshake with SIM800L module
            /////////////////////////////////////////////////////////////////
            GSM_Init();
			//GSM_TurnOnOff();

            /////////////////////////////////////////////////////////////////
            // (While loop) to check sim card status
            /////////////////////////////////////////////////////////////////
            GSM_CheckSIMCard();

            /////////////////////////////////////////////////////////////////
            // (While loop) to check network registration status
            /////////////////////////////////////////////////////////////////
            GSM_CheckNetworkReg();

            /////////////////////////////////////////////////////////////////
            // (While loop) to check Attach or Detach from GPRS Service
            /////////////////////////////////////////////////////////////////
            GSM_CheckGPRSService();

            /////////////////////////////////////////////////////////////////
            // (While loop) to set network registration
            // 2 Enable network registration unsolicited result code with
            // location information +CREG: <stat>[,<lac>,<ci>]
            /////////////////////////////////////////////////////////////////
            GSM_SetNetworkReg();

            /////////////////////////////////////////////////////////////////
            // Check CPIMode and (While loop 5) to Set CIPMODE
            /////////////////////////////////////////////////////////////////
            GSM_SetCIPMode(0);
            /////////////////////////////////////////////////////////////////
            // Check CSTT and (While loop 5) to Start task and Set APN "CMNET"
            /////////////////////////////////////////////////////////////////
            GSM_StartTaskAndSetAPN();

            /////////////////////////////////////////////////////////////////
            // (While loop 5) to Bring Up Wireless Connection
            /////////////////////////////////////////////////////////////////
            GSM_BringUpConnect();

            /////////////////////////////////////////////////////////////////
            // (While loop 5) to Get Local IP Address
            /////////////////////////////////////////////////////////////////
            GSM_GetLocalIP();

            /////////////////////////////////////////////////////////////////
            // (While loop 5) to Start Up TCP or UDP Connection
            /////////////////////////////////////////////////////////////////
            GSM_StartUpConnect();

            /////////////////////////////////////////////////////////////////
            // Query SIM IMEI and Package LOGIN Message
            /////////////////////////////////////////////////////////////////
            while(1)
            {
            	if(USART_FAIL == GSM_QueryImei(imeiBuf))
            	{
					errNum++;
					if(errNum > 5)
					{
						break;
					}
            	}
				else
				{
					break;
				}
            
            }
			errNum = 0;
            loadLoginMsg(imeiBuf, g_sequenceNum);
#ifndef FACTORY_ENABLE_MACRO
            PackLoginMsg();
            g_sequenceNum++;
            /////////////////////////////////////////////////////////////////
            // While loop to Send LOGIN Message
            /////////////////////////////////////////////////////////////////
            while(1)
            {
            	if(USART_FAIL == GPRS_SendData_rsp(loginBuf, EELINK_LOGIN_MSGLEN, &pRecvBuf, &recvLen))
            	{
					errNum++;
					if(errNum > 5)
					{
						break;
					}
					
#ifdef DBG_ENABLE_MACRO
	                STM_EVAL_LEDToggle(LED1);
	                DEBUG("GPRS_SendData LOGIN MSG Fail\n");
#endif // DBG_ENABLE_MACRO
            	}
				else
				{
					// parse response data, pp is 0x70 0x70
		            pfeed = strstr_len(pRecvBuf, "pp", recvLen);
		            if(pfeed != NULL)
		            {
		                sleepSec = (uint32_t)(((*(pfeed + 7)) << 24)
		                                      + ((*(pfeed + 8)) << 16)
		                                      + ((*(pfeed + 9)) << 8)
		                                      + (*(pfeed + 10)));
						DEBUG("g_setSleepSec = %d\n", sleepSec);
		                // Check sleep time setting value
		                if((sleepSec > SLEEP_TIME_MIN) && (sleepSec < SLEEP_TIME_MAX))
		                {
		                    g_setSleepSec = sleepSec;
							
		                }
		            }
		            else
		            {
		                DEBUG("Server Set Sleep Timer ERROR\n");
		            }
					break;
				}
            }
			// If Login Msg Send Error then break go to sleep
			//if(errNum > 5)
			//{
			//	break;
			//}
			//errNum = 0;

            
#endif // FACTORY_ENABLE_MACRO

            /////////////////////////////////////////////////////////////////
            // Query SIM IMSI and Analyze
            /////////////////////////////////////////////////////////////////
            while(1)
            {
            	if(USART_FAIL == GSM_QueryImsi(&g_imsiInfo))
            	{
					errNum++;
					if(errNum > 5)
					{
						break;
					}
            	}
				else
				{
					break;
				}
            }
			errNum = 0;

            /////////////////////////////////////////////////////////////////
            // While loop to Send GPS Message
            /////////////////////////////////////////////////////////////////
            while(1)
            {
                /////////////////////////////////////////////////////////////////
                // Receive GPS Data and Analyze
                /////////////////////////////////////////////////////////////////
                rst = GPSInfoAnalyze(&rmc);
                if( GPS_SUCCESS == rst)
                {
                    g_gpsStatus = 0xAA;
                    DEBUG("GPS Recv Success!\n");
                }
                else if(GPS_INVALID == rst)
                {
                    g_gpsStatus = 0xAA;
                    DEBUG("GPS Recv Invalid\n");
                }
                else
                {
                    g_gpsStatus = 0x55;
                    memset(&rmc, sizeof(rmc), 0);
                    DEBUG("GPS Recv Error\n");
                }

                ParseGPSInfo(rmc, &g_gpsData);
                /////////////////////////////////////////////////////////////////
                // Get GSM related Data and Analyze, Package GPS Message
                /////////////////////////////////////////////////////////////////
                GetGsmData(&g_simData, g_imsiInfo);
                LoadGpsMsg(g_sequenceNum);
#ifdef FACTORY_ENABLE_MACRO
				while(1)
				{
                	if(USART_FAIL == GSM_QueryImsiBuf(g_IMSIBuf))
                	{
						errNum++;
						if(errNum > 5)
						{
							break;
						}
                	}
					else
					{
						break;
					}
                }
				errNum = 0;
                //GSM_QueryNumber(g_phoneNum); // NOT Support Now
                PackFactoryMsg();
                sendLen = FACTORY_REPORT_MSGLEN;
#else
                // detect remove action and alarm flag is setted then send alarm msg
                if(((uint32_t)Bit_SET == STM_EVAL_PBGetState(BUTTON_KEY))
                        && (SET == g_alarmFlag))
                {
                    PackAlarmMsg();
                    sendLen = EELINK_ALARM_MSGLEN;
					g_alarmPacketFlag = SET;
                }
                else
                {
                    PackGpsMsg();
                    sendLen = EELINK_GPS_MSGLEN;
					g_alarmPacketFlag = RESET;
                }
#endif // FACTORY_ENABLE_MACRO
                /////////////////////////////////////////////////////////////////
                // Send GPS Message or ALARM or FACTORY TEST Message
                /////////////////////////////////////////////////////////////////
                if(USART_FAIL == GPRS_SendData(gpsBuf, sendLen))
                {
#ifdef DBG_ENABLE_MACRO
                    STM_EVAL_LEDToggle(LED1);
#endif // DBG_ENABLE_MACRO
                    gprsSendErrNum++;
                    if(gprsSendErrNum > GSM_RETERY_TIMES)
                    {
                        gprsSendErrNum = 0;
                        GPRS_CloseLink();
                        GPRS_CIPShut();
                        break;
                    }
                    DEBUG("GPRS_SendData Fail %d\n", gprsSendErrNum);
                }
                else
                {
                    // Increase success number
                    g_successNum++;
					DEBUG("GPRS_SendData Success %d\n", g_successNum);
                }

#ifdef DBG_ENABLE_MACRO
                ShowGpsMsg();
#endif // DBG_ENABLE_MACRO

                // if send ok then into sleep
                if(g_successNum > GSM_SUCCESS_TIMES)
                {
                    // Toggle alarm flag
                    if((SET == g_alarmFlag) && (SET == g_alarmPacketFlag))
                    {
                        g_alarmFlag = RESET;
                    }
                    else
                    {
                        g_alarmFlag = SET;
                    }
                    DEBUG("send_ok alarmflag = %d\n", g_alarmFlag);
                    break;
                }


                // Increase sequence number if overflow then re-init to 1
                g_sequenceNum++;
                if(g_sequenceNum == 0)
                {
                    g_sequenceNum = 1;
                }

                // delay 1 second
                delay_10ms(100);
            }

        }

        /////////////////////////////////////////////////////////////////
        // This Process is Finished, Then goto sleep
        /////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////
        // Power OFF GPS and GSM before going into sleep mode
        /////////////////////////////////////////////////////////////////
        STM_EVAL_LEDOff(LED1);
        GPRS_CPOwd();
        GPSPowerOff();
        GSM_PowerOff();
		
        /* Wait till RTC Second event occurs */
        RTC_ClearFlag(RTC_FLAG_SEC);
        while(RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET);


        RTC_SetAlarm(RTC_GetCounter() + g_setSleepSec);

        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        DEBUG("GOING INTO STOPMODE %d sec\n", g_setSleepSec);
        /* Request to enter STOP mode with regulator in low power mode*/
        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

        /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
           PLL as system clock source (HSE and PLL are disabled in STOP mode) */
        SYSCLKConfig_STOP();
    }

	// SHOULD NOT GO HAERE
	while(1)
	{
		STM_EVAL_LEDToggle(LED1);
		delay_10ms(50);
	}
}

/*********************************************************************************************************
 ** Function name:       ProcessIMEI()
 ** Descriptions:        将15个字节的IMEI字符串处理成8个字节的EELINK协议格式
 ** input parameters:    pImei 需要处理的15个字节的IMEI字符串
 **                      pRst 返回的字符串指针
 **                      imeilen IMEI字符串的长度
 **                      rstlen 返回的字符串的长度
 ** output parameters:
 ** Returned value:      成功RST_OK 失败RST_FAIL
 *********************************************************************************************************/
uint8_t ProcessIMEI(uint8_t *pImei, uint8_t *pRst, int32_t imeilen, int32_t rstlen)
{
    uint32_t i;
    if(((imeilen / 2 + 1) < rstlen) || (imeilen != 15))
    {
        return RST_FAIL;
    }

    *(pRst + 0) = (*(pImei + 0)) & 0x0F;
    for(i = (imeilen / 2); i > 0 ; i--)
    {
        *(pRst + i) = (((*(pImei + i * 2 - 1)) << 4) & 0xF0) + ((*(pImei + i * 2)) & 0x0F);
    }
    return RST_OK;
}

/**
  * @brief  Load IMEI to loginMsg structure
  * @param  None
  * @retval None
  */
void loadLoginMsg(uint8_t *imei, uint16_t sequence)
{
    loginMsg.hdr.header[0] = PROTO_EELINK_HEADER;
    loginMsg.hdr.header[1] = PROTO_EELINK_HEADER;
    loginMsg.hdr.type = PACKET_EELINK_LOGIN;
    loginMsg.hdr.len[0] = 0x00;
    loginMsg.hdr.len[1] = 0x0C;
    loginMsg.hdr.seq[0] = (uint8_t)((sequence >> 8) & 0x00FF);
    loginMsg.hdr.seq[1] = (uint8_t)((sequence) & 0x00FF);
    if(RST_FAIL == ProcessIMEI(imei, loginMsg.imei, IMEI_INFO_LEN, 8))
    {
        // re-init imei buffer
        memset(loginMsg.imei, sizeof(loginMsg.imei), 0);
        DEBUG("IMEI process ERROR!\n");
    }
    loginMsg.lang = EELINK_LANG; // english
    loginMsg.zone = EELINK_ZONE; // east 8
}

/**
  * @brief  Load loginMsg structure to send buffer
  * @param  None
  * @retval None
  */
void PackLoginMsg(void)
{
    uint32_t i;
    uint32_t offset = 0;
    offset = 0;
    for(i = 0; i < 2; i++)
    {
        loginBuf[offset] = loginMsg.hdr.header[i];
        offset++;
    }
    loginBuf[offset] = loginMsg.hdr.type;
    offset++;
    for(i = 0; i < 2; i++)
    {
        loginBuf[offset] = loginMsg.hdr.len[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        loginBuf[offset] = loginMsg.hdr.seq[i];
        offset++;
    }
    for(i = 0; i < 8; i++)
    {
        loginBuf[offset] = loginMsg.imei[i];
        offset++;
    }
    loginBuf[offset] = loginMsg.lang;
    offset++;
    loginBuf[offset] = loginMsg.zone;
    offset++;
    if(offset != (EELINK_LOGIN_MSGLEN))
    {
        DEBUG("PackLoginMsg ERROR!\n");
    }
}

/**
  * @brief  Load GPS information to gpsMsg structure
  * @param  None
  * @retval None
  */
void LoadGpsMsg(uint16_t sequence)
{
    uint32_t i;
    gpsMsg.hdr.header[0] = PROTO_EELINK_HEADER;
    gpsMsg.hdr.header[1] = PROTO_EELINK_HEADER;
    gpsMsg.hdr.type = PACKET_EELINK_GPS;
    gpsMsg.hdr.len[0] = 0x00;
    gpsMsg.hdr.len[1] = (EELINK_GPS_MSGLEN - 5);  // 37
    gpsMsg.hdr.seq[0] = (uint8_t)((sequence >> 8) & 0x00FF);
    gpsMsg.hdr.seq[1] = (uint8_t)((sequence) & 0x00FF);

    for(i = 0; i < 4; i++)
    {
        gpsMsg.utctime[i] = g_gpsData.utc.s[3 - i];
    }

    for(i = 0; i < 4; i++)
    {
        gpsMsg.lati[i] = g_gpsData.latitude.s[3 - i];
    }

    for(i = 0; i < 4; i++)
    {
        gpsMsg.longi[i] = g_gpsData.longitude.s[3 - i];
    }

    gpsMsg.speed = g_gpsData.speed;

    for(i = 0; i < 2; i++)
    {
        gpsMsg.course[i] = g_gpsData.course.s[1 - i];
    }

    for(i = 0; i < 9; i++)
    {
        gpsMsg.basestation[i] = g_simData.Station[i];
    }

    gpsMsg.gps_valid = g_gpsData.status;

    for(i = 0; i < 2; i++)
    {
        gpsMsg.dev_status[i] = g_deviceData.status[i];
    }

    for(i = 0; i < 2; i++)
    {
        gpsMsg.battery_voltage[i] = g_simData.Battery[i];
    }

    for(i = 0; i < 2; i++)
    {
        gpsMsg.signal_strength[i] = g_simData.Signal[i];
    }

    for(i = 0; i < 2; i++)
    {
        gpsMsg.analog_input1[i] = g_deviceData.analog1[i];
    }

    for(i = 0; i < 2; i++)
    {
        gpsMsg.analog_input2[i] = g_deviceData.analog2[i];
    }
}

/**
  * @brief  Load gpsMsg structure to send buffer
  * @param  None
  * @retval None
  */
void PackGpsMsg(void)
{
    uint32_t i;
    uint32_t offset = 0;
    offset = 0;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.hdr.header[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.hdr.type;
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.hdr.len[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.hdr.seq[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.utctime[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.lati[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.longi[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.speed;
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.course[i];
        offset++;
    }
    for(i = 0; i < 9; i++)
    {
        gpsBuf[offset] = gpsMsg.basestation[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.gps_valid;
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.dev_status[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.battery_voltage[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.signal_strength[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.analog_input1[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.analog_input2[i];
        offset++;
    }

    if(offset != (EELINK_GPS_MSGLEN))
    {
        DEBUG("PackGpsMsg ERROR!\n");
    }
}

/**
  * @brief  Load gpsMsg structure to alarm send buffer
  * @param  None
  * @retval None
  */
void PackAlarmMsg(void)
{
    uint32_t i;
    uint32_t offset = 0;
    offset = 0;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.hdr.header[i];
        offset++;
    }
    gpsBuf[offset] = PACKET_EELINK_WARNING; //gpsMsg.hdr.type;
    offset++;
    gpsBuf[offset] = 0; // header len
    offset++;
    gpsBuf[offset] = (EELINK_ALARM_MSGLEN - 5); // header len
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.hdr.seq[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.utctime[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.lati[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.longi[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.speed;
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.course[i];
        offset++;
    }
    for(i = 0; i < 9; i++)
    {
        gpsBuf[offset] = gpsMsg.basestation[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.gps_valid;
    offset++;
    gpsBuf[offset] = 0x71;  // remove alarm flag
    offset++;

    if(offset != (EELINK_ALARM_MSGLEN))
    {
        DEBUG("PackAlarmMsg ERROR!\n");
    }
}

/**
  * @brief  Load gpsMsg and loginMsg structure to factory send buffer
  * @param  None
  * @retval None
  */
void PackFactoryMsg(void)
{
    uint32_t i = 0;
    uint32_t offset = 0;
    offset = 0;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = PROTO_FACTORY_HEADER;
        offset++;
    }
    gpsBuf[offset] = PACKET_FACTORY_REPORT;  // header type
    offset++;
    gpsBuf[offset] = 0; // header len
    offset++;
    gpsBuf[offset] = (FACTORY_REPORT_MSGLEN - 5); // header len
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.hdr.seq[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.utctime[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.lati[i];
        offset++;
    }
    for(i = 0; i < 4; i++)
    {
        gpsBuf[offset] = gpsMsg.longi[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.speed;
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.course[i];
        offset++;
    }
    for(i = 0; i < 9; i++)
    {
        gpsBuf[offset] = gpsMsg.basestation[i];
        offset++;
    }
    gpsBuf[offset] = gpsMsg.gps_valid;
    offset++;
    gpsBuf[offset] = 0;  // device status
    offset++;
    gpsBuf[offset] = STM_EVAL_PBGetState(BUTTON_KEY);  // device status
    offset++;
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.battery_voltage[i];
        offset++;
    }
    for(i = 0; i < 2; i++)
    {
        gpsBuf[offset] = gpsMsg.signal_strength[i];
        offset++;
    }
    for(i = 0; i < IMEI_INFO_LEN; i++)
    {
        gpsBuf[offset] = imeiBuf[i];
        offset++;
    }

    for(i = 0; i < IMSI_INFO_LEN; i++)
    {
        gpsBuf[offset] = g_IMSIBuf[i];
        offset++;
    }
    for(i = 0; i < PHONE_NUMBER_LEN; i++)
    {
        gpsBuf[offset] = g_phoneNum[i];
        offset++;
    }
    gpsBuf[offset] = g_gpsStatus;  // gps status
    offset++;
    gpsBuf[offset] = 0x01;  // software version v1.0
    offset++;
    gpsBuf[offset] = 0x00;
    offset++;

    if(offset != (FACTORY_REPORT_MSGLEN))
    {
        DEBUG("PackFactoryMsg ERROR!\n");
    }
}

/**
 * @}
 */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
