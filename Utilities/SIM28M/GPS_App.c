/****************************************Copyright (c)****************************************************
 **                                  		 广州星海科技
 **                                  http://simhi.taobao.com
 **																		 TEL:15018750559
 **																			 QQ:86817447
 **------------------------File Info----------------------------------------------------------------------
 ** File Name:             GPS_App.c
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
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"

#include "stm32gps_config.h"
#include "stm32gps_board.h"
#include "usart.h"
#include "GSM_App.h"
#include "GPS_App.h"


#define INT_MAX ((int)0x7FFFFFFF)
#define INT_MIN ((int)0x80000000)

static int my_atoi_len(const char * str, const int len);

char GPSBuffer[USART_GPS_BUFSIZE];

/*
 $GPGGA,054753.000,2308.161553,N,11322.462574,E,1,9,0.88,45.402,M,-6.310,M,,*72
 $GPGLL,2308.161553,N,11322.462574,E,054753.000,A,A*50
 $GPGSA,A,3,14,22,31,25,30,18,32,29,16,,,,1.66,0.88,1.41*00
 $GPGSV,3,1,12,14,56,032,35,22,56,183,50,31,52,312,39,25,44,046,21*7C
 $GPGSV,3,2,12,30,37,210,49,18,17,156,47,32,17,319,29,29,16,107,51*75
 $GPGSV,3,3,12,16,10,207,45,12,03,036,26,01,00,295,,40,,,45*43
 $GPRMC,054753.000,A,2308.161553,N,11322.462574,E,0.000,0,010512,,,A*7E
 $GPVTG,0,T,,M,0.000,N,0.000,K,A*13
 $GPZDA,054753.000,01,05,2012,,*53
 */

/*********************************************************************************************************
 ** Function name:       my_atoi_len
 ** Descriptions:        依据len将字符串转换为数字
 ** input parameters:    str 要转换的字符串
 **						len 要转换的长度
 ** output parameters:   NONE
 ** Returned value:      转换得到的数字值
 ** Example: my_atoi_len("12345", 2) 将得到数字 12
 *********************************************************************************************************/
static int my_atoi_len(const char * str, const int len)
{
    int minus=0;
    long long result=0;
	int rst = 0;
	int valid;
    
    if(str==NULL)
        return 0;
    while(*str==' ')
        str++;
    if(*str=='-')
    {
        minus=1;
        str++;
    }
    else if(*str=='+')
        str++;
    if(*str<'0'||*str>'9')
        return 0;

    valid=1;
    while((*str>='0' && *str<='9') && (rst < len))
    {
        result=result*10+*str-'0';
        if((minus && result>INT_MAX + 1LL) || (!minus && result>INT_MAX))
        {
            valid=0;
            return 0;
        }

        str++;
		rst++;
    }

    if(minus)
        result*=-1;
    return (int)result;
}

/**
  * @brief  Turn on GPS's VCC power.
  * Output LOW to Turn On VCC Power; HIGH to Turn Off VCC Power.
  * @param  None
  * @retval None
  */
void GPSPowerOn(void)
{
	GPIO_ResetBits(GPS_PWR_CTRL_PORT, GPS_PWR_CTRL_PIN);
	
}

/**
  * @brief  Turn off GPS's VCC power.
  * Output LOW to Turn On VCC Power; HIGH to Turn Off VCC Power.
  * @param  None
  * @retval None
  */
void GPSPowerOff(void)
{
	GPIO_SetBits(GPS_PWR_CTRL_PORT, GPS_PWR_CTRL_PIN);
}

void GPSClearBuffer(void)
{
    memset(GPSBuffer, 0, USART_GPS_BUFSIZE);
}

/*********************************************************************************************************
 ** Function name:       GPSInfoAnalyze
 ** Descriptions:        解析GPS数据
 ** input parameters:    NONE
 ** output parameters:   NONE
 ** Returned value:      NONE
 *********************************************************************************************************/
unsigned char GPSInfoAnalyze(pST_GPSRMCINFO pRmcInfo)
{
    unsigned int uLen;
    unsigned char i;
    char *phead, *psearch;

    uLen = 1024;


    if (USART_ENPROCESS
        != usart_readbuffer(STM32_SIM908_GPS_COM, GPSBuffer, &uLen))
    {
        return GPS_FAIL;
    }

    if (uLen > 0)
    {
#ifdef DBG_ENABLE_MACRO
        {
            unsigned int i;

            DEBUG("GPSInfoAnalyze recv %d bytes\r\n", uLen);
            for (i = 0; i < uLen; i++)
            {
                DEBUG("%c", GPSBuffer[i]);
            }

            DEBUG("\r\n");
            DEBUG("GPSInfoAnalyze done\r\n");
        }
#endif

        phead = strstr((const char *) GPSBuffer, (const char *) "$GPRMC");
        if (NULL != phead)
        {
            /* 解析GPS有效性 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 2);
            if (NULL != psearch)
            {
				psearch++;
				if((*psearch == 'A') || (*psearch == 'a'))
				{
					pRmcInfo->status = 1;
				}
				else
				{
					pRmcInfo->status = 0;
					return GPS_INVALID;
				}
                
            }
            /* 解析时间 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 1);
            if (NULL != psearch)
            {
                psearch++;
                for (i = 0; *psearch != ','; i++)
                {
                    pRmcInfo->time[i] = *(psearch++);
                }
            }

			/* 解析纬度 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 3);
            if (NULL != psearch)
            {
                psearch++;
                for (i = 0; *psearch != ','; i++)
                {
                    pRmcInfo->latitude[i] = *(psearch++);
                }
            }

			/* 解析纬度指示 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 4);
            if (NULL != psearch)
            {
                psearch++;
				if(*psearch != ',')
				{
					pRmcInfo->ns_indic = *psearch;
				}
            }

			/* 解析经度 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 5);
            if (NULL != psearch)
            {
                psearch++;
                for (i = 0; *psearch != ','; i++)
                {
                    pRmcInfo->longitude[i] = *(psearch++);
                }
            }

			/* 解析经度指示 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 6);
            if (NULL != psearch)
            {
                psearch++;
				if(*psearch != ',')
				{
					pRmcInfo->ew_indic = *psearch;
				}
            }

			/* 解析速度 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 7);
            if (NULL != psearch)
            {
                psearch++;
                for (i = 0; *psearch != ','; i++)
                {
                    pRmcInfo->speed[i] = *(psearch++);
                }
				pRmcInfo->speed[i] = '\0';
            }

			/* 解析航向 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 8);
            if (NULL != psearch)
            {
                psearch++;
                for (i = 0; *psearch != ','; i++)
                {
                    pRmcInfo->course[i] = *(psearch++);
                }
				pRmcInfo->course[i] = '\0';
            }

			/* 解析日期 */
            psearch = NULL;
            psearch = strnchr(phead, ',', 9);
            if (NULL != psearch)
            {
                psearch++;
                for (i = 0; *psearch != ','; i++)
                {
                    pRmcInfo->date[i] = *(psearch++);
                }
            }
        }
		else
		{
			return GPS_ERROR;
		}

		// clear GPSBuffer
        memset(GPSBuffer, 0, uLen);
    }

	return GPS_SUCCESS;
}


void ParseGPSInfo(ST_GPSRMCINFO rmcInfo, pST_GPSDATA pGpsData)
{
	uint32_t i;
	struct tm t;
	int valid;

	t.tm_sec = my_atoi_len(rmcInfo.time + 4, 2);
	t.tm_min = my_atoi_len(rmcInfo.time + 2, 2);
	t.tm_hour = my_atoi_len(rmcInfo.time + 0, 2);
	t.tm_mday = my_atoi_len(rmcInfo.date + 0, 2);
	t.tm_mon = my_atoi_len(rmcInfo.date + 2, 2);
	t.tm_year = my_atoi_len(rmcInfo.date + 4, 2) + 100;
	t.tm_isdst = 0;

	pGpsData->utc.i = mktime(&t);
#ifdef DBG_ENABLE_MACRO
	DEBUG("UTC:");
	for(i = 0; i < 4; i++)
	{
		DEBUG("0x%x-", pGpsData->utc.s[i]);
	}
	DEBUG("\n");
#endif

	// latitude
	if(rmcInfo.ns_indic == 'N')
	{
		pGpsData->latitude.i = (((my_atoi_len(rmcInfo.latitude, 2))*60 
						+ my_atoi_len(rmcInfo.latitude + 2, 2))*30000 
						+ (my_atoi_len(rmcInfo.latitude + 5, 1))*3000 + (my_atoi_len(rmcInfo.latitude + 6, 1))*300 
						+ (my_atoi_len(rmcInfo.latitude + 7, 1))*30 + (my_atoi_len(rmcInfo.latitude + 8, 1))*3);
	}
	else if(rmcInfo.ns_indic == 'S')
	{
		pGpsData->latitude.i = -(((my_atoi_len(rmcInfo.latitude, 2))*60 
						+ my_atoi_len(rmcInfo.latitude + 2, 2))*30000 
						+ (my_atoi_len(rmcInfo.latitude + 5, 1))*3000 + (my_atoi_len(rmcInfo.latitude + 6, 1))*300 
						+ (my_atoi_len(rmcInfo.latitude + 7, 1))*30 + (my_atoi_len(rmcInfo.latitude + 8, 1))*3);
	}

#ifdef DBG_ENABLE_MACRO
	DEBUG("LATI:");
	for(i = 0; i < 4; i++)
	{
		DEBUG("0x%x-", pGpsData->latitude.s[i]);
	}
	DEBUG("\n");
#endif

	// longitude
	if(rmcInfo.ew_indic == 'E')
	{
		pGpsData->longitude.i = ((my_atoi_len(rmcInfo.longitude, 3)*60 
						+ my_atoi_len(rmcInfo.longitude + 3, 2))*30000 
						+ my_atoi_len(rmcInfo.longitude + 6, 1)*3000 + my_atoi_len(rmcInfo.longitude + 7, 1)*300 
						+ my_atoi_len(rmcInfo.longitude + 8, 1)*30 + my_atoi_len(rmcInfo.longitude + 9, 1)*3);
	}
	else if(rmcInfo.ew_indic == 'W')
	{
		pGpsData->longitude.i = -((my_atoi_len(rmcInfo.longitude, 3)*60 
						+ my_atoi_len(rmcInfo.longitude + 3, 2))*30000 
						+ my_atoi_len(rmcInfo.longitude + 6, 1)*3000 + my_atoi_len(rmcInfo.longitude + 7, 1)*300 
						+ my_atoi_len(rmcInfo.longitude + 8, 1)*30 + my_atoi_len(rmcInfo.longitude + 9, 1)*3);
	}

#ifdef DBG_ENABLE_MACRO
	DEBUG("LONGI:");
	for(i = 0; i < 4; i++)
	{
		DEBUG("0x%x-", pGpsData->longitude.s[i]);
	}
	DEBUG("\n");
#endif

	// 1 knot = 1.85km/h
	pGpsData->speed = (unsigned char)((atof(rmcInfo.speed))*(1.85));

	pGpsData->course.i = (unsigned short)(atof(rmcInfo.course));

	pGpsData->status = rmcInfo.status;

#ifdef DBG_ENABLE_MACRO
	DEBUG("SPEED: 0x%x\n", pGpsData->speed);
	DEBUG("COURSE:");
	for(i = 0; i < 2; i++)
	{
		DEBUG("0x%x-", pGpsData->course.s[i]);
	}
	DEBUG("\n");
	DEBUG("STATUS: 0x%x\n", pGpsData->status);
#endif
	
}



