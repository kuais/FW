#include "bsp_uart.h"

UART_T g_tUart3;

#if UART3_DMA
static void uart3_ConfigDMA(void)
{
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    /* fill init structure */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设递增模式-1个外设
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存递增模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 以字节为传输单位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR;
    /* DMA TX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; // 外设为数据传输的目的地
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;     // 先随意设，在发送时设实际值
    DMA_InitStructure.DMA_BufferSize = 1;              // 先随意设，在发送时设实际值
    DMA_DeInit(UART3_DMA_ChTx);
    DMA_Init(UART3_DMA_ChTx, &DMA_InitStructure);
    DMA_Cmd(UART3_DMA_ChTx, DISABLE); // 使DMA TX通道停止工作
    /* DMA RX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // 外设为数据传输的目的地
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_tUart3.bufDma;
    DMA_InitStructure.DMA_BufferSize = DMA_RX_BUF_SIZE;
    DMA_DeInit(UART3_DMA_ChRx);
    DMA_Init(UART3_DMA_ChRx, &DMA_InitStructure);
    DMA_Cmd(UART3_DMA_ChRx, ENABLE); // 使DMA RX 通道开始工作
    /* DMA 中断 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    DMA_ITConfig(UART3_DMA_ChRx, DMA_IT_TC, ENABLE); // 配置DMA接收缓冲区满后产生中断
    // NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    // NVIC_Init(&NVIC_InitStructure);
    // DMA_ITConfig(UART3_DMA_ChTx, DMA_IT_TC, ENABLE); //配置DMA接收缓冲区满后产生中断
    /* Enable DMA request */
    USART_DMACmd(USART3, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
}
void DMA1_Channel3_IRQHandler(void)
{ // DMA接收中断
    if (DMA_GetFlagStatus(DMA1_FLAG_TC3))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC3);
        dma_RxEnd(&g_tUart3);
        dma_RxBegin(&g_tUart3);
    }
}
// void DMA1_Channel2_IRQHandler(void)
// { // DMA发送中断
//     if (DMA_GetFlagStatus(DMA1_FLAG_TC2))
//     {
//         DMA_ClearITPendingBit(DMA1_IT_TC2);
//         dma_TxEnd(&g_tUart3);
//     }
// }
#endif

void USART3_IRQHandler(void)
{
    /* 接收中断  */
    uart_IRQ_Rx(&g_tUart3);
    if (uart_IRQ_RxEnd(&g_tUart3))
    {
#if UART3_DMA
        /* DMA 缓存大于1的时候下面语句不能注释 */
        // dma_RxEnd(&g_tUart3);
        // dma_RxBegin(&g_tUart3);
#endif
        isr_sem_send(&semaphore_uart3);
    }
    /* 发送中断 */
    uart_IRQ_Tx(&g_tUart3);
    if (uart_IRQ_TxEnd(&g_tUart3))
    {
#if UART3_DMA
        dma_TxEnd(&g_tUart3);
#endif
    }
}
void uart3_Send(uint8_t *buf, uint16_t len)
{
    if (!g_tUart3.isReady)
        return;
    /* 等待上一次发送完成 */
    while (1)
    {
        if (buffer_dataCount(g_tUart3.bufTx) == 0)
            break;
    }
    /* 将新数据填入发送缓冲区 */
    buffer_put(g_tUart3.bufTx, buf, len);
#if UART3_DMA
    dma_TxBegin(&g_tUart3); // 开始DMA发送
#else
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
#endif
    USART_ITConfig(USART3, USART_IT_TC, ENABLE);
}

/**
 * @brief USART3 GPIO Config
 */
void uart3_Config(void)
{
    /* clock config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* GPIO config, TX = PB10 RX = PB11*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = UART3_PINTX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART3_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = UART3_PINRX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART3_PORT, &GPIO_InitStructure);

    /* mode config */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = UART3_BAUD;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);
    uart_InitUart(USART3);

    /* NVIC config, Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    USART_ITConfig(USART3, USART_IT_TC, DISABLE);

#if UART3_DMA
    uart3_ConfigDMA();
#else
    /* Enable Rx */
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
#endif
    g_tUart3.isReady = 1;
}
