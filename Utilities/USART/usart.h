#ifndef USART_H
#define USART_H

#include "stm32f10x.h"

//#define USART_GSM_BUFSIZE    1024
//#define USART_GPS_BUFSIZE    1024

#define     USART_SUCESS              0   // 成功
#define     USART_DATAERROR           1   // 数据错误
#define     USART_FAIL                1   //失败
#define     USART_NEEDWAIT            2   // 需要等待
#define     USART_ENPROCESS           3   // 可以处理


typedef struct
{
    unsigned char   *RStartpt;      // 接收缓冲起始地址
    uint32_t        uiRMaxLen;      // 接收缓冲最大长度
    unsigned char   *RBuffWpt;      // 接收缓冲指针
    unsigned char   *RBuffRpt;      // 接收处理指针
    uint32_t        ucRLen;         // 接收数据长度
    uint32_t        ucRTime;        // 接收超时时间
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
