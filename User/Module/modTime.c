#include "modTime.h"
#include "KuFrame/kuconvert.h"
#include <math.h>
#include <main.h>

#define USE_RTC

#ifdef USE_RTC
#include "bsp_rtc.h"
#define getClock rtc_ReadClock
#define setClock rtc_WriteClock
#define setAlarmAt rtc_SetAlarmAt
#define setAlarmAfter rtc_SetAlarmAfter

#else
#include "bsp_ds1302.h"
#define getClock ds1302_ReadBytes
#define setClock ds1302_WriteBytes
#endif

// JulianTime julianTime;
void time_init(void)
{
    Time t;
    //    time_now(&t);
    t.year = 21;
    t.month = 1;
    t.day = 1;
    t.hour = 0;
    t.minute = 0;
    t.second = 0;
    time_set(&t);
}
void time_now(Time *t)
{
    U8 datas[8];
    getClock(datas);
#ifdef USE_RTC
    t->year = (datas[0]);
    t->month = (datas[1]);
    t->day = (datas[2]);
    t->hour = (datas[3]);
    t->minute = (datas[4]);
    t->second = (datas[5]);
#else
    t->year = bcdToDec(datas[6]);
    t->month = bcdToDec(datas[4]);
    t->day = bcdToDec(datas[3]);
    t->hour = bcdToDec(datas[2]);
    t->minute = bcdToDec(datas[1]);
    t->second = bcdToDec(datas[0]);
#endif
}
int time_day(void)
{
    U8 datas[8];
    getClock(datas);
#ifdef USE_RTC
    return datas[2];
#else
    return bcdToDec(datas[3]);
#endif
}
int time_hour(void)
{
    U8 datas[8];
    getClock(datas);
#ifdef USE_RTC
    return datas[3];
#else
    return bcdToDec(datas[2]);
#endif
}
int time_minute(void)
{
    U8 datas[8];
    getClock(datas);
#ifdef USE_RTC
    return datas[4];
#else
    return bcdToDec(datas[1]);
#endif
}
void time_set(Time *t)
{
    U8 datas[8] = {0};
#ifdef USE_RTC
    datas[0] = (t->year);
    datas[1] = (t->month);
    datas[2] = (t->day);
    datas[3] = (t->hour);
    datas[4] = (t->minute);
    datas[5] = (t->second);
#else
    datas[6] = decToBcd(t->year);
    datas[4] = decToBcd(t->month);
    datas[3] = decToBcd(t->day);
    datas[2] = decToBcd(t->hour);
    datas[1] = decToBcd(t->minute);
    datas[0] = decToBcd(t->second);
#endif
    setClock(datas);
}
void time_parse(const char *s, Time *t)
{ // 2021-01-01 11:11:11
    t->year = (s[2] - 0x30) * 10 + (s[3] - 0x30);
    t->month = (s[5] - 0x30) * 10 + (s[6] - 0x30);
    t->day = (s[8] - 0x30) * 10 + (s[9] - 0x30);
    t->hour = (s[11] - 0x30) * 10 + (s[12] - 0x30);
    t->minute = (s[14] - 0x30) * 10 + (s[15] - 0x30);
    t->second = (s[17] - 0x30) * 10 + (s[18] - 0x30);
}

void time_SetAlarmAt(Time *t)
{
    U8 datas[8] = {0};
#ifdef USE_RTC
    datas[0] = (t->year);
    datas[1] = (t->month);
    datas[2] = (t->day);
    datas[3] = (t->hour);
    datas[4] = (t->minute);
    datas[5] = (t->second);
    setAlarmAt(datas);
#endif
    char strTemp[LEN_INCIDENTINFO];
    sprintf(strTemp, "Set Alarm at 20%02d-%02d-%02d %02d:%02d:%02d",
            t->year, t->month, t->day, t->hour, t->minute, t->second);
    printLog(strTemp);
}
void time_SetAlarmAfter(int sec)
{
#ifdef USE_RTC
    setAlarmAfter(sec);
#endif
}

unsigned int time_toJulianDate(int y, int m, int d)
{
    unsigned int b, jd;
    if (m < 3)
    {
        m += 12;
        y--;
    }
    if ((y > 1582) || (y == 1582 && m > 10) || (y == 1582 && m == 10 && d >= 15))
        b = 2 - (y / 100) + (y / 400); // 公元1582年10月15日以后每400年减少3闰
    jd = 365.25 * (y + 4716) + round(30.6001 * (m + 1) + d + b - 1524.5);
    return jd;
}

int time_daysOfMonth(int y, int m)
{
    switch (m)
    {
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        /* 1. 非整百年：能被4整除的为闰年。
           2、整百年：能被400整除的是闰年
           闰年 29天 其他 28天
        */
        if (y < 100)
            y += 2000; // 如果输入的年份只有最后两位，补足
        return ((y % 4 == 0) && (y % 100 != 0 || y % 400 == 0)) ? 29 : 28;
    default:
        return 31;
    }
}
int time_daysOfYear(int y)
{
    if (y < 100)
        y += 2000; // 如果输入的年份只有最后两位，补足
    return ((y % 4 == 0) && (y % 100 != 0 || y % 400 == 0)) ? 366 : 365;
}
int time_daysInYear(int y, int m, int d)
{
    int count = 0;
    for (int i = 1; i < m; i++)
        count += time_daysOfMonth(y, i);
    count += d;
    return count;
}

int time_dateDiff(Time *t)
{
    unsigned int date1, date2;
    Time t0;
    time_now(&t0);
    date1 = time_daysInYear(t->year, t->month, t->day);
    date2 = time_daysInYear(t0.year, t0.month, t0.day);
    if (t0.year > t->year)
    { // 只考虑跨1年的情况, TODO
        date2 += time_daysOfYear(t->year);
    }
    return date2 - date1 + 1;
}
/**
 * @brief 计算时间差
 * @param t1
 * @param t2
 * @return int 秒
 */
int time_timeDiff(Time *t1, Time *t2)
{
    int sub1 = t2->hour - t1->hour;
    int sub2 = sub1 * 60 + t2->minute - t1->minute;
    int sub3 = sub2 * 60 + t2->second - t1->second;
    //    printf("time_timeDiff: %d\r\n", sub3);
    return sub3;
}

void time_addMonth(Time *t, int m)
{ // month : 1-12
    int v = t->month;
    v += m;
    while (v <= 0)
    {
        v += 12;
        t->year--;
    }
    while (v > 12)
    {
        v -= 12;
        t->year++;
    }
    t->month = v;
}
void time_addDay(Time *t, int d)
{
    int v = t->day;
    v += d;
    while (v <= 0)
    {
        time_addMonth(t, -1);
        v += time_daysOfMonth(t->year, t->month); // 这两行顺序不能换
    }
    while (v > time_daysOfMonth(t->year, t->month))
    {
        v -= time_daysOfMonth(t->year, t->month); // 这两行顺序不能换
        time_addMonth(t, 1);
    }
    t->day = v;
}
void time_addHour(Time *t, int h)
{
    int v = t->hour;
    v += h;
    while (v < 0)
    {
        time_addDay(t, -1);
        v += 24;
    }
    while (v >= 24)
    {
        time_addDay(t, 1);
        v -= 24;
    }
    t->hour = v;
}
void time_addMinute(Time *t, int min)
{
    int v = t->minute;
    v += min;
    while (v < 0)
    {
        v += 60;
        time_addHour(t, -1);
    }
    while (v >= 60)
    {
        v -= 60;
        time_addHour(t, 1);
    }
    t->minute = v;
}
void time_addSecond(Time *t, int sec)
{
    int v = t->second;
    v += sec;
    while (v < 0)
    {
        v += 60;
        time_addMinute(t, -1);
    }
    while (v >= 60)
    {
        v -= 60;
        time_addMinute(t, 1);
    }
    t->second = v;
}

/**
 * @brief 时间比较
 * @return int <0:t1<t2, =0:t1=t2, >0:t1>t2
 */
int time_compare(Time *t1, Time *t2)
{
    int ret = 0;
    ret = t1->year - t2->year;
    if (ret != 0)
        return ret;
    ret = t1->month - t2->month;
    if (ret != 0)
        return ret;
    ret = t1->day - t2->day;
    if (ret != 0)
        return ret;
    ret = t1->hour - t2->hour;
    if (ret != 0)
        return ret;
    ret = t1->minute - t2->minute;
    if (ret != 0)
        return ret;
    ret = t1->second - t2->second;
    return ret;
}

void time_FromString(Time *t, const char *fmt, char *text)
{
    sscanf(text, fmt, &t->year, &t->month, &t->day, &t->hour, &t->minute, &t->second);
}
void time_ToString(Time *t, const char *fmt, char *text)
{
    sprintf(text, fmt, t->year, t->month, t->day, t->hour, t->minute, t->second);
}

// void time_getJulianTime()
//{
//     Time t;
//     time_now(&t);
//     julianTime.date = time_daysInYear(t.year, t.month, t.day);
//     julianTime.hour = decToBcd(t.hour);
//     julianTime.minute = decToBcd(t.minute);
// }
