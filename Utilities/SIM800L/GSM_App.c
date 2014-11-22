/****************************************Copyright (c)****************************************************
 **                                  		 广州星海科技
 **                                  http://simhi.taobao.com
 **																		 TEL:15018750559
 **																			 QQ:86817447
 **------------------------File Info----------------------------------------------------------------------
 ** File Name:             GSM_App.c
 ** Last modified Date:   	2012.5.15
 ** Last Version:          V1.0
 ** Description:           GPS管理解析
 **
 **--------------------------------------------------------------------------------------------------------
 ** Created By:            Chenht
 ** Created date:          2012-5-3
 ** Version:               v1.0
 ** Descriptions:          The original version 初始版本
 **
 **--------------------------------------------------------------------------------------------------------
 ** Modified by:
 ** Modified date:
 ** Version:
 ** Description:
 **
 *********************************************************************************************************/
#include "at_sim800.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

#include "stm32gps_config.h"
#include "usart.h"
#include "GSM_App.h"

#define RESEND_TIMES_CPIN   3
#define RESEND_TIMES_COPS   5   // COPS command resend times

static char BackBuf[USART_GSM_BUFSIZE];
static char sendBuf[USART_GSM_BUFSIZE_SEND];
static char receiveBuf[USART_GSM_BUFSIZE_RECEIVE];

unsigned char GSMNetType;  // GSM Net type

/*********************************************************************************************************
 ** Function name:       strnchr()
 ** Descriptions:        查找字符串中第n个字符的位置
 ** input parameters:    S 需要查找的目标字符串
 **                      C 需要查找的字符
 **                      n 第几个位置的C
 ** output parameters:   需要查找的字符位置指针
 ** Returned value:
 *********************************************************************************************************/
char *strnchr(char *S, int C, int n)
{
    char *pnchr = NULL;
    char *pStr = S;

    if (n > strlen(S))
    {
        return NULL;
    }
    while (n--)
    {
        pnchr = strchr((const char *) pStr, C);
        if (NULL == pnchr)
        {
            break;
        }
        else
        {
            pStr = pnchr + 1;
        }
    }
    return pnchr;
}

/*********************************************************************************************************
 ** Function name:       strstr_len()
 ** Descriptions:        查找字符串中子字符串的位置，返回子字符串的位置
 ** input parameters:    str 需要查找的目标字符串
 **                      subStr 需要查找的子字符串
 **                      strlenth 目标字符串的长度
 ** output parameters:   
 ** Returned value:      子字符串的位置指针
 ** Example:  strstr_len("this,are,my,pointer", "my", 19) 返回的指针指向“m“的位置
 *********************************************************************************************************/
char *strstr_len(char *str, char *subStr, uint32_t strlenth)
{
    uint32_t subStrLen = strlen(subStr);
    int32_t cmpCnt = strlenth - strlen(subStr) + 1;

    char *retPtr = NULL;
    int32_t i;

    if (cmpCnt > 0)
    {
        for (i = 0; i < cmpCnt; i ++)
        {
            if (memcmp(str+i, subStr, subStrLen) == 0)
            {
                break;
            }
        }

        if (i < cmpCnt)
        {
            retPtr = str + i;
        }
    }

    return retPtr;
}

/*********************************************************************************************************
 ** Function name:       strnchr_len()
 ** Descriptions:        查找字符串中第几个字符值的位置，返回C的位置
 ** input parameters:    S 需要查找的目标字符串
 **                      C 需要查找的字符值
 **                      n 查找几个字符
 **                      len 目标字符串的长度
 ** output parameters:   
 ** Returned value:      C的位置指针
 ** Example: strnchr_len("this,are,my,pointer", ',', 2, 19); 返回指针指向are后面的','的位置
 *********************************************************************************************************/
char *strnchr_len(char *S, int C, uint32_t n, uint32_t len)
{
    char *pnchr = NULL;
    char *pStr = S;
    uint32_t i;

    if (n > len)
    {
        return NULL;
    }
    while (n--)
    {
        for (i = 0; i < len; i ++)
        {
            if (*(pStr+i) == C)
            {
                break;
            }
        }

        if (i < len)
        {
            pnchr = pStr+i;
            len -= i + 1;
        }
        else {
            pnchr = NULL;
        }

        if (NULL == pnchr)
        {
            break;
        }
        else
        {
            pStr = pnchr + 1;
        }
    }
    return pnchr;
}

/*********************************************************************************************************
 ** Function name:       strdig_len()
 ** Descriptions:        查找字符串中是否有n个数字，如果有则返回第一个数字位置的指针，如果没有则返回NULL。
 **                      字符串的长度必须大于需要查找的数字个数。
 ** input parameters:    str 需要查找的目标字符串
 **                      strlen 需要查找的字符串的长度
 **                      diglen 需要查找的数字的个数
 ** output parameters:   
 ** Returned value:      第一个数字的字符位置指针 or NULL
 *********************************************************************************************************/
char *strdig_len(char *str, uint32_t strlen, uint32_t diglen)
{

    char *retPtr = NULL;
    int32_t i;
	int32_t sumlen = 0;


    if (strlen > 0 && strlen > diglen)
    {
        for (i = 0; i < strlen; i ++)
        {
			if((*(str+i)>='0') && (*(str+i)<='9'))
			{
				sumlen++;
			}
			else
			{
				sumlen = 0;
			}
            if (sumlen >= diglen)
            {
				break;
            }
        }

        if (sumlen >= diglen)
        {
            retPtr = str + i - diglen +1;
        }
    }

    return retPtr;
}

/*********************************************************************************************************
 ** Function name:       strhex_len()
 ** Descriptions:        将十六进制字符串转换为十六进制数
 ** input parameters:    str 需要查找的目标字符串
 **                      len 目标字符串的长度
 ** output parameters:   
 ** Returned value:      转换后的十六进制数
 ** Example: 比如，字符串是"04"，转换后的值是0x04， 字符串是"2F"，转换后的值是0x2F
 *********************************************************************************************************/
uint32_t strhex_len(char *str, uint32_t len)
{
    uint32_t i;
	uint32_t sum = 0;
	uint32_t tmp = 0;
    for(i = 0; i < len; i++)
    {
	
	 if (str[i]>='A' && str[i] <='F')
	 {
		tmp = str[i]-55;//a-f之间的ascii与对应数值相差55如'A'为65,65-55即为A
	 }
	 else if(str[i]>='a' && str[i] <='f')
	 {
		tmp = str[i]-87;
	 }
	 else if(str[i]>='0' && str[i] <='9')
	 {
		tmp = str[i]-48;
	 }
		sum <<= 4;
		sum |= tmp;
	}

    return sum;
}

/*********************************************************************************************************
 ** Function name:       GSM_RingPinInit()
 ** Descriptions:        Ring状态检测IO初始化
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:
 *********************************************************************************************************/
void GSM_RingPinInit(void)
{
#if 0
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource8);

    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
#endif
}

/*********************************************************************************************************
 ** Function name:       GSM_ChkRingSta()
 ** Descriptions:        检测Ring状态
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      Ring状态
 *********************************************************************************************************/
unsigned char GSM_ChkRingSta(void)
{
#if 0
    if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8))
    {
        return 1;
    }
#endif
    return 0;
}

/*********************************************************************************************************
 ** Function name:       GSM_PowerCtrlInit()
 ** Descriptions:        启动控制IO初始化
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:
 *********************************************************************************************************/
void GSM_PowerCtrlInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
  * @brief  Turn on GSM's VCC power.
  * Output LOW to Turn Off VCC Power; HIGH to Turn On VCC Power.
  * @param  None
  * @retval None
  */
void GSM_PowerOn(void)
{
    GPIO_SetBits(GSM_PWR_CTRL_PORT, GSM_PWR_CTRL_PIN);
	
}

/**
  * @brief  Turn off GSM's VCC power.
  * Output LOW to Turn Off VCC Power; HIGH to Turn On VCC Power.
  * @param  None
  * @retval None
  */
void GSM_PowerOff(void)
{
    GPIO_ResetBits(GSM_PWR_CTRL_PORT, GSM_PWR_CTRL_PIN);
}

/*********************************************************************************************************
 ** Function name:       GSM_TurnOnOff()
 ** Descriptions:        启动或关闭模块
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:
 *********************************************************************************************************/
void GSM_TurnOnOff(void)
{
    GPIO_ResetBits(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN);
    delay_10ms(300);
    GPIO_SetBits(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN);
    delay_10ms(300);
}

/*********************************************************************************************************
 ** Function name:       GSM_TurnOnOff()
 ** Descriptions:        启动或关闭模块
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:
 *********************************************************************************************************/
void GSM_TurnOnOff_delay(void)
{
    GPIO_ResetBits(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN);
    delay_ms(3000);
    GPIO_SetBits(GSM_PWRKEY_PORT, GSM_PWRKEY_PIN);
    delay_ms(3000);
}

/*********************************************************************************************************
 ** Function name:       GSM_ClearBuffer()
 ** Descriptions:        清空接收缓存
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:
 *********************************************************************************************************/
void GSM_ClearBuffer(void)
{
    memset(BackBuf, 0, USART_GSM_BUFSIZE);
}
/*********************************************************************************************************
 ** Function name:       GSM_SendAT()
 ** Descriptions:        发送一条AT指令
 ** input parameters:    pCMD：指令指针
 **                      pCMDBack： 指令返回参考指针
 **                      CMDLen:  指令长度
 ** output parameters:   NONE
 ** Returned value:      发送指令返回状态
 *********************************************************************************************************/
unsigned char GSM_SendAT(char *pCMD, char *pCMDBack, uint32_t CMDLen)
{
    unsigned char i = AT_RESEND_TIMES;
    unsigned int len;
    char *pBackBuf = BackBuf;
    unsigned char retFlag = 0;

    len = CMDLen;

#ifdef DBG_ENABLE_MACRO
    {
        uint32_t tmpIdx;

        if (len > 0)
        {
            DEBUG("GSM_SendAT send\r\n");
            for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
            {
                //DEBUG("%d-%c\r\n", tmpIdx, pBackBuf[tmpIdx]);
                DEBUG("%c",pCMD[tmpIdx]);
            }
            DEBUG("\r\n");
            DEBUG("GSM_SendAT done\r\n");
        }
    }
#endif

    usart_sendbuffer(STM32_SIM908_GSM_COM, pCMD, (unsigned int *) &len);

    if (NULL != pCMDBack)
    {
        while (--i)
        {
            len = USART_GSM_BUFSIZE;
            delay_10ms(10);

            //DEBUG("GSM_SendAT before usart_readbuffer len %d\n", len);

            retFlag = usart_readbuffer(STM32_SIM908_GSM_COM, pBackBuf, &len);
            //DEBUG("GSM_SendAT after usart_readbuffer len %d\n", len);

#ifdef DBG_ENABLE_MACRO
            {
                uint32_t tmpIdx;

                if (len > 0 && retFlag == USART_ENPROCESS)
                {
                    DEBUG("GSM_SendAT recv %d bytes\r\n", len);
                    for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
                    {
                        //DEBUG("%d-%c\r\n", tmpIdx, pBackBuf[tmpIdx]);
                        DEBUG("%c",pBackBuf[tmpIdx]);
                    }
                    DEBUG("\r\n");
					
                    DEBUG("GSM_SendAT recv done\r\n");
                }
            }
#endif
            if (len > 0 && retFlag == USART_ENPROCESS)
            {
                if (NULL != strstr_len(pBackBuf, pCMDBack, len))
                {
                    break;
                }
            }
        }
		
        if (i == 0)
        {
            return USART_FAIL;
        }
    }
    return USART_SUCESS;
}


unsigned char GSM_SendAT_rsp(char *pCMD, char *pCMDBack,
        uint32_t CMDLen, char **ppRecvBuf, uint32_t *pRecvLen)
{
    unsigned char i = AT_RESEND_TIMES;
    unsigned int len;
    char *pBackBuf = BackBuf;
    unsigned char retFlag = 0;

    len = CMDLen;

#ifdef DBG_ENABLE_MACRO
        {
            uint32_t tmpIdx;

            if (len > 0)
            {
                DEBUG("GSM_SendAT send\r\n");
                for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
                {
                    //DEBUG("%d-%c\r\n", tmpIdx, pBackBuf[tmpIdx]);
                    DEBUG("%c",pCMD[tmpIdx]);
                }
                DEBUG("\r\n");
                DEBUG("GSM_SendAT done\r\n");
            }
        }
#endif

    usart_sendbuffer(STM32_SIM908_GSM_COM, pCMD, (unsigned int *) &len);

    while (--i)
    {
        len = USART_GSM_BUFSIZE;
        delay_10ms(10);

        //DEBUG("GSM_SendAT before usart_readbuffer len %d\n", len);
        retFlag = usart_readbuffer(STM32_SIM908_GSM_COM, pBackBuf, &len);
        //DEBUG("GSM_SendAT after usart_readbuffer len %d\n", len);

#ifdef DBG_ENABLE_MACRO
        {
            uint32_t tmpIdx;

            if (len > 0 && retFlag == USART_ENPROCESS)
            {
                DEBUG("GSM_SendAT recv\r\n");
                for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
                {
                    //DEBUG("%d-%c\r\n", tmpIdx, pBackBuf[tmpIdx]);
                    DEBUG("%c",pBackBuf[tmpIdx]);
                }
                DEBUG("\r\n");
                DEBUG("GSM_SendAT done\r\n");
            }
        }
#endif

        if (len > 0 && retFlag == USART_ENPROCESS)
        {
            if (ppRecvBuf != NULL)
            {
                *ppRecvBuf = pBackBuf;
            }

            if (pRecvLen != NULL)
            {
                *pRecvLen = len;
            }

            if (pCMDBack != NULL)
            {
                if (NULL
                        != strstr((const char *) pBackBuf,
                                  (const char *) pCMDBack))
                {
                    break;
                }
            }
            else {
                break;
            }
        }


        if (i == 0)
        {
            return USART_FAIL;
        }
    }
    return USART_SUCESS;
}
/*********************************************************************************************************
 ** Function name:       GSM_QueryNetType
 ** Descriptions:        查询运营商名称
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      运营商名称
 *********************************************************************************************************/
unsigned char GSM_QueryNetType(void)
{
    if (USART_SUCESS
            == GSM_SendAT((char *) AT_COPS, (char *) "MOBILE", sizeof(AT_COPS) - 1))
    {
        return CHINAMOBILE;
    }
    else if (USART_SUCESS
            == GSM_SendAT((char *) AT_COPS, (char *) "UNICOM", sizeof(AT_COPS) - 1))
    {
        return CHINAUNICOM;
    }
    else
    {
        return USART_FAIL;
    }
}
/*********************************************************************************************************
 ** Function name:       GSM_QuerySignal
 ** Descriptions:        查询信号强度
 ** input parameters:    NONE
 ** output parameters:   pSig: 信号强度
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_QuerySignal(unsigned char *pSig)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;

    cmdLen = strlen(AT_CSQ);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT((char *) AT_CSQ, (char *) AT_OK, cmdLen))
    {
        pfeed = strnchr(BackBuf, ',', 1);
        if (pfeed == NULL)
            return USART_FAIL;
        pfeed -= 2;
        if (pfeed[0] < 0x30 || pfeed[0] > 0x39)
        {
            *pSig = pfeed[1] - 0x30;
        }
        else
        {
            *pSig = (pfeed[0] - 0x30) * 10 + (pfeed[1] - 0x30);
        }
        return USART_SUCESS;
    }
    return USART_FAIL;
}

/**
  * @brief  Request TA Serial Number Identification (IMEI)
  * @param  pImei: pointer of IMEI
  * @retval STATUS
  */
unsigned char GSM_QueryImei(uint8_t *pImei)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

#if 0
	cmdLen = strlen(AT_GSN_TEST);
	while (USART_SUCESS != GSM_SendAT((char *)AT_GSN_TEST, (char *)AT_OK, cmdLen));
#endif
    cmdLen = strlen(AT_GSN);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT_rsp((char *)AT_GSN, (char *)AT_OK, cmdLen, &pRecvBuf, &recvLen))
    {
		
        pfeed = strdig_len(pRecvBuf, recvLen, 15);
        if (pfeed == NULL)
        {
            return USART_FAIL;
        }
        
		if (pImei != NULL)
        {
            memcpy(pImei, pfeed, 15);
        }
		
        return USART_SUCESS;
    }
    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GSM_QueryImsiBuf
 ** Descriptions:        查询IMSI
 ** input parameters:    NONE
 ** output parameters:   pImsiInfo
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_QueryImsiBuf(uint8_t *pImsi)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    cmdLen = strlen(AT_CIMI);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT_rsp((char *)AT_CIMI, (char *)AT_OK, cmdLen, &pRecvBuf, &recvLen))
    {
		
        pfeed = strdig_len(pRecvBuf, recvLen, 15);
        if (pfeed == NULL)
        {
            return USART_FAIL;
        }

		if (pImsi != NULL)
        {
            memcpy(pImsi, pfeed, 15);
        }
		
        return USART_SUCESS;
    }
    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GSM_QueryImsi
 ** Descriptions:        查询IMSI
 ** input parameters:    NONE
 ** output parameters:   pImsiInfo
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_QueryImsi(pST_IMSIINFO pImsiInfo)
{
	uint32_t i;
    unsigned int cmdLen;
    static char *pfeed = NULL;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;
	char tmpMcc[4];
	char tmpMnc[3];
	uint32_t tmpValue = 0;

    cmdLen = strlen(AT_CIMI);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT_rsp((char *)AT_CIMI, (char *)AT_OK, cmdLen, &pRecvBuf, &recvLen))
    {
		
        pfeed = strdig_len(pRecvBuf, recvLen, 15);
        if (pfeed == NULL)
        {
            return USART_FAIL;
        }

		for(i = 0; i < 3; i++)
		{
			tmpMcc[i] = *(pfeed+i);
		}
		tmpMcc[3] = '\0';

		for(i = 0; i < 2; i++)
		{
			tmpMnc[i] = *(pfeed+3+i);
		}
		tmpMnc[2] = '\0';

		tmpValue = atoi(tmpMcc);
		pImsiInfo->Mcc[0] = (tmpValue/255);
		if(tmpValue > 255)
		{
			pImsiInfo->Mcc[1] = (tmpValue%255 - 1);
		}
		else
		{
			pImsiInfo->Mcc[1] = (tmpValue%255);
		}

		tmpValue = atoi(tmpMnc);
		pImsiInfo->Mnc[0] = (tmpValue/255);
		if(tmpValue > 255)
		{
			pImsiInfo->Mnc[1] = (tmpValue%255 - 1);
		}
		else
		{
			pImsiInfo->Mnc[1] = (tmpValue%255);
		}
		
        return USART_SUCESS;
    }
    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GSM_QueryCREG
 ** Descriptions:        Query Location information elements
 ** input parameters:    NONE
 ** output parameters:   pCregInfo
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_QueryCreg(pST_CREGINFO pCregInfo)
{
    unsigned int cmdLen = 0;
    char *pRecvBuf = NULL;
    uint32_t recvLen = 0;
    char *pStr = NULL;
	char tmpbuf[2];
#if 0
    pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CREG_SET, 2);
    cmdLen = strlen(pcmdbuf);
    while (USART_SUCESS != GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen));
#endif
	cmdLen = strlen(AT_CREG);
	GSM_ClearBuffer();
    if(USART_SUCESS == GSM_SendAT_rsp((char *)AT_CREG, (char *) "CREG",
            							cmdLen, &pRecvBuf, &recvLen))
    {

		// lac
	    pStr = strnchr_len(pRecvBuf, '"', 1, recvLen);
		if(pStr == NULL)
		{
			return USART_FAIL;
		}
	    if (pStr != NULL)
	    {
	        tmpbuf[0] = *(pStr+1);
			tmpbuf[1] = *(pStr+2);
			pCregInfo->Lac[0] = strhex_len(tmpbuf, strlen(tmpbuf));
			tmpbuf[0] = *(pStr+3);
			tmpbuf[1] = *(pStr+4);
			pCregInfo->Lac[1] = strhex_len(tmpbuf, strlen(tmpbuf));
	    }

		// ci
	    pStr = strnchr_len(pRecvBuf, '"', 3, recvLen);
		if(pStr == NULL)
		{
			return USART_FAIL;
		}
	    if (pStr != NULL)
	    {
	        tmpbuf[0] = *(pStr+1);
			tmpbuf[1] = *(pStr+2);
			pCregInfo->Ci[0] = strhex_len(tmpbuf, strlen(tmpbuf));
			tmpbuf[0] = *(pStr+3);
			tmpbuf[1] = *(pStr+4);
			pCregInfo->Ci[1] = strhex_len(tmpbuf, strlen(tmpbuf));
	    }

		// stat
	    pStr = strnchr_len(pRecvBuf, ',', 2, recvLen);
		if(pStr == NULL)
		{
			return USART_FAIL;
		}
	    if (pStr != NULL)
	    {
			pCregInfo->Stat = *(pStr-1);
	    }

		// n
	    pStr = strnchr_len(pRecvBuf, ',', 1, recvLen);
		if(pStr == NULL)
		{
			return USART_FAIL;
		}
	    if (pStr != NULL)
	    {
			pCregInfo->n = *(pStr-1);
    	}

		return USART_SUCESS;
    }

    return USART_FAIL;
}

/**
  * @brief  Read GSM's ADC
  * @param  pSig
  * @retval Status
  */
unsigned char GSM_QueryBatVoltage(pST_BATVOLTAGESTATUS pSig)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    cmdLen = strlen(AT_CADC);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT_rsp((char *) AT_CADC, (char *) AT_OK, cmdLen, &pRecvBuf, &recvLen))
    {
        pfeed = strnchr(pRecvBuf, ',', 1);	//提取状态
        if (pfeed == NULL)
        {
            return USART_FAIL;
        }
        pSig->Status = (*(pfeed-1) - '0');

		// 分压电阻是100K和200K
		pfeed++;
		pSig->BatVoltage.i= ((atoi(pfeed)*3)/2);
        return USART_SUCESS;
    }
    return USART_FAIL;
}


/**
  * @brief  Request Phone Number
  * @param  pNum: pointer of number
  * @retval STATUS
  */
unsigned char GSM_QueryNumber(uint8_t *pNum)
{
    unsigned int cmdLen;
	char *pfeed = NULL;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    cmdLen = strlen(AT_CNUM);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT_rsp((char *)AT_CNUM, (char *)AT_OK, cmdLen, &pRecvBuf, &recvLen))
    {
		// Should Parse Number
		
        return USART_SUCESS;
    }
    return USART_FAIL;
}


/*********************************************************************************************************
 ** Function name:       GSM_SendSMS
 ** Descriptions:        发送一条短信
 ** input parameters:    pNumb: 接收短信的号码指针
 **											pSMS: 短信内容
 **											type: 短信发送格式 1-TEXT 0-PUD
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_SendSMS(char *pNumb, char *pSMS, unsigned char type)
{
    static char *pcmdbuf = NULL;
    static char smsBuffer[128];
    static char numbBuffer[15];
    static char smsLen[2];
    unsigned int cmdLen = 0;
    unsigned char i, j;

    GSM_ClearBuffer();
    pcmdbuf = sendBuf;
    if (pcmdbuf == NULL)
    {
        return USART_FAIL;
    }
    sprintf(pcmdbuf, AT_CMGF, type);
    cmdLen = strlen(pcmdbuf);
    GSM_SendAT(pcmdbuf, (char *) AT_OK, cmdLen);	 //设置短信模式

    if (1 == type)	//TEXT模式
    {
        strcpy(smsBuffer, pSMS);								//拷贝需要发送的短信内容
        GSM_ClearBuffer();
        pcmdbuf = sendBuf;
        if (pcmdbuf == NULL)
        {
            return USART_FAIL;
        }
        sprintf(pcmdbuf, AT_CMGS, pNumb);
        cmdLen = strlen(pcmdbuf);
        GSM_SendAT(pcmdbuf, ">", cmdLen);					  //设置目标号码

        delay_10ms(50);
        strcat(smsBuffer, "\x1A");					 				//添加结束符号0x1A
        cmdLen = strlen(smsBuffer);
        GSM_ClearBuffer();
        usart_sendbuffer(STM32_SIM908_GSM_COM, smsBuffer,
                (unsigned int *) &cmdLen);
        delay_10ms(200);
        cmdLen = USART_GSM_BUFSIZE;
        usart_sendbuffer(STM32_SIM908_GSM_COM, BackBuf,
                (unsigned int*) &cmdLen);
        if (NULL != strstr((const char *) BackBuf, "OK"))
        {
            return USART_SUCESS;
        }
    }
    else if (0 == type)
    {
        i = 0;
        j = 0;
        do
        {
            numbBuffer[i++] = pNumb[j + 1];
            numbBuffer[i++] = pNumb[j];
            j = j + 2;
            if (pNumb[j + 1] == 0x0d || pNumb[j + 1] == 0)
            {
                numbBuffer[i++] = 'F';
                numbBuffer[i++] = pNumb[j];
                break;
            }
        } while (j < 11);
        strcpy(smsBuffer, "0011000D9168");
        strcat(smsBuffer, (const char *) &numbBuffer);
        strcat(smsBuffer, "0008A0");
        cmdLen = strlen(pSMS) / 2;
        sprintf(smsLen, "%X%X", cmdLen / 16, cmdLen % 16);
        strcat(smsBuffer, smsLen);
        strcat(smsBuffer, pSMS);

        GSM_ClearBuffer();
        pcmdbuf = sendBuf;
        if (pcmdbuf == NULL)
        {
            return USART_FAIL;
        }
        cmdLen = strlen(smsBuffer) / 2 - 1;
        sprintf(pcmdbuf, AT_CMGSPDU, cmdLen);
        cmdLen = strlen(pcmdbuf);
        GSM_SendAT(pcmdbuf, ">", cmdLen);					  //设置目标号码

        delay_10ms(50);

        strcat(smsBuffer, "\x1A");
        cmdLen = strlen(smsBuffer);
        GSM_ClearBuffer();
        usart_sendbuffer(STM32_SIM908_GSM_COM, smsBuffer,
                (unsigned int *) &cmdLen);
        delay_10ms(200);
        cmdLen = USART_GSM_BUFSIZE;
        usart_sendbuffer(STM32_SIM908_GSM_COM, BackBuf,
                (unsigned int*) &cmdLen);
        if (NULL != strstr((const char *) BackBuf, "OK"))
        {
            return USART_SUCESS;
        }
    }
    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GSM_CheckSIMCard
 ** Descriptions:        Check SIM CARD, inifite while loop until sim card is OK, and query operator
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      NONE
 *********************************************************************************************************/
void GSM_CheckSIMCard(void)
{
    uint32_t i;
    uint32_t len;

    //查询卡状态
    i = 0;
    len = strlen(AT_CPIN);
    while (USART_SUCESS != GSM_SendAT((char *) AT_CPIN, (char *)AT_READY, len))
    {
        delay_10ms(20);
        i++;

        if (i > RESEND_TIMES_CPIN)
        {
            break;
        }
    }
    i = 0;
    GSMNetType = USART_FAIL;
    //查询网络运营商
    while (USART_FAIL == GSMNetType)
    {
        GSMNetType = GSM_QueryNetType();
		i++;
		
        if (i > RESEND_TIMES_COPS)
        {
            break;
        }
    }
	
}

/**
  * @brief  Check Network Registration
  * @param  None
  * @retval None
  */
void GSM_CheckNetworkReg(void)
{
    unsigned int cmdLen = 0;
    char *pRecvBuf = NULL;
    uint32_t recvLen = 0;
    char *pStr;
	uint32_t num = 0;

	cmdLen = strlen(AT_CREG);
    while (num < 11)
    {
        if (USART_SUCESS == GSM_SendAT_rsp((char *)AT_CREG, (char *) "CREG",
                cmdLen, &pRecvBuf, &recvLen))
        {
			pStr = strnchr_len(pRecvBuf, ',', 1, recvLen);
	        if (pStr != NULL)
	        {
				// 1 Registered, home network
	            if (*(pStr+1) == '1')
	            {
	                break;
	            }
	        }
		}
		
		num++;
		delay_10ms(20);
    }
}

/**
  * @brief  Set Network Registration
  * 2 Enable network registration unsolicited result code with
  * location information +CREG: <stat>[,<lac>,<ci>]
  * @param  None
  * @retval None
  */
void GSM_SetNetworkReg(void)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;
	uint32_t errNum = 0;

    pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CREG_SET, 2);
    cmdLen = strlen(pcmdbuf);
    while (USART_SUCESS != GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen))
	{
		if(errNum > 5)
		{
			break;
		}
    }
}

/**
  * @brief  Get TCPIP Application Mode
  * 0 Normal mode; 1 Transparent mode
  * @param  None
  * @retval None
  */
unsigned char GSM_QueryCIPMode(unsigned char *pMode)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;

    cmdLen = strlen(AT_CIPMODE_GET);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT((char *) AT_CIPMODE_GET, (char *) AT_OK, cmdLen))
    {
        pfeed = strnchr(BackBuf, ':', 1);
        if (pfeed == NULL)
            return USART_FAIL;
        pfeed++;
        *pMode = atoi(pfeed);
        return USART_SUCESS;
    }
    return USART_FAIL;
}

/**
  * @brief  Select TCPIP Application Mode
  * 0 Normal mode; 1 Transparent mode
  * @param  None
  * @retval None
  */
void GSM_SetCIPMode(unsigned char mode)
{
	static char *pcmdbuf = NULL;
	unsigned int cmdLen = 0;
	unsigned char preMode = 0;
	uint32_t errNum = 0;

	// get cpimode
	GSM_QueryCIPMode(&preMode);
	// Select TCPIP application mode is normal mode
	if(preMode != mode)
	{
		pcmdbuf = sendBuf;
	    sprintf(pcmdbuf, AT_CIPMODE_SET, mode);
		cmdLen = strlen(pcmdbuf);
	    while(USART_SUCESS != GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen))
		{
			if(errNum > 5)
			{
				break;
			}
	    }
	}
}


/**
  * @brief  Check Attach or Detach from GPRS Service Status
  * @param  None
  * @retval None
  */
void GSM_CheckGPRSService(void)
{
    char *pRecvBuf = NULL;
    uint32_t recvLen = 0;
	uint32_t errNum = 0;

    char *pStr;

    while (1)
    {
        while (USART_SUCESS != GSM_SendAT_rsp((char *) AT_CGATT, (char *) "CGATT",
                sizeof(AT_CGATT), &pRecvBuf, &recvLen));
		
        // analyze CGATT rsp 1 Attached; 0 Detached
        pStr = strnchr_len(pRecvBuf, ':', 1, recvLen);
        if (pStr != NULL)
        {
            do {
                pStr ++;
            } while (*pStr == ' ');

            if (*pStr == '1')
            {
                break;
            }
        }
		errNum++;
		if(errNum > 5)
		{
			break;
		}
    }

}

/**
  * @brief  Start Task and Set APN, USER NAME, PASSWORD
  * @param  None
  * @retval None
  */
void GSM_StartTaskAndSetAPN(void)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;
	uint32_t errNum = 0;

    cmdLen = strlen(AT_CSTT);
    GSM_ClearBuffer();

	if(USART_SUCESS == GSM_SendAT_rsp((char *)AT_CSTT, (char *)AT_OK, cmdLen, &pRecvBuf, &recvLen))
	{
		if(NULL != strstr_len(pRecvBuf, (char *)"CMNET", recvLen))
		{
			return;
		}
	}

    pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CSTT_SET, "\"CMNET\"");
    cmdLen = strlen(pcmdbuf);
    while (USART_SUCESS != GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen))
	{
		errNum++;
		if(errNum > 5)
		{
			break;
		}
    }
}

/**
  * @brief  Get Local IP Address
  * @param  None
  * @retval None
  */
void GSM_GetLocalIP(void)
{
	unsigned int cmdLen = 0;
	uint32_t errNum = 0;

	cmdLen = strlen(AT_CIFSR);
    while (USART_SUCESS != GSM_SendAT((char *) AT_CIFSR, NULL, cmdLen))
	{
		errNum++;
		if(errNum > 5)
		{
			break;
		}
    }
}

/**
  * @brief  Bring Up Wireless Connection with GPRS or CSD
  * @param  None
  * @retval None
  */
void GSM_BringUpConnect(void)
{
	unsigned int cmdLen = 0;
	uint32_t errNum = 0;
	
	cmdLen = strlen(AT_CIICR);
    while (USART_SUCESS != GSM_SendAT((char *) AT_CIICR, (char *) AT_OK, cmdLen))
	{
		errNum++;
		if(errNum> 5)
		{
			break;
		}
    }
}

/**
  * @brief  Start Up TCP or UDP Connection
  * @param  None
  * @retval None
  */
void GSM_StartUpConnect(void)
{
    ST_NETWORKCONFIG stNetCfg;
	uint32_t errNum = 0;

	sprintf(stNetCfg.TransferMode, "%s", "TCP");
#ifdef FACTORY_ENABLE_MACRO
	sprintf(stNetCfg.RemoteIP, "%s", GSM_FACTORY_IP);
    sprintf(stNetCfg.RemotePort, "%s", GSM_FACTORY_PORT);
#else
    sprintf(stNetCfg.RemoteIP, "%s", GSM_SERVER_IP);
    sprintf(stNetCfg.RemotePort, "%s", GSM_SERVER_PORT);
#endif // FACTORY_ENABLE_MACRO
    while (USART_SUCESS != GPRS_LinkServer(&stNetCfg))
	{
		errNum++;
		if(errNum > 5)
		{
			break;
		}
    }
}


void GSM_Init(void)
{
    unsigned char i = 0;
    unsigned char len;
	uint32_t errNum = 0;

    // AT握手
    len = strlen(AT_Cmd);
	GSM_ClearBuffer();
    while (USART_SUCESS != GSM_SendAT((char *) AT_Cmd, (char *) AT_OK, len))
    {
        delay_10ms(30);
        i++;
		errNum++;
        if (i > 2)
        {
            GSM_TurnOnOff();
            //GSM_TurnOnOff_delay();
            i = 0;
        }
		if(errNum > 12)
		{
			break;
		}
    }

    //关闭回显
    len = strlen(ATE0_Cmd);
    GSM_SendAT((char *) ATE0_Cmd, (char *) AT_OK, len);

    //GSM_CheckSIMCard();
    //GPS_Init();
}

/*********************************************************************************************************
 ** Function name:       GSM_CallNumber
 ** Descriptions:        呼叫一个电话号码
 ** input parameters:    pNumber: 呼叫的电话号码指针
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_CallNumber(char *pNumber)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;

    pcmdbuf = sendBuf;
    if (NULL == pcmdbuf)
    {
        return USART_FAIL;
    }
    sprintf(pcmdbuf, ATD_Cmd, pNumber);
    cmdLen = strlen(pcmdbuf);
    if (USART_SUCESS == GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen))
    {
        return USART_SUCESS;
    }

    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GSM_ReadNumberFromSIM
 ** Descriptions:        SIM卡中读取一个电话号码
 ** input parameters:    No: SIM卡中电话号码位置
 ** output parameters:   pNumber: 读取到的电话号码
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_ReadNumberFromSIM(unsigned char No, ST_PHONEBOOKS *pNumber)
{
    static char *pcmdbuf = NULL;
    static char *pfeed_per = NULL, *pfeed_next = NULL;
    unsigned int cmdLen = 0;

    if (No == 0 || No > 250)
    {
        return USART_FAIL;
    }
    pcmdbuf = sendBuf;
    if (NULL == pcmdbuf)
    {
        return USART_FAIL;
    }
    sprintf(pcmdbuf, AT_CPBR, No);
    cmdLen = strlen(pcmdbuf);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT((char *) pcmdbuf, (char *) "+CPBR", cmdLen))
    {
        pfeed_per = strnchr(BackBuf, '"', 1);
        pfeed_next = strnchr(BackBuf, '"', 2);
        if ((NULL == pfeed_per) || (NULL == pfeed_next))
        {

            return USART_FAIL;
        }
        memcpy(&(pNumber->PhoneNumber), pfeed_per + 1,
                abs(pfeed_next - pfeed_per - 1));

        pfeed_per = strnchr(BackBuf, '"', 3);
        pfeed_next = strnchr(BackBuf, '"', 4);
        if ((NULL == pfeed_per) || (NULL == pfeed_next))
        {

            return USART_FAIL;
        }
        memcpy(&(pNumber->Name), pfeed_per + 1,
                abs(pfeed_next - pfeed_per - 1));

        return USART_SUCESS;
    }

    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GSM_CallSIMNumber
 ** Descriptions:        呼叫SIM卡中的一个电话号码
 ** input parameters:    numberNo: SIM卡中电话号码位置
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_CallSIMNumber(unsigned char numberNo)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen;

    if (numberNo > 250 || numberNo == 0)
    {
        return USART_FAIL;
    }
    pcmdbuf = sendBuf;
    if (NULL == pcmdbuf)
    {
        return USART_FAIL;
    }
    sprintf(pcmdbuf, ATD_n, numberNo);
    cmdLen = strlen(pcmdbuf);
    if (USART_SUCESS == GSM_SendAT(pcmdbuf, (char *) AT_OK, cmdLen))
    {

        return USART_SUCESS;
    }
    else
    {

        return USART_FAIL;
    }
}
/*********************************************************************************************************
 ** Function name:       GSM_QueryCallStatus
 ** Descriptions:        查询呼叫状态
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回呼叫状态
 *********************************************************************************************************/
unsigned char GSM_QueryCallStatus(void)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;
    char status = AT_ACTIVE;

    cmdLen = strlen(AT_CLCC);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT((char *) AT_CLCC, (char *) AT_OK, cmdLen))
    {
        pfeed = strnchr(BackBuf, ',', 2);
        if (pfeed == NULL)
            return USART_FAIL;
        if (NULL != pfeed)
        {
            status = *(pfeed + 1);
            return status;
        }
    }
    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GSM_HangCall
 ** Descriptions:        挂断通话
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_HangCall(void)
{
    unsigned int cmdLen;

    cmdLen = strlen(ATH_Cmd);
    if (USART_SUCESS == GSM_SendAT((char *) ATH_Cmd, (char *) AT_OK, cmdLen))
    {
        return USART_SUCESS;
    }
    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GSM_AnswerCall
 ** Descriptions:        接听通话
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_AnswerCall(void)
{
    unsigned int cmdLen;

    cmdLen = strlen(ATA_Cmd);
    if (USART_SUCESS == GSM_SendAT((char *) ATA_Cmd, (char *) AT_OK, cmdLen))
    {
        return USART_SUCESS;
    }
    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GSM_GetRTCTime
 ** Descriptions:        查询RTC时钟时间
 ** input parameters:    NONE
 ** output parameters:   rtctime: 返回时间信息
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_GetRTCTime(pST_RTCTIME rtctime)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;

    cmdLen = strlen(AT_CCLK);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT((char *) AT_CCLK, (char *) AT_OK, cmdLen))
    {
        pfeed = strnchr(BackBuf, ',', 1);
        if (pfeed == NULL)
            return USART_FAIL;
        pfeed -= 8;
        rtctime->Year = (pfeed[0] - 0x30) * 10 + (pfeed[1] - 0x30);
        rtctime->Month = (pfeed[3] - 0x30) * 10 + (pfeed[4] - 0x30);
        rtctime->Day = (pfeed[6] - 0x30) * 10 + (pfeed[7] - 0x30);
        rtctime->Hour = (pfeed[9] - 0x30) * 10 + (pfeed[10] - 0x30);
        rtctime->Min = (pfeed[12] - 0x30) * 10 + (pfeed[13] - 0x30);
        rtctime->Sec = (pfeed[15] - 0x30) * 10 + (pfeed[16] - 0x30);
        return USART_SUCESS;
    }
    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GSM_SetRTCTime
 ** Descriptions:        设置RTC时钟时间
 ** input parameters:    rtctime: 需要设置的时间信息
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_SetRTCTime(ST_RTCTIME rtctime)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;

    GSM_ClearBuffer();
    pcmdbuf = sendBuf;
    if (pcmdbuf == NULL)
    {
        return USART_FAIL;
    }
    //格式化时间
    sprintf(pcmdbuf, AT_CCLKSet, (rtctime.Year) / 10, (rtctime.Year) % 10,
            (rtctime.Month) / 10, (rtctime.Month) % 10, (rtctime.Day) / 10,
            (rtctime.Day) % 10, (rtctime.Hour) / 10, (rtctime.Hour) % 10,
            (rtctime.Min) / 10, (rtctime.Min) % 10, (rtctime.Sec) / 10,
            (rtctime.Sec) % 10, (rtctime.Timezone) / 10,
            (rtctime.Timezone) % 10);
    cmdLen = strlen(pcmdbuf);
    if (USART_SUCESS == GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen))
    {

        return USART_SUCESS;
    }

    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GSM_SetSClk
 ** Descriptions:        设置Slow Clock
 ** input parameters:    value: 需要设置的clock信息 2 - enable slow clock auto
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_SetSClk(uint32_t value)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;

    GSM_ClearBuffer();
    pcmdbuf = sendBuf;
    if (pcmdbuf == NULL)
    {
        return USART_FAIL;
    }

	pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CSCLK_SET, 2);
    cmdLen = strlen(pcmdbuf);
    if (USART_SUCESS == GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen))
	{
		return USART_SUCESS;
    }
	
    return USART_FAIL;
}

void GetGsmData(pST_SIMDATA pSimData, ST_IMSIINFO imsi)
{
	uint32_t i;
	ST_CREGINFO creg;
	ST_BATVOLTAGESTATUS battery;
	unsigned char signal = 0;

	//while(USART_SUCESS != GSM_QueryImsi(&imsi));
	if(USART_SUCESS != GSM_QueryCreg(&creg))
	{
		DEBUG("Query CREG Fail\n");
	}
	if(USART_SUCESS != GSM_QueryBatVoltage(&battery))
	{
		DEBUG("Query Bat Voltage Fail\n");
	}
	if(USART_SUCESS != GSM_QuerySignal(&signal))
	{
		DEBUG("Query Signal Quality Fail\n");
	}

	pSimData->Station[0] = imsi.Mcc[0];
	pSimData->Station[1] = imsi.Mcc[1];
	pSimData->Station[2] = imsi.Mnc[0];
	pSimData->Station[3] = imsi.Mnc[1];
	pSimData->Station[4] = creg.Lac[0];
	pSimData->Station[5] = creg.Lac[1];
	pSimData->Station[6] = 0x00;  // 补零
	pSimData->Station[7] = creg.Ci[0];
	pSimData->Station[8] = creg.Ci[1];

	// when status is valid then parse
	if(1 == battery.Status)
	{
		pSimData->Battery[0] = battery.BatVoltage.s[1];
		pSimData->Battery[1] = battery.BatVoltage.s[0];
	}

	sprintf(pSimData->Signal, "%x", signal);

#ifdef DBG_ENABLE_MACRO
	DEBUG("STATION:");
	for(i = 0; i < 9; i++)
	{
		DEBUG("0x%x-", pSimData->Station[i]);
	}
	DEBUG("\n");
	DEBUG("BATTERY:");
	for(i = 0; i < 2; i++)
	{
		DEBUG("0x%x-", pSimData->Battery[i]);
	}
	DEBUG("\n");
	DEBUG("SIGNAL:");
	for(i = 0; i < 2; i++)
	{
		DEBUG("0x%x-", pSimData->Signal[i]);
	}
	DEBUG("\n");
#endif
}


/*********************************************************************************************************
 ** Function name:       GPRS_Init
 ** Descriptions:      	GPRS初始化
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      NONE
 *********************************************************************************************************/
void GPRS_Init(void)
{
    unsigned int cmdLen;

    cmdLen = strlen(AT_CIPHEAD);
    GSM_SendAT((char *) AT_CIPHEAD, (char *) AT_OK, cmdLen);
    cmdLen = strlen(AT_CIPQSEND);
    GSM_SendAT((char *) AT_CIPQSEND, (char *) AT_OK, cmdLen);
    cmdLen = strlen(AT_CIPSPRT);
    GSM_SendAT((char *) AT_CIPSPRT, (char *) AT_OK, cmdLen);
    cmdLen = strlen(AT_CSTT);
    GSM_SendAT((char *) AT_CSTT, (char *) AT_OK, cmdLen);
    cmdLen = strlen(AT_CIICR);
    GSM_SendAT((char *) AT_CIICR, (char *) AT_OK, cmdLen);
    cmdLen = strlen(AT_CIPMODE);
    GSM_SendAT((char *) AT_CIPMODE, (char *) AT_OK, cmdLen);
}

/*********************************************************************************************************
 ** Function name:       GPRS_LinkServer
 ** Descriptions:      	建立一个链接
 ** input parameters:    pnetconfig: 网络配置信息
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GPRS_LinkServer(pST_NETWORKCONFIG pnetconfig)
{
    ST_NETWORKCONFIG netcfg = *pnetconfig;
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;
	char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    pcmdbuf = sendBuf;
    if (pcmdbuf == NULL)
    {
        return USART_FAIL;
    }
    sprintf(pcmdbuf, AT_CIPSTART, netcfg.TransferMode, netcfg.RemoteIP,
            netcfg.RemotePort);
    cmdLen = strlen(pcmdbuf);
    if (USART_SUCESS == GSM_SendAT_rsp((char *) pcmdbuf, (char *) AT_CONNECTOK, cmdLen, &pRecvBuf, &recvLen))
    {
        //return USART_SUCESS;
        if(NULL != strstr_len(pRecvBuf, (char *)AT_CONNECTOK, recvLen))
        {
			return USART_SUCESS;
        }
		else if(NULL != strstr_len(pRecvBuf, "ALREADY CONNECT", recvLen))
		{
			return USART_SUCESS;
		}
    }
	

    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GPRS_SendData
 ** Descriptions:      	GPRS非透传模式下发送数据
 ** input parameters:    pString: 需要发送的数据指针
 **											len:	需要发送的数据长度
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GPRS_SendData(char *pString, unsigned int len)
{
	char *pBackBuf = BackBuf;
    unsigned int cmdLen;

    cmdLen = strlen(AT_CIPSEND);
    if (USART_SUCESS == GSM_SendAT((char *) AT_CIPSEND, (char *) '>', cmdLen))
    {
        cmdLen = sizeof(pString);
        usart_sendbuffer(STM32_SIM908_GSM_COM, pString, &len);
        cmdLen = 1;
        usart_sendbuffer(STM32_SIM908_GSM_COM, "\x1A", &cmdLen);
        //return USART_SUCESS;
    }

    {
        uint32_t i = 8;
        uint32_t len;
        unsigned char retFlag;

        while (--i)
        {
            len = USART_GSM_BUFSIZE;
            delay_10ms(20);

            //DEBUG("GSM_SendAT before usart_readbuffer len %d\n", len);

            retFlag =
                    usart_readbuffer(STM32_SIM908_GSM_COM, BackBuf, &len);
            //DEBUG("GSM_SendAT after usart_readbuffer len %d\n", len);

#ifdef DBG_ENABLE_MACRO
            {
                uint32_t tmpIdx;
				uint32_t tmpStart;

                if (len > 0 && retFlag == USART_ENPROCESS)
                {
                    DEBUG("GPRS_SendData recv\r\n");
                    for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
                    {
                        //DEBUG("%d-%c\r\n", tmpIdx, pBackBuf[tmpIdx]);
                        DEBUG("%c",BackBuf[tmpIdx]);
						
                    }
                    DEBUG("\r\n");
                    DEBUG("GPRS_SendData done\r\n");
                }
            }
			DEBUG("%d retFlag = %d len = %d\n\n", i, retFlag, len);
#endif
			if (len > 0 && retFlag == USART_ENPROCESS)
            {
                if (NULL != strstr_len(pBackBuf, "SEND OK", len))
                {
                    break;
                }
            }

        }
        if (i == 0)
        {
            return USART_FAIL;
        }
    }
    return USART_SUCESS;
}

/**
  * @brief  Send GPRS Data
  * @param  None
  * @retval None
  */
unsigned char GPRS_SendData_rsp(char *pString, unsigned int len, char **ppRecvBuf, uint32_t *pRecvLen)
{
	char *pBackBuf = BackBuf;
    unsigned int cmdLen;

    cmdLen = strlen(AT_CIPSEND);
    if (USART_SUCESS == GSM_SendAT((char *) AT_CIPSEND, (char *) '>', cmdLen))
    {
        cmdLen = sizeof(pString);
        usart_sendbuffer(STM32_SIM908_GSM_COM, pString, &len);
        cmdLen = 1;
        usart_sendbuffer(STM32_SIM908_GSM_COM, "\x1A", &cmdLen);
        //return USART_SUCESS;
    }

    {
        uint32_t i = 10;
        uint32_t len;
        unsigned char retFlag;

        while (--i)
        {
            len = USART_GSM_BUFSIZE;
            delay_10ms(20);

            //DEBUG("GSM_SendAT before usart_readbuffer len %d\n", len);

            retFlag =
                    usart_readbuffer(STM32_SIM908_GSM_COM, pBackBuf, &len);
            //DEBUG("GSM_SendAT after usart_readbuffer len %d\n", len);

#ifdef DBG_ENABLE_MACRO
            {
                uint32_t tmpIdx;
				uint32_t tmpStart;

                if (len > 0 && retFlag == USART_ENPROCESS)
                {
                    DEBUG("GPRS_SendData_rsp recv\r\n");
                    for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
                    {
                        //DEBUG("%c\r\n", pBackBuf[tmpIdx]);
                        DEBUG(" [%d] 0x%x-",tmpIdx, pBackBuf[tmpIdx]);
						
                    }
                    DEBUG("\r\n");
                    DEBUG("GPRS_SendData_rsp done\r\n");
                }
            }
			//DEBUG("%d retFlag = %d len = %d\n\n", i, retFlag, len);
#endif
			if (len > 0 && retFlag == USART_ENPROCESS)
            {
				if (ppRecvBuf != NULL)
	            {
	                *ppRecvBuf = pBackBuf;
	            }

	            if (pRecvLen != NULL)
	            {
	                *pRecvLen = len;
	            }
				
                if (NULL != strstr_len(pBackBuf, "SEND OK", len))
                {
                    break;
                }
            }

        }
        if (i == 0)
        {
            return USART_FAIL;
        }
    }
    return USART_SUCESS;
}

/*********************************************************************************************************
 ** Function name:       GPRS_ReceiveData
 ** Descriptions:      	GPRS接收数据,SIM908直接串口接收,不需要发送指令读取
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GPRS_ReceiveData(char *pString)
{
	uint32_t i = 5;
    uint32_t len;
    unsigned char retFlag;
	char *pReceiveBuf = receiveBuf;
	
    while (--i)
    {
        len = USART_GSM_BUFSIZE_RECEIVE;
        delay_10ms(20);

        //DEBUG("GSM_SendAT before usart_readbuffer len %d\n", len);

        retFlag =
                usart_readbuffer(STM32_SIM908_GSM_COM, receiveBuf, &len);
        //DEBUG("GSM_SendAT after usart_readbuffer len %d\n", len);

#ifdef DBG_ENABLE_MACRO
        {
            uint32_t tmpIdx;
			

            if (len > 0 && retFlag == USART_ENPROCESS)
            {
                DEBUG("GPRS_ReceiveData recv\r\n");
                for (tmpIdx = 0; tmpIdx < len; tmpIdx ++)
                {
                    //DEBUG("%d-%c\r\n", tmpIdx, pBackBuf[tmpIdx]);
                    DEBUG("%c",receiveBuf[tmpIdx]);
					
                }
                DEBUG("\r\n");
                DEBUG("GPRS_ReceiveData done\r\n");
            }
        }
		DEBUG("%d retFlag = %d len = %d\n\n", i, retFlag, len);

		if (len > 0 && retFlag == USART_ENPROCESS)
        {
            if (NULL != strstr_len(pReceiveBuf, pString, len))
            {
				DEBUG("Receive New Message.\n");
                break;
            }
        }
        
#endif
    }

    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GPRS_CheckLinkStatus
 ** Descriptions:      	查询GPRS链接状态
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回GPRS链接状态结果
 *********************************************************************************************************/
unsigned char GPRS_CheckLinkStatus(void)
{
    return USART_FAIL;
}

/**
  * @brief  Close TCP or UDP Connection
  * 1 Quick close
  * @param  None
  * @retval Status
  */
unsigned char GPRS_CloseLink(void)
{
    unsigned int cmdLen;

    cmdLen = strlen(AT_CIPCLOSE);
    if (USART_SUCESS
            == GSM_SendAT((char *) AT_CIPCLOSE, (char *) AT_OK, cmdLen))
    {
        return USART_SUCESS;
    }
    return USART_FAIL;
}
/*********************************************************************************************************
 ** Function name:       GPRS_CIPShut
 ** Descriptions:      	退出GPRS链接
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GPRS_CIPShut(void)
{
    unsigned int cmdLen;

    cmdLen = strlen(AT_CIPSHUT);
    if (USART_SUCESS
            == GSM_SendAT((char *) AT_CIPSHUT, (char *) AT_OK, cmdLen))
    {
        return USART_SUCESS;
    }
    return USART_FAIL;
}

unsigned char GPRS_CPOwd(void)
{
	char *pcmdbuf = NULL;
    unsigned int cmdLen;

	//sprintf(pcmdbuf, AT_CPOWD, 1);
    cmdLen = strlen(AT_CPOWD);
    if (USART_SUCESS
            == GSM_SendAT((char *) AT_CPOWD, (char *) "POWER", cmdLen))
    {
        return USART_SUCESS;
    }
    return USART_FAIL;
}

/*********************************************************************************************************
 ** Function name:       GSM_QueryBattery
 ** Descriptions:      	查询电池状态
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      返回状态结果
 *********************************************************************************************************/
unsigned char GSM_QueryBattery(pST_BATTERYSTATUS pSig)
{
    unsigned int cmdLen;
    static char *pfeed = NULL;

    cmdLen = strlen(AT_CBC);
    GSM_ClearBuffer();
    if (USART_SUCESS == GSM_SendAT((char *) AT_CBC, (char *) AT_OK, cmdLen))
    {
        pfeed = strnchr(BackBuf, ',', 1);	//提取充电状态
        if (pfeed == NULL)
        {
            return USART_FAIL;
        }
        pfeed -= 1;
        pSig->BatStatus = pfeed[0];

        pfeed = strnchr(BackBuf, ',', 2);	//提取电池电量信息
        if (pfeed == NULL)
        {
            return USART_FAIL;
        }
		
		pSig->BatVoltage.i= (pfeed[1] - 0x30)*1000 + (pfeed[2] - 0x30)*100 + (pfeed[3] - 0x30)*10 + (pfeed[4] - 0x30);
		
        pfeed -= 3;
        if (pfeed[0] >= 0x30 && pfeed[0] <= 0x39)
        {
            pSig->BatPower = (pfeed[0] - 0x30) * 100 + (pfeed[1] - 0x30) * 10
                    + (pfeed[2] - 0x30);
        }
        else
        {
            pSig->BatPower = (pfeed[1] - 0x30) * 10 + (pfeed[2] - 0x30);
        }

		
        return USART_SUCESS;
    }
    return USART_FAIL;
}

unsigned char GSM_creg(void)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;
    char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    char *pStr;

    pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CREG_SET, 2);
    cmdLen = strlen(pcmdbuf);
    while (USART_SUCESS != GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen));

    while (1)
    {
        while (USART_SUCESS != GSM_SendAT_rsp((char *)AT_CREG, (char *) "CREG",
                sizeof(AT_CREG), &pRecvBuf, &recvLen));

        pStr = strnchr_len(pRecvBuf, ':', 1, recvLen);
        if (pStr != NULL)
        {
            do {
                pStr ++;
            } while (*pStr == ' ');

            if (*pStr == '2')
            {
                break;
            }
        }
    }

    // analyze AT_CREG rsp


    return USART_SUCESS;
}

unsigned char GSM_cgatt(void)
{
    char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    char *pStr;

    while (1)
    {
        while (USART_SUCESS != GSM_SendAT_rsp((char *) AT_CGATT, (char *) "CGATT",
                sizeof(AT_CGATT), &pRecvBuf, &recvLen));

        // analyze CGATT rsp
        pStr = strnchr_len(pRecvBuf, ':', 1, recvLen);
        if (pStr != NULL)
        {
            do {
                pStr ++;
            } while (*pStr == ' ');

            if (*pStr == '1')
            {
                break;
            }
        }
    }

    return USART_SUCESS;
}

unsigned char GSM_cstt(void)
{
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;

    pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CSTT_SET, "\"CMNET\"");
    cmdLen = strlen(pcmdbuf);
    while (USART_SUCESS != GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen));

    return USART_SUCESS;
}

unsigned char GSM_ciicr(void)
{
    while (USART_SUCESS != GSM_SendAT((char *) AT_CIICR, (char *) AT_OK, sizeof(AT_CIICR)));

    return USART_SUCESS;
}

unsigned char GSM_cifsr(void)
{
    char *pRecvBuf = NULL;
    uint32_t recvLen = 0;

    while (USART_SUCESS != GSM_SendAT_rsp((char *) AT_CIFSR, NULL,
            sizeof(AT_CIFSR), &pRecvBuf, &recvLen));

    // analyze CGATT rsp


    return USART_SUCESS;
}

unsigned char GSM_cgpsstatus(void)
{
    //char *pRecvBuf = NULL;

    while (USART_SUCESS != GSM_SendAT((char *) AT_CGPSSTATUS,
            (char *) NULL/*AT_OK*/, sizeof(AT_CGPSSTATUS)));

    // analyze CGATT rsp


    return USART_SUCESS;
}

unsigned char GSM_ceng(void)
{
    //char *pRecvBuf = NULL;
    static char *pcmdbuf = NULL;
    unsigned int cmdLen = 0;

    GSM_ClearBuffer();
    pcmdbuf = sendBuf;
    if (pcmdbuf == NULL)
    {
        return USART_FAIL;
    }

	pcmdbuf = sendBuf;
    sprintf(pcmdbuf, AT_CENG_SET, 2);
    cmdLen = strlen(pcmdbuf);
    GSM_SendAT((char *) pcmdbuf, (char *) AT_OK, cmdLen);


    while (USART_SUCESS != GSM_SendAT((char *) AT_CENG,
            (char *) NULL/*AT_OK*/, sizeof(AT_CENG)));

    // analyze CGATT rsp


    return USART_SUCESS;
}

void GPRS_Init_Interface(void)
{
    unsigned int cmdLen;
	ST_NETWORKCONFIG stNetCfg;

    sprintf(stNetCfg.TransferMode, "%s", "TCP");
    sprintf(stNetCfg.RemoteIP, "%s", "121.40.200.84");
    sprintf(stNetCfg.RemotePort, "6666");

	// Select TCPIP application mode is normal mode
	cmdLen = strlen(AT_CIPMODE_0);
    while(USART_SUCESS != GSM_SendAT((char *) AT_CIPMODE_0, (char *) AT_OK, cmdLen));
	GSM_cgatt();
    GSM_cstt();
    GSM_ciicr();
    GSM_cifsr();
    while (USART_SUCESS != GPRS_LinkServer(&stNetCfg));
}

char str[30] = "123456789012345678901234567890";
void GSM_str_test(void)
{
    char *retPtr;

    retPtr = strstr_len(str, "2", 1);
    DEBUG("GSM_str_test %d, %d, %d\n", retPtr-str, retPtr, str);
    retPtr = strstr_len(str, "2", 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);
    retPtr = strstr_len(str, "23456", 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);
    retPtr = strstr_len(str, str, 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);


    retPtr = strnchr_len(str, '1', 1, 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);
    retPtr = strnchr_len(str, '1', 2, 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);
    retPtr = strnchr_len(str, '1', 3, 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);
    retPtr = strnchr_len(str, '1', 4, 30);
    DEBUG("GSM_str_test %d\n", retPtr-str);
}

s8 Test_TelNumber[12]="15306568880\0";
void GSM_test_once(void)
{
	unsigned int cmdLen;
	char str[24];
    ST_NETWORKCONFIG stNetCfg;
//[67 67 01  00 0C 00 01 01 23  45 67 89 01 23 45 00 20]
	str[0] = 0x67;
	str[1] = 0x67;
	str[2] = 0x01;
	str[3] = 0x00;
	str[4] = 0x0C;
	str[5] = 0x00;
	str[6] = 0x01;
	str[7] = 0x01;
	str[8] = 0x23;
	str[9] = 0x45;
	str[10] = 0x67;
	str[11] = 0x89;
	str[12] = 0x01;
	str[13] = 0x23;
	str[14] = 0x45;
	str[15] = 0x00;
	str[16] = 0x20;

	  sprintf(stNetCfg.TransferMode, "%s", "TCP");
    sprintf(stNetCfg.RemoteIP, "%s", "121.40.200.84");
    sprintf(stNetCfg.RemotePort, "6666");
    //while (USART_SUCESS != GSM_CallNumber((char*)Test_TelNumber));

	cmdLen = strlen(AT_CIPMODE_0);
    GSM_SendAT((char *) AT_CIPMODE_0, (char *) AT_OK, cmdLen);
	
    GSM_creg();
    GSM_cgatt();
    GSM_cstt();
    GSM_ciicr();
    GSM_cifsr();

#if 1
    while (USART_SUCESS != GPRS_LinkServer(&stNetCfg));

	while(1)
	{
    	if(USART_FAIL == GPRS_SendData(str, 17))
    	{
			DEBUG("GPRS_SendData Fail\n");
    	}
		delay_10ms(1000);
	}
#endif
}


void GSM_test(void)
{
	int i;
	uint8_t signal = 0;
    //GSM_cgpsstatus();
    uint8_t imeibuf[IMEI_BUFSIZE];
	ST_BATTERYSTATUS battery;
	ST_IMSIINFO imsi;
	ST_CREGINFO creg;
	//unsigned int cmdLen;
    while(USART_SUCESS != GSM_QueryImei(imeibuf));
	
	for(i = 0; i < 15; i++)
	{
		DEBUG("0x%x - %c;", imeibuf[i],imeibuf[i]);
	}
	DEBUG("\n");
	
	while(USART_SUCESS != GSM_QuerySignal(&signal));
	DEBUG("signal = %d\n", signal);

	while(USART_SUCESS != GSM_QueryBattery(&battery));
	DEBUG("power = %d;status=%d voltage[0]=0x%x [1] = 0x%x\n", battery.BatPower,battery.BatStatus, battery.BatVoltage.s[0], battery.BatVoltage.s[1]);

// cimi
	
	while (USART_SUCESS != GSM_QueryImsi(&imsi));
	DEBUG("mcc[0] = 0x%x, mcc[1] = 0x%x; mnc[0]=0x%x,mnc[1]=0x%x\n", imsi.Mcc[0],imsi.Mcc[1],imsi.Mnc[0],imsi.Mnc[1]);


	//while(USART_SUCESS != GSM_QueryCreg(&creg));
	//DEBUG("n = %d; status=%d lac[0] = 0x%x, lac[1] = 0x%x ci[0] = 0x%x ci[1] = 0x%x\n", creg.n, creg.Stat, creg.Lac[0], creg.Lac[1], creg.Ci[0], creg.Ci[1]);
    //GSM_creg();
	//GSM_cgatt();
    //GSM_cstt();
    //GSM_ciicr();
    //GSM_cifsr();
    GSM_ceng();
}

void GSM_tcpip_test(void)
{

}

