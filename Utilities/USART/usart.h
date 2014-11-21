#ifndef USART_H
#define USART_H

#include "stm32f10x.h"

//#define USART_GSM_BUFSIZE    1024
//#define USART_GPS_BUFSIZE    1024

#define     USART_SUCESS              0   // �ɹ�
#define     USART_DATAERROR           1   // ���ݴ���
#define     USART_FAIL                1   //ʧ��
#define     USART_NEEDWAIT            2   // ��Ҫ�ȴ�
#define     USART_ENPROCESS           3   // ���Դ���


typedef struct
{
    unsigned char   *RStartpt;      // ���ջ�����ʼ��ַ
    uint32_t        uiRMaxLen;      // ���ջ�����󳤶�
    unsigned char   *RBuffWpt;      // ���ջ���ָ��
    unsigned char   *RBuffRpt;      // ���մ���ָ��
    uint32_t        ucRLen;         // �������ݳ���
    uint32_t        ucRTime;        // ���ճ�ʱʱ��
    uint32_t        ucRTick;
}USART_ST;

void usart_irq_my(uint32_t chan, uint8_t data);
void usart_init(uint32_t chan);
uint16_t USART_Send(USART_TypeDef* USARTx, uint8_t *Data, uint16_t nBytes);
void usart_sendbuffer(uint32_t chan, char *byData, uint32_t *pReqLen);
unsigned char usart_readbuffer(uint32_t chan, char *byData, uint32_t *pReqLen);
void usart_irq(uint32_t chan, USART_ST *usart_st);
void usart_timeout(uint32_t chan, USART_ST *pusart);

void USART1_Config(void);

#endif /* USART_H */
