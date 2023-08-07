
#ifndef __BSP_RTC__
#define __BSP_RTC__

#include "platform.h"

extern void rtc_Config(void);
extern void rtc_ReadClock(u8 *buf);
extern void rtc_WriteClock(u8 *buf);
extern void rtc_ClearAlarm(void);
extern void rtc_SetAlarmAt(u8 *buf);
extern void rtc_SetAlarmAfter(u32 sec);
extern bool isLeap(u16 y);
#endif
