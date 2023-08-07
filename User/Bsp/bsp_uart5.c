#include "bsp_uart.h"

UART_T g_tUart5;

void UART5_IRQHandler(void)
{
    /* 接收中断  */
    uart_IRQ_Rx(&g_tUart5);
    if (uart_IRQ_RxEnd(&g_tUart5))
    {
        isr_sem_send(&semaphore_uart5);
    }
    /* 发送中断 */
    uart_IRQ_Tx(&g_tUart5);
    if (uart_IRQ_TxEnd(&g_tUart5))
    {
    }
}
void uart5_Send(uint8_t *buf, uint16_t len)
{
    if (!g_tUart5.isReady)
        return;
    /* 等待上一次发送完成 */
    while (1)
    {
        if (buffer_dataCount(g_tUart5.bufTx) == 0)
            break;
    }
    /* 将新数据填入发送缓冲区 */
    buffer_put(g_tUart5.bufTx, buf, len);
    USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
    USART_ITConfig(UART5, USART_IT_TC, ENABLE);
}

void uart5_Config(void)
{
    /* clock config*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    /* GPIO config, TX = PC10 RX = PC11*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = UART5_PINTX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART5_PORT_TX, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = UART5_PINRX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART5_PORT_RX, &GPIO_InitStructure);

    /* mode config */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = UART5_BAUD;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART5, &USART_InitStructure);
    uart_InitUart(UART5);

    /* NVIC config, Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);
    USART_ITConfig(UART5, USART_IT_TC, DISABLE);
    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

    g_tUart5.isReady = 1;
}

// void uart5_ConfigExti(void)
// {
//     USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);
//     USART_Cmd(UART5, DISABLE);

//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_InitStructure.GPIO_Pin = UART5_PINRX;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_Init(UART5_PORT_RX, &GPIO_InitStructure);
// }
