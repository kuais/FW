#include "bsp_uart.h"

UART_T g_tUart2;

#if UART2_DMA
static void uart2_ConfigDMA(void)
{
    /* DMA clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    /* fill init structure */
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DR;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 以字节为传输单位
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // 外设地址寄存器不变-1个外设
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // DMA内存基地址
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // 内存地址寄存器递增
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;

    /* DMA TX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; // 外设为数据传输的目的地
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;     // DMA内存基地址
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_DeInit(UART2_DMA_ChTx);
    DMA_Init(UART2_DMA_ChTx, &DMA_InitStructure);
    DMA_Cmd(UART2_DMA_ChTx, DISABLE); // 使DMA TX通道停止工作
    /* DMA RX CHANNEL Config */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // 外设为数据传输的目的地
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)g_tUart2.bufDma;
    DMA_InitStructure.DMA_BufferSize = DMA_RX_BUF_SIZE;
    DMA_DeInit(UART2_DMA_ChRx);
    DMA_Init(UART2_DMA_ChRx, &DMA_InitStructure);
    DMA_Cmd(UART2_DMA_ChRx, ENABLE); // 使DMA RX 通道开始工作

    /* DMA 中断 */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_Init(&NVIC_InitStructure);
    DMA_ITConfig(UART2_DMA_ChRx, DMA_IT_TC, ENABLE); // 配置DMA接收缓冲区满后产生中断

    // NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    // NVIC_Init(&NVIC_InitStructure);
    // DMA_ITConfig(UART2_DMA_ChTx, DMA_IT_TC, ENABLE); //配置DMA接收缓冲区满后产生中断

    /* Enable DMA request */
    USART_DMACmd(USART2, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
}
void DMA1_Channel6_IRQHandler(void)
{ // DMA接收中断
    if (DMA_GetFlagStatus(DMA1_FLAG_TC6))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        dma_RxEnd(&g_tUart2);
        dma_RxBegin(&g_tUart2);
    }
}
// void DMA1_Channel7_IRQHandler(void)
//{ // DMA发送中断
//     if (DMA_GetFlagStatus(DMA1_FLAG_TC7))
//     {
//         DMA_ClearITPendingBit(DMA1_IT_TC7);
//         dma_TxEnd(&g_tUart2);
//     }
// }
#endif
void USART2_IRQHandler(void)
{
    /* 接收中断  */
    uart_IRQ_Rx(&g_tUart2);
    if (uart_IRQ_RxEnd(&g_tUart2))
    {
#if UART2_DMA
        /* DMA 缓存大于1的时候下面语句不能注释 */
        // dma_RxEnd(&g_tUart2);
        // dma_RxBegin(&g_tUart2);、
#endif
        isr_sem_send(&semaphore_uart2);
    }
    /* 发送中断 */
    uart_IRQ_Tx(&g_tUart2);
    if (uart_IRQ_TxEnd(&g_tUart2))
    {
#if UART2_DMA
        dma_TxEnd(&g_tUart2);
#endif
    }
}
void uart2_Send(uint8_t *buf, uint16_t len)
{
    if (!g_tUart2.isReady)
        return;
    int count = 0xFFFF;
    /* 等待上一次发送完成 */
    while (1)
    {
        if (buffer_dataCount(g_tUart2.bufTx) == 0)
            break;
        if (count-- == 0)
            dma_TxEnd(&g_tUart2);
    }
    /* 将新数据填入发送缓冲区 */
    buffer_put(g_tUart2.bufTx, buf, len);
#if UART2_DMA
    dma_TxBegin(&g_tUart2); // 开始DMA发送
#else
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
#endif
    USART_ITConfig(USART2, USART_IT_TC, ENABLE);
}

/**
 * @brief USART2 GPIO Config
 */
void uart2_Config(void)
{
    /* clock config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* GPIO config, TX = PA2 RX = PA3*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = UART2_PINTX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART2_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = UART2_PINRX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART2_PORT, &GPIO_InitStructure);

    /* mode config */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = UART2_BAUD;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    uart_InitUart(USART2);

    /* NVIC config, Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    USART_ITConfig(USART2, USART_IT_TC, DISABLE);

#if UART2_DMA
    uart2_ConfigDMA();
#else
    /* Enable Rx */
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#endif
    g_tUart2.isReady = 1;
}
