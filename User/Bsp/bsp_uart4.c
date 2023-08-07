#include "bsp_uart.h"

UART_T g_tUart4;

#if UART4_DMA
static void uart4_ConfigDMA(void)
{
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    /* fill init structure */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设递增模式-1个外设
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存递增模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 以字节为传输单位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR;
    /* DMA TX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; // 外设为数据传输的目的地
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;     // 先随意设，在发送时设实际值
    DMA_InitStructure.DMA_BufferSize = 1;              // 先随意设，在发送时设实际值
    DMA_DeInit(UART4_DMA_ChTx);
    DMA_Init(UART4_DMA_ChTx, &DMA_InitStructure);
    DMA_Cmd(UART4_DMA_ChTx, DISABLE); // 使DMA TX通道停止工作
    /* DMA RX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // 外设为数据传输的目的地
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_tUart4.bufDma;
    DMA_InitStructure.DMA_BufferSize = DMA_RX_BUF_SIZE;
    DMA_DeInit(UART4_DMA_ChRx);
    DMA_Init(UART4_DMA_ChRx, &DMA_InitStructure);
    DMA_Cmd(UART4_DMA_ChRx, ENABLE); // 使DMA RX 通道开始工作

    /* DMA 中断 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel3_IRQn;
    NVIC_Init(&NVIC_InitStructure);                  // 配置接收中断
    DMA_ITConfig(UART4_DMA_ChRx, DMA_IT_TC, ENABLE); // 配置DMA接收满后产生中断

    // NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQn;
    // NVIC_Init(&NVIC_InitStructure);                  // 配置发送中断
    // DMA_ITConfig(UART4_DMA_ChTx, DMA_IT_TC, ENABLE); // 配置DMA发送完成后产生中断

    /* Enable DMA request */
    USART_DMACmd(UART4, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
}
void DMA2_Channel3_IRQHandler(void)
{ // DMA接收中断
    if (DMA_GetFlagStatus(DMA2_FLAG_TC3))
    {
        DMA_ClearITPendingBit(DMA2_IT_TC3);
        dma_RxEnd(&g_tUart4);
        dma_RxBegin(&g_tUart4);
    }
}
// void DMA2_Channel4_5_IRQHandler(void)
// { // DMA发送中断
//     if (DMA_GetFlagStatus(DMA2_FLAG_TC5))
//     {
//         DMA_ClearITPendingBit(DMA2_IT_TC5);
//         dma_TxEnd(&g_tUart4);
//     }
// }
#endif

void UART4_IRQHandler(void)
{
    /* 接收中断  */
    uart_IRQ_Rx(&g_tUart4);
    if (uart_IRQ_RxEnd(&g_tUart4))
    {
#if UART4_DMA
        // dma_RxEnd(&g_tUart4);
        // dma_RxBegin(&g_tUart4);
#endif
        isr_sem_send(&semaphore_uart4);
    }
    /* 发送中断 */
    uart_IRQ_Tx(&g_tUart4);
    if (uart_IRQ_TxEnd(&g_tUart4))
    {
#if UART4_DMA
        dma_TxEnd(&g_tUart4);
#endif
    }
}
void uart4_Send(uint8_t *buf, uint16_t len)
{
    if (!g_tUart4.isReady)
        return;
    /* 等待上一次发送完成 */
    while (1)
    {
        if (buffer_dataCount(g_tUart4.bufTx) == 0)
            break;
    }
    /* 将新数据填入发送缓冲区 */
    buffer_put(g_tUart4.bufTx, buf, len);
#if UART4_DMA
    dma_TxBegin(&g_tUart4); // 开始DMA发送
#else
    USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
#endif
    USART_ITConfig(UART4, USART_IT_TC, ENABLE);
}

/**
 * @brief UART4 GPIO Config
 */
void uart4_Config(void)
{
    /* clock config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = UART4_PINTX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART4_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = UART4_PINRX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART4_PORT, &GPIO_InitStructure);

    /* mode config */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = UART4_BAUD;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);
    uart_InitUart(UART4);

    /* NVIC config, Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
    USART_ITConfig(UART4, USART_IT_TC, DISABLE);

#if UART4_DMA
    uart4_ConfigDMA();
#else
    /* Enable Rx */
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
#endif
    g_tUart4.isReady = 1;
}
