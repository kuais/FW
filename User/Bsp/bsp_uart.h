#ifndef __BSP_USART__
#define __BSP_USART__

#include "KuFrame/kubuffer.h"
#include "RTE_Device.h"
#include "platform.h"

#define DMA_RX_BUF_SIZE 1 // 83

/* 串口设备结构体 */
typedef struct
{
    USART_TypeDef *uart;        /* STM32内部串口设备指针 */
    KuBuffer *bufTx;            /* 发送缓冲区 */
    KuBuffer *bufRx;            /* 接收缓冲区 */
    DMA_Channel_TypeDef *dmaRx; /* DMA接收通道 */
    DMA_Channel_TypeDef *dmaTx; /* DMA发送通道 */
    uint8_t *bufDma;            /* DMA接收临时缓存 */
    uint8_t isReady;
} UART_T;

extern void uart_InitUart(USART_TypeDef *uart);
extern void uart_Config(void);
extern void uart_Send(UART_T *pUart, uint8_t *pBuf, uint16_t pNum);
extern void uart_Handle(void);
extern void uart_IRQ_Rx(UART_T *pUart);
extern void uart_IRQ_Tx(UART_T *pUart);
extern int uart_IRQ_RxEnd(UART_T *pUart);
extern int uart_IRQ_TxEnd(UART_T *pUart);
extern void dma_RxBegin(UART_T *pUart);
extern void dma_RxEnd(UART_T *pUart);
extern void dma_TxBegin(UART_T *pUart);
extern void dma_TxEnd(UART_T *pUart);

#if RTE_USART1
#define UART1_BAUD 115200
#define UART1_TX_BUF_SIZE 200
#define UART1_RX_BUF_SIZE 200
#define UART1_DMA 0
#if UART1_DMA
#define UART1_DMA_ChRx DMA1_Channel5
#define UART1_DMA_ChTx DMA1_Channel4
#endif

extern UART_T g_tUart1;
extern void uart1_Config(void);
extern void uart1_Send(uint8_t *buf, uint16_t len);
#endif

#if RTE_USART2
#define UART2_BAUD 115200
#define UART2_TX_BUF_SIZE 200
#define UART2_RX_BUF_SIZE 200
#define UART2_DMA 1
#if UART2_DMA
#define UART2_DMA_ChRx DMA1_Channel6
#define UART2_DMA_ChTx DMA1_Channel7
#endif
extern UART_T g_tUart2;
extern void uart2_Config(void);
extern void uart2_Send(uint8_t *buf, uint16_t len);
#endif

#if RTE_USART3
#define UART3_BAUD 115200
#define UART3_TX_BUF_SIZE 10
#define UART3_RX_BUF_SIZE 85
#define UART3_DMA 1
#if UART3_DMA
#define UART3_DMA_ChRx DMA1_Channel3
#define UART3_DMA_ChTx DMA1_Channel2
#endif
extern UART_T g_tUart3;
extern void uart3_Config(void);
extern void uart3_Send(uint8_t *buf, uint16_t len);
#endif

#if RTE_UART4
#define UART4_BAUD 115200
#define UART4_TX_BUF_SIZE 10
#define UART4_RX_BUF_SIZE 85
#define UART4_DMA 1
#if UART4_DMA
#define UART4_DMA_ChRx DMA2_Channel3
#define UART4_DMA_ChTx DMA2_Channel5
#endif
extern UART_T g_tUart4;
extern void uart4_Config(void);
extern void uart4_Send(uint8_t *buf, uint16_t len);
#endif

#if RTE_UART5
#define UART5_BAUD 115200
#define UART5_TX_BUF_SIZE 60
#define UART5_RX_BUF_SIZE 64
extern UART_T g_tUart5;
extern void uart5_Config(void);
extern void uart5_Send(uint8_t *buf, uint16_t len);
#endif

#endif
