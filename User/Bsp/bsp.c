/**
 * @brief 驱动层设置
 * @author kuais (dlm321@126.com)
 * @version 1.0
 * @date 2021-06-10 10:07:08
 */
#include "bsp.h"
#include "main.h"

/**
 * @brief   重启软件，特权级下调用
 */
void bsp_SystemReset(void)
{
    DISABLE_INT(); // 关中断
    bsp_delay(10);
    NVIC_SystemReset();
}
/**
 * @brief   重启软件，用户级下调用
 */
void bsp_Restart(void)
{
    DISABLE_INT(); // 关中断
    powerOff_RelayBoard;
    powerOff_Scanner;
    powerOff_BLE;
    powerOff_Route;
    os_dly_wait(5000);
    SVC_2_FunCall();
}
void __SVC_1()
{
    /* 开启外部中断 */
    exti_Switch(1);
    /* 进入停机模式，设置电压调节器为低功耗模式，等待中断唤醒 */
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}
void __SVC_2() { NVIC_SystemReset(); }

#ifdef FUNCTION_WATCHDOG
void iwdg_Feed(void)
{
    IWDG_ReloadCounter();
}
/**
 * @brief 初始化看门狗
 */
void iwdg_Config(void)
{
    /* LSI的启动*/
    RCC_LSICmd(ENABLE); // 打开LSI
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ;                                         // 等待直到LSI稳定
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); // 访问之前要首先使能寄存器写
    IWDG_SetPrescaler(IWDG_Prescaler_64);         // 64分频 一个周期1.6ms
    IWDG_SetReload(4000);                         // 最长12位 [0,4096] 4096*1.6=6.55S
}
void iwdg_Start(void)
{
    iwdg_Config();
    iwdg_Feed();   /* Reload IWDG counter */
    IWDG_Enable(); // Enable IWDG (the LSI oscillator will be enabled by hardware
}
void iwdg_Stop(void)
{
    RCC_LSICmd(DISABLE); // 关闭LSI
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ; // 等待直到LSI稳定
}
#endif

/**
 * @brief  停机唤醒后配置系统时钟: 使能 HSE, PLL
 *         并且选择PLL作为系统时钟.
 * @param  None
 * @retval None
 */
void sysClk_Config(void)
{
    RCC_DeInit();                           /*把RCC外设初始化为复位状态*/
    RCC_HSEConfig(RCC_HSE_ON);              /* 使能 HSE */
    if (RCC_WaitForHSEStartUp() == SUCCESS) /*选择HSE作为PLL时钟源，将HSE二分频后再二倍频*/
    {
        // AHB预分频因子设置为1分频，HCLK = SYSCLK
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        // APB2预分频因子设置为1分频，PCLK2 = HCLK
        RCC_PCLK2Config(RCC_HCLK_Div1);
        // APB1预分频因子设置为1分频，PCLK1 = HCLK/2
        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    }
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) /* 等待PLL准备就绪 */
    {
    }
    RCC_PLLCmd(ENABLE);                                 /* 使能PLL */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) /* 等待PLL准备就绪 */
    {
    }
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK); /*选择PLL作为系统时钟源 */
    while (RCC_GetSYSCLKSource() != 0x08)      /* 等待PLL被选择为系统时钟源 */
    {
    }
}

/**
 * @brief 驱动层初始设置
 */
void bsp_Config(void)
{
    /* 开启用到的RCC时钟总线 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 禁止JTAG使用 将PB3,PB4用作普通IO
    // GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // 禁止SWD使用

    /* 系统时钟 */
    systick_Init(72);
    rtc_Config();
    adc_Config();
    power_Config();
    keypad_Config();
    uart_Config();
    // ds1302_Config();
    e2p_Config();
    spi1_Config();
    spi2_Config();
    w5500_Config();
    exti_Config();

    /* IO口配置完成，执行其他初始化操作 */
    //    sf_Config();
}

void bsp_delay(int n)
{
    while (n-- > 0)
        __NOP();
}
void bsp_Beep(int ms)
{
    powerOn_Beep;
    // 	delay_ms(ms);  // 只能在特权级调用
    //    ms *= 10000;
    //    while (ms-- > 0)
    //        __NOP();
    os_dly_wait(ms);
    powerOff_Beep;
}

void bsp_BeepSome(int count, int ms)
{
    while (count--)
    {
        bsp_Beep(ms);
        os_dly_wait(100);
    }
}
/**
 * @brief 获取当前电压
 * 参考电压3.3V 对应 4096, 电阻比为 (22 + 4.7) / 4.7 = 5.68
 */
u16 bsp_Voltage(void)
{
    u16 v = adc_Value(0);
    return (u16)(v * 330 * 5.68 / 4096);
}

// 重定向c库函数printf到USART2
int fputc(int c, FILE *f)
{
    uart2_Send((uint8_t *)&c, 1);
    return (c);
}

////重定向c库函数scanf到USART1
// int fgetc(FILE *f)
//{
//	return 0;
//}
