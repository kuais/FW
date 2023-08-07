#include "bsp_uart.h"
#include "main.h"

void uart_InitUart(USART_TypeDef *uart)
{
#if RTE_USART1
    if ((uart == USART1) && (g_tUart1.uart != USART1))
    {
        g_tUart1.uart = uart;
        g_tUart1.bufRx = buffer_new(UART1_RX_BUF_SIZE);
        g_tUart1.bufTx = buffer_new(UART1_TX_BUF_SIZE);
#if UART1_DMA
        g_tUart1.dmaRx = UART1_DMA_ChRx;
        g_tUart1.dmaTx = UART1_DMA_ChTx;
        g_tUart1.bufDma = (uint8_t *)mymalloc(DMA_RX_BUF_SIZE * sizeof(uint8_t));
#endif
    }
#endif
#if RTE_USART2
    if ((uart == USART2) && (g_tUart2.uart != USART2))
    {
        g_tUart2.uart = uart;
        g_tUart2.bufRx = buffer_new(UART2_RX_BUF_SIZE);
        g_tUart2.bufTx = buffer_new(UART2_TX_BUF_SIZE);
#if UART2_DMA
        g_tUart2.dmaRx = UART2_DMA_ChRx;
        g_tUart2.dmaTx = UART2_DMA_ChTx;
        g_tUart2.bufDma = (uint8_t *)mymalloc(DMA_RX_BUF_SIZE * sizeof(uint8_t));
#endif
    }
#endif
#if RTE_USART3
    if ((uart == USART3) && (g_tUart3.uart != USART3))
    {
        g_tUart3.uart = uart;
        g_tUart3.bufRx = buffer_new(UART3_RX_BUF_SIZE);
        g_tUart3.bufTx = buffer_new(UART3_TX_BUF_SIZE);
#if UART3_DMA
        g_tUart3.dmaRx = UART3_DMA_ChRx;
        g_tUart3.dmaTx = UART3_DMA_ChTx;
        g_tUart3.bufDma = (uint8_t *)mymalloc(DMA_RX_BUF_SIZE * sizeof(uint8_t));
#endif
    }
#endif
#if RTE_UART4
    if ((uart == UART4) && (g_tUart4.uart != UART4))
    {
        g_tUart4.uart = uart;
        g_tUart4.bufRx = buffer_new(UART4_RX_BUF_SIZE);
        g_tUart4.bufTx = buffer_new(UART4_TX_BUF_SIZE);
#if UART4_DMA
        g_tUart4.dmaRx = UART4_DMA_ChRx;
        g_tUart4.dmaTx = UART4_DMA_ChTx;
        g_tUart4.bufDma = (uint8_t *)mymalloc(DMA_RX_BUF_SIZE * sizeof(uint8_t));
#endif
    }
#endif
#if RTE_UART5
    if ((uart == UART5) && (g_tUart5.uart != UART5))
    {
        g_tUart5.uart = uart;
        g_tUart5.bufTx = buffer_new(UART5_TX_BUF_SIZE);
        g_tUart5.bufRx = buffer_new(UART5_RX_BUF_SIZE);
    }
#endif
    USART_ClearFlag(uart, USART_FLAG_TC);
    USART_Cmd(uart, ENABLE);
}

void uart_Config(void)
{
#if RTE_USART1
    uart1_Config();
#endif
#if RTE_USART2
    uart2_Config();
#endif
#if RTE_USART3
    uart3_Config();
#endif
#if RTE_UART4
    uart4_Config();
#endif
#if RTE_UART5
    uart5_Config();
#endif
}

void uart_IRQ_Rx(UART_T *pUart)
{
    if (USART_GetITStatus(pUart->uart, USART_IT_RXNE) != RESET)
    {
        /* 从串口接收数据寄存器读取数据存放到接收FIFO,这步同时清除了接收中断 */
        uint8_t c = USART_ReceiveData(pUart->uart);
        buffer_put(pUart->bufRx, &c, 1);
        // /*  清除接收中断 */
        // USART_ClearITPendingBit(pUart->uart, USART_IT_RXNE);
    }
}
void uart_IRQ_Tx(UART_T *pUart)
{
    if (USART_GetITStatus(pUart->uart, USART_IT_TXE) != RESET)
    {
        if (buffer_dataCount(pUart->bufTx) > 0)
        {
            uint8_t *c = buffer_get(pUart->bufTx, 0);
            USART_SendData(pUart->uart, *c);
            buffer_remove(pUart->bufTx, 1);
        }
        else
        { /* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断
             （注意：此时最后1个数据还未真正发送完毕）*/
            USART_ITConfig(pUart->uart, USART_IT_TXE, DISABLE);
        }
    }
}
int uart_IRQ_RxEnd(UART_T *pUart)
{
    if (USART_GetITStatus(pUart->uart, USART_IT_IDLE) != RESET)
    {                    /* 清除空闲中断 */
        pUart->uart->SR; //先读SR
        pUart->uart->DR; //再读DR
        return 1;
    }
    return 0;
}
int uart_IRQ_TxEnd(UART_T *pUart)
{
    if (USART_GetITStatus(pUart->uart, USART_IT_TC) != RESET)
    { /* 数据bit位全部发送完毕的中断 */
        USART_ITConfig(pUart->uart, USART_IT_TC, DISABLE);
        USART_ClearFlag(pUart->uart, USART_FLAG_TC); // 清除标志位
        return 1;
    }
    return 0;
}
// uint16_t uart_Recv(UART_T *pUart, KuBuffer **pBuf)
//{ /* 处理接收到的数据 */
//    int len = buffer_dataCount(pUart->bufRx);
//    if (len > 0)
//        *pBuf = &pUart->bufRx;
//    return len;
//}
void uart_Send(UART_T *pUart, uint8_t *pBuf, uint16_t pNum)
{
    /* 开启发送 */
    if (pUart->uart == USART1)
    {
#if RTE_USART1
        uart1_Send(pBuf, pNum);
#endif
    }
    else if (pUart->uart == USART2)
    {
#if RTE_USART2
        uart2_Send(pBuf, pNum);
#endif
    }
    else if (pUart->uart == USART3)
    {
#if RTE_USART3
        uart3_Send(pBuf, pNum);
#endif
    }
    else if (pUart->uart == UART4)
    {
#if RTE_UART4
        uart4_Send(pBuf, pNum);
#endif
    }
    else if (pUart->uart == UART5)
    {
#if RTE_UART5
        uart5_Send(pBuf, pNum);
#endif
    }
    // os_dly_wait(1);
}

void dma_RxBegin(UART_T *pUart)
{
    pUart->dmaRx->CMAR = (u32)pUart->bufDma;
    pUart->dmaRx->CNDTR = DMA_RX_BUF_SIZE; // DMA通道的DMA缓存的大小
    DMA_Cmd(pUart->dmaRx, ENABLE);         // 使能DMA RX通道
}
void dma_RxEnd(UART_T *pUart)
{
    DMA_Cmd(pUart->dmaRx, DISABLE); //关闭USART2 RX通道
    u8 len = DMA_RX_BUF_SIZE - pUart->dmaRx->CNDTR;
    buffer_put(pUart->bufRx, pUart->bufDma, len);
}
void dma_TxBegin(UART_T *pUart)
{
    pUart->dmaTx->CMAR = (u32)buffer_get(pUart->bufTx, 0); //设置要发送的数据地址
    pUart->dmaTx->CNDTR = buffer_dataCount(pUart->bufTx);  //设置要发送的字节数目
    DMA_Cmd(pUart->dmaTx, ENABLE);                         //开始DMA发送
}
void dma_TxEnd(UART_T *pUart)
{
    DMA_Cmd(pUart->dmaTx, DISABLE); // 关闭TX通道
    buffer_clear(pUart->bufTx);
}
