#include "bsp_rtc.h"
#include "gtypes.h"

// 开启备份区域写入权限	和 配置时钟
#define PWR_BackupAccessCmd_ENABLE                                           \
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); \
    PWR_BackupAccessCmd(ENABLE);
// 关闭备份区域写入权限	和 配置时钟
#define PWR_BackupAccessCmd_DISABLE PWR_BackupAccessCmd(DISABLE)
/* 默认时间为 2021-01-01 00:00:00 */
#define DEFAULTCOUNTER 0x5FEE6600

extern volatile char curFlow;
extern void power_Normal(void);

/**
 * @brief           判断是否为闰年
 * @param y         年
 * @return true     闰年
 * @return false    平年
 */
bool isLeap(u16 y)
{
    if (y % 4 != 0)
        return 0; /* 必须能被4整除 */
    if (y % 100 != 0)
        return 1;
    return (y % 400 == 0); /* 如果以00结尾,还要能被400整除 */
}

static u8 daysOfMonth(u16 y, u8 m)
{
    switch (m)
    {
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return isLeap(y) ? 29 : 28;
    default:
        return 31;
    }
}
static u32 secondsOfYear(u16 y) { return isLeap(y) ? 31622400 : 31536000; }
static u32 secondOfMonth(u16 y, u8 m) { return (u32)daysOfMonth(y, m) * 86400; }

static u32 getCounter(u8 *buf)
{
    u32 counter = 0;
    u16 t;
    u16 y = buf[0] + 2000;
    /* 计算从 1970-01-01 00:00:00 到现在的秒数 */
    for (t = 1970; t < y; t++) /* 把所有年份的秒钟相加 */
        counter += secondsOfYear(t);
    for (t = 1; t < buf[1]; t++)        /* 把前面月份的秒钟数相加 */
        counter += secondOfMonth(y, t); /* 月份秒钟数相加 */

    counter += (u32)(buf[2] - 1) * 86400; /* 把前面日期的秒钟数相加 */
    counter += (u32)buf[3] * 3600;        /* 小时秒钟数 */
    counter += (u32)buf[4] * 60;          /* 分钟秒钟数 */
    counter += buf[5];                    /* 最后的秒钟加上去 */
    return counter;
}
static void getTime(u32 counter, u8 *buf)
{
    u16 y0;
    u32 t0;
    for (int i = 0; i < 6; i++)
        buf[i] = 0;
    /* 年，从1970开始, 只取最后2位 */
    //    counter = DEFAULTCOUNTER;
    y0 = 1970;
    while (1)
    {
        t0 = secondsOfYear(y0);
        if (counter >= t0)
            counter -= t0;
        else
            break;
        y0++;
    }
    if (y0 <= 2000)
        buf[0] = 0;
    else
        buf[0] = (u8)(y0 - 2000);
    /* 月，从1开始 */
    buf[1] = 1;
    while (1)
    {
        t0 = secondOfMonth(y0, buf[1]);
        if (counter > t0)
            counter -= t0;
        else
            break;
        buf[1]++;
    }
    /* 日 从1开始 */
    if (counter > 86400)
    {
        buf[2] = counter / 86400;
        counter = counter - buf[2] * 86400;
    }
    buf[2]++;
    /* 时 */
    if (counter >= 3600)
    {
        buf[3] = counter / 3600;
        counter = counter - buf[3] * 3600;
    }
    /* 分 */
    if (counter >= 60)
    {
        buf[4] = counter / 60;
        counter = counter - buf[4] * 60;
    }
    /* 秒 */
    buf[5] = counter;
}

// RTC时钟中断
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET) // 秒钟中断
    {
        RTC_ClearITPendingBit(RTC_IT_SEC);
        RTC_WaitForLastTask();
    }
    if (RTC_GetITStatus(RTC_IT_ALR) != RESET) // 秒钟中断
    {
        RTC_ClearITPendingBit(RTC_IT_ALR);
        RTC_WaitForLastTask();
        if (curFlow == PowerSaveFlow)
        {
            power_Normal();
            curFlow = AlarmWakeupFlow;
        }
    }
}

void RTCAlarm_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_ALR) != RESET) // 秒钟中断
    {
        // 清除外部中断标志
        EXTI_ClearITPendingBit(EXTI_Line17);
        if (PWR_GetFlagStatus(PWR_FLAG_WU) != RESET)
        { // 清除唤醒标志
            PWR_ClearFlag(PWR_FLAG_WU);
        }
        RTC_WaitForLastTask();
        RTC_ClearITPendingBit(RTC_IT_ALR);
        RTC_WaitForLastTask();
        if (curFlow == PowerSaveFlow)
        {
            power_Normal();
            curFlow = AlarmWakeupFlow;
        }
    }
}

/**
 * @brief           读取时间
 * @param buf       读取到的时间数据, 6个字节 年，月，日，时，分，秒
 */
void rtc_ReadClock(u8 *buf) { getTime(RTC_GetCounter(), buf); }
/**
 * @brief           设置时间
 * @param buf       时间, 6个字节 年，月，日，时，分，秒
 */
void rtc_WriteClock(u8 *buf)
{
    PWR_BackupAccessCmd_ENABLE;
    RTC_SetCounter(getCounter(buf));
    RTC_WaitForLastTask(); /* 必须加 PWR_BackupAccessCmd(ENABLE); 不然进入死循环 */
    PWR_BackupAccessCmd_DISABLE;
    RTC_WaitForLastTask();
}

void rtc_ClearAlarm(void)
{
    RTC_ITConfig(RTC_IT_ALR, DISABLE);
    RTC_WaitForLastTask();
}
/**
 * @brief           设置指定时间的闹钟
 * @param buf       时间数据数组, 6个字节 年，月，日，时，分，秒
 */
void rtc_SetAlarmAt(u8 *buf)
{
    PWR_BackupAccessCmd_ENABLE;
    RTC_SetAlarm(getCounter(buf));
    RTC_WaitForLastTask();
    PWR_BackupAccessCmd_DISABLE;
    RTC_WaitForLastTask();
}
/**
 * @brief           设置几秒后的闹钟
 * @param sec       要延迟的秒数
 */
void rtc_SetAlarmAfter(u32 sec)
{
    PWR_BackupAccessCmd_ENABLE;
    RTC_SetAlarm(RTC_GetCounter() + sec);
    RTC_WaitForLastTask();
    PWR_BackupAccessCmd_DISABLE;
    RTC_WaitForLastTask();
}

void rtc_ConfigAlarm(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    // 配置EXTI_Line17(RTC Alarm)上升沿触发
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line17);

    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 必须比RTC全局中断的优先级高
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    RTC_WaitForLastTask();
}
void rtc_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    { /* BKP的后备寄存器1中，存了一个特殊字符0xA5A5, 该寄存器数据丢失，表明RTC数据丢失，需要重新配置 */
        PWR_BackupAccessCmd_ENABLE;
        /* Reset Backup Domain */ /* 将外设BKP的全部寄存器重设为缺省值 */
        BKP_DeInit();
        /* Enable LSE */
        RCC_LSEConfig(RCC_LSE_ON);
        /* Wait till LSE is ready */ /* 等待外部晶振震荡稳定输出 */
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
            ;
        /* Select LSE as RTC Clock Source */ /*使用外部32.768KHz晶振作为RTC时钟 */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        /* Enable RTC Clock */
        RCC_RTCCLKCmd(ENABLE);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();
        /* Set RTC prescaler: set RTC period to 1sec */
        RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        RTC_SetCounter(DEFAULTCOUNTER);
        RTC_WaitForLastTask();
        /* 配置完成后，向后备寄存器中写特殊字符0xA5A5 */
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
        PWR_BackupAccessCmd_DISABLE;
    }
    else
    { /* 若后备寄存器没有掉电，则无需重新配置RTC */
        /* 上电复位 */
        RTC_WaitForSynchro();
        // 使能秒中断
        //  RTC_ITConfig(RTC_IT_SEC, ENABLE);
        // 等待操作完成
        RTC_WaitForLastTask();
    }

    /* 开启RTC 中断 */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC, ENABLE); // 省电状态进闹钟中断需要开启
    RTC_WaitForLastTask();

    rtc_ConfigAlarm();
}
