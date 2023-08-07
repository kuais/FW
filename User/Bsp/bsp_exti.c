#include "bsp_exti.h"

extern void power_Normal(void);

void exti_Config(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    // uint32_t lines = EXTI_Line4 | EXTI_Line8 | EXTI_Line9;

    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //下降沿
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Line = EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line8 | EXTI_Line9;
    EXTI_Init(&EXTI_InitStructure); //初始化外设EXTI寄存器

    /* 按键产生外部中断 */
    /* PA8 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    /* PB4 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);
    /* PC8 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource8);
    /* PC9 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);

    /* 串口通信产生外部中断 */
    /* PA3 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);
    /* 蓝牙输入产生外部中断 */
    /* PD2 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2);
}

/**
 * @brief 开关外部中断
 * UarT2:
 * @param flag
 */
void exti_Switch(int flag)
{

    NVIC_InitTypeDef NVIC_InitStructure;
    uint32_t lines = EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line8 | EXTI_Line9;

    EXTI_ClearITPendingBit(lines);
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; // 抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;        // 子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = flag;                // 使能|关闭外部中断通道

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;   // 外部中断通道[2]
    NVIC_Init(&NVIC_InitStructure);                    // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;   // 外部中断通道[3]
    NVIC_Init(&NVIC_InitStructure);                    // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;   // 外部中断通道[4]
    NVIC_Init(&NVIC_InitStructure);                    // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; // 外部中断通道[9:5]
    NVIC_Init(&NVIC_InitStructure);                    // 根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
}

// 外部中断[2]服务程序
void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    { /* Uart5 */
        EXTI_ClearITPendingBit(EXTI_Line2);
        power_Normal();
    }
}
// 外部中断[3]服务程序
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    { /* Uart2 */
        EXTI_ClearITPendingBit(EXTI_Line3);
        power_Normal();
    }
}
//外部中断[4]服务程序
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line4);
        power_Normal();
    }
}

// 外部中断[9:5]服务程序
void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
        power_Normal();
    }

    if (EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line9);
        power_Normal();
    }
}
