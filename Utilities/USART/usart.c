#include "usart.h"
#include "stm32gps_config.h"
#include "stm32f10x_it_api.h"
#include "string.h"
#include "stdio.h"

USART_ST st_Serial[2];
unsigned char USART1_RBuffer[USART_GPS_BUFSIZE] = { 0 };
unsigned char USART2_RBuffer[USART_GSM_BUFSIZE] = { 0 };

static uint32_t usart_chan2st(uint32_t chan)
{
    uint32_t idx = 0xFFFFFFFF;

    if (chan == 0)
    {
        idx = 0;
    }
    else if (chan == 1)
    {
        idx = 1;
    }

    return idx;
}

static USART_TypeDef *usart_hw_base_get_from_chan(uint32_t chan)
{
    USART_TypeDef *usart_hw_base;

    if (chan == 0)
    {
        usart_hw_base = USART1;
    }
    else if (chan == 1)
    {
        usart_hw_base = USART2;
    }
    else
    {
        usart_hw_base = 0;
    }

    return usart_hw_base;
}

void usart_init(uint32_t chan)
{
    uint32_t idx = usart_chan2st(chan);

    if (idx == 0)
    {
        st_Serial[idx].RStartpt = (unsigned char *) USART1_RBuffer;   // ���н��ջ�����ʼ��ַ
        st_Serial[idx].uiRMaxLen = USART_GPS_BUFSIZE;                 // ���н��ջ��峤��
        st_Serial[idx].ucRTick = (sysTickPerSec() * GPS_USART_TIMEOUT_MS) / 1000 + 1;
    }
    else if (idx == 1)
    {
        st_Serial[idx].RStartpt = (unsigned char *) USART2_RBuffer;   // ���н��ջ�����ʼ��ַ
        st_Serial[idx].uiRMaxLen = USART_GSM_BUFSIZE;                 // ���н��ջ��峤��
        st_Serial[idx].ucRTick = (sysTickPerSec() * GSM_USART_TIMEOUT_MS) / 1000 + 1;
    }
	else
	{
		;
	}
	
    st_Serial[idx].RBuffWpt = st_Serial[idx].RStartpt;    // ���н��ջ���ָ�룬+2��2�ֽڱ�����̶�
    st_Serial[idx].RBuffRpt = st_Serial[idx].RStartpt;    // ���н��մ���ָ��
    st_Serial[idx].ucRTime = 0;
	
    if (st_Serial[idx].ucRTick <= 1)
    {
        st_Serial[idx].ucRTick = 2;
    }

    //printf("idx %d ucRTick %d", idx, st_Serial[idx].ucRTick);
}

uint16_t USART_Send(USART_TypeDef* USARTx, uint8_t *Data, uint16_t nBytes)
{
	uint16_t i;
	for (i = 0; i < nBytes; i++) 
	{
		USART_SendData(USARTx, *(Data + i));
		while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET )
		{
			;
		}
	}
	return i;
}

void usart_sendbuffer(uint32_t chan, char *byData, uint32_t *pReqLen)
{
    USART_TypeDef *usart_hw_base = usart_hw_base_get_from_chan(chan);
    uint32_t reqLen = *pReqLen;

    while (reqLen > 0)
    {
        //printf("%d-%c\n", *pReqLen - reqLen, byData[*pReqLen - reqLen]);
        USART_SendData(usart_hw_base, byData[*pReqLen - reqLen]);

        while (USART_GetFlagStatus(usart_hw_base, USART_FLAG_TXE) == RESET)
        {
			;
        }
		
        USART_ClearFlag(usart_hw_base, USART_FLAG_TXE);

        reqLen--;
    }
}

#undef DBG_usart_readbuffer
unsigned char usart_readbuffer(uint32_t chan, char *byData, unsigned int *pReqLen)
{
    unsigned char dFlag = USART_DATAERROR;
    uint32_t curLen = 0;
    uint32_t ReqLen = *pReqLen;
    uint32_t idx = usart_chan2st(chan);

    curLen = st_Serial[idx].ucRLen;
	
    if (curLen != 0)                   // ������������
    {
        if (st_Serial[idx].ucRTime != 0)     // ���ڽ�������
        {
            dFlag = USART_NEEDWAIT;
            *pReqLen = 0;
#ifdef DBG_usart_readbuffer
            printf("usart_readbuffer[%d] busy\n", chan);
#endif
        }
        else
        {
            dFlag = USART_ENPROCESS;
            curLen = st_Serial[idx].ucRLen;
            if (curLen > ReqLen)
            {
                // ��������������������������������������������
                curLen = ReqLen;
            }
            else
            {
				;
            }

#ifdef DBG_usart_readbuffer
            printf("usart_readbuffer[%d] ", chan);
            printf("curLen %d ", curLen);
            printf("reqLen %d\n", ReqLen);
#endif

            if (st_Serial[idx].RBuffWpt > st_Serial[idx].RBuffRpt)
            {
                // ��ָ����дָ���ǰ��
                //printf("rd before wr\n");

                memcpy(byData, st_Serial[idx].RBuffRpt, curLen);
                st_Serial[idx].RBuffRpt += curLen;
                st_Serial[idx].ucRLen -= curLen;
                *pReqLen = curLen;
            }
            else
            {
                // ��ָ����дָ��ĺ���
                unsigned int backLen = (st_Serial[idx].RStartpt
                        			    + st_Serial[idx].uiRMaxLen
                        			    - st_Serial[idx].RBuffRpt); // ��������벿���ݳ���
                unsigned int saveLen = curLen;

                if (saveLen <= backLen) // ֻ�����벿�Ϳ������
                {
                    //printf("only end half\n");
                    memcpy(byData, st_Serial[idx].RBuffRpt, saveLen);
                    st_Serial[idx].RBuffRpt += saveLen;

					// ���λ�������β������
                    if (st_Serial[idx].RBuffRpt >= st_Serial[idx].RStartpt + st_Serial[idx].uiRMaxLen)
                    {
                        st_Serial[idx].RBuffRpt = st_Serial[idx].RStartpt;
                    }
					
                    st_Serial[idx].ucRLen -= saveLen;
                    *pReqLen = curLen;
                }
                else // Ҫȡ����������
                {

                    //printf("both front and end half\n");
                    // ��ȡ��벿������
                    memcpy(byData, st_Serial[idx].RBuffRpt, backLen);
                    st_Serial[idx].RBuffRpt = st_Serial[idx].RStartpt;  // ָ��ǰ��
                    st_Serial[idx].ucRLen -= backLen;

                    // ȡǰ�벿������
                    saveLen -= backLen;                     // ǰ��Ҫȡ�����ݳ���
                    memcpy(byData + backLen, st_Serial[idx].RBuffRpt, saveLen);
                    st_Serial[idx].RBuffRpt += saveLen;
                    st_Serial[idx].ucRLen -= saveLen;
                    *pReqLen = saveLen + backLen;
                }
            }
        }
#ifdef DBG_usart_readbuffer
        printf("usart_readbuffer[%d],st_reqLen %d, reqLen %d\r\n",
                chan, st_Serial[idx].ucRLen, *pReqLen);
#endif
    }
    else
    {
        //û�����ݽ���
        ReqLen = 0;
        *pReqLen = 0;
        dFlag = USART_FAIL;
    }

    return dFlag;
}

void usart_irq(uint32_t chan, USART_ST *pusart)
{
    unsigned char RxData;
    uint32_t idx = usart_chan2st(chan);
    USART_TypeDef * usart_hw_base = usart_hw_base_get_from_chan(chan);

    /* ������յ������� */
    while (USART_GetFlagStatus(usart_hw_base, USART_IT_RXNE) == SET)
    {
        //STM_EVAL_LEDToggle(LED4);

        RxData = USART_ReceiveData(usart_hw_base);
		
        if ((st_Serial[idx].RBuffRpt == (st_Serial[idx].RBuffWpt + 1)) ||  // дָ�뼴�����Ƕ�ָ��
             ((st_Serial[idx].RBuffRpt == st_Serial[idx].RStartpt) && // ��ָ��ָ�򻺴��һ��λ��,дָ��ָ�򻺴����һ��λ��
               (st_Serial[idx].RBuffWpt == (st_Serial[idx].RStartpt + st_Serial[idx].uiRMaxLen - 1)))) // ���ջ�����
        {
            return;
        }

        *(st_Serial[idx].RBuffWpt++) = RxData;
        st_Serial[idx].ucRLen++;

		// ����ָ�뵽���ײ�
        if (st_Serial[idx].RBuffWpt >= (st_Serial[idx].RStartpt + st_Serial[idx].uiRMaxLen)) 
        {
            st_Serial[idx].RBuffWpt = st_Serial[idx].RStartpt;
        }
		
        st_Serial[idx].ucRTime = st_Serial[idx].ucRTick; // 20ms���ճ�ʱ
    }
}

void usart_irq_my(uint32_t chan, uint8_t data)
{
//    unsigned char RxData;
    uint32_t idx = usart_chan2st(chan);
    USART_TypeDef * usart_hw_base = usart_hw_base_get_from_chan(chan);


        if ((st_Serial[idx].RBuffRpt == (st_Serial[idx].RBuffWpt + 1)) ||  // дָ�뼴�����Ƕ�ָ��
             ((st_Serial[idx].RBuffRpt == st_Serial[idx].RStartpt) && // ��ָ��ָ�򻺴��һ��λ��,дָ��ָ�򻺴����һ��λ��
               (st_Serial[idx].RBuffWpt == (st_Serial[idx].RStartpt + st_Serial[idx].uiRMaxLen - 1)))) // ���ջ�����
        {
            return;
        }

        *(st_Serial[idx].RBuffWpt++) = data;
        st_Serial[idx].ucRLen++;

		// ����ָ�뵽���ײ�
        if (st_Serial[idx].RBuffWpt >= (st_Serial[idx].RStartpt + st_Serial[idx].uiRMaxLen)) 
        {
            st_Serial[idx].RBuffWpt = st_Serial[idx].RStartpt;
        }
		
        st_Serial[idx].ucRTime = st_Serial[idx].ucRTick; // 20ms���ճ�ʱ
    
}


void usart_timeout(uint32_t chan, USART_ST *pusart)
{
    uint32_t idx = usart_chan2st(chan);

    if (st_Serial[idx].ucRTime != 0)
    {
        st_Serial[idx].ucRTime--;

#if 0
        if (st_Serial[idx].ucRTime == 0)
        {
            STM_EVAL_LEDToggle(LED3);
        }
#endif
    }
}

/*
 * ��������USART1_Config
 * ����  ��USART1 GPIO ����,����ģʽ����
 * ����  ����
 * ���  : ��
 * ����  ���ⲿ����
 */
void USART1_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	/* USART1 GPIO config */
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(USART1, ENABLE);
}



