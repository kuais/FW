#ifndef __MODULE_TIME__
#define __MODULE_TIME__

// typedef __packed struct
//{
//     Time time;
//     Pin pin;
// } PinWithTime;

// typedef struct
//{
//     unsigned short date;
//     unsigned char hour;
//     unsigned char minute;
// } JulianTime;

// extern JulianTime julianTime;

typedef __packed struct
{
    char year;
    char month;
    char day;
    char hour;
    char minute;
    char second;
} Time;

extern int time_daysOfYear(int y);
extern int time_daysOfMonth(int y, int m);
extern int time_daysInYear(int y, int m, int d);
extern int time_dateDiff(Time *t);
extern int time_timeDiff(Time *t1, Time *t2);

extern void time_init(void);
extern void time_parse(const char *s, Time *t);
extern void time_now(Time *t);
extern void time_set(Time *t);
extern void time_SetAlarmAt(Time *t);
extern void time_SetAlarmAfter(int sec);
extern void time_addMonth(Time *t, int month);
extern void time_addDay(Time *t, int day);
extern void time_addHour(Time *t, int hour);
extern void time_addMinute(Time *t, int min);
extern void time_addSecond(Time *t, int sec);

extern int time_day(void);
extern int time_hour(void);
extern int time_minute(void);
extern int time_compare(Time *t1, Time *t2);
extern void time_FromString(Time *t, const char *fmt, char *text);
extern void time_ToString(Time *t, const char *fmt, char *text);
// extern void time_getJulianTime(void);

#endif
