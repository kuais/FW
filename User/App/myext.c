/**
 * @brief 外部函数都放在这里
 * @author kuais (dlm321@126.com)
 * @version 1.0
 * @date 2021-06-25 15:02:48
 */
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"
#include "module/modTime.h"
#include "module/modScanner.h"
#include "KuFrame/kualloc.h"

#define Mem_print // printf
extern MemMgr mmgr1;
extern bool db_newIncident(char type, u16 boxid, char *info);

void https_printf(const char *fmt, ...)
{
    if (!boardParams.debug)
        return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
void dprintf(const char *fmt, ...)
{
    if (!boardParams.debug)
        return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
void printLog(char *info)
{
    printf("%s\r\n", info);
    if (appFlag.synclog)
        db_newIncident(IT_Log, 0, info);
}
/**
 * @brief 生成随机数
 * @param output        输出数据
 * @param len           长度
 * @param olen          输出数据长度
 */
void randomBytes(unsigned char *output, size_t len, size_t *olen)
{
    Time t;
    time_now(&t);
    u32 v = 0;
    v = (t.second << 8) + (t.minute << 4) + t.hour + t.minute + t.second + rand();
    *olen = 0;
    srand(v);
    int temp = rand();
    while (*olen < len)
    {
        output[*olen] = (temp & 0xFF);
        temp >>= 1;
        *olen += 1;
    }
}
/**
 * @brief 生成随机数 只能是数字Ascii码
 * @param output        输出数据
 * @param len           长度
 * @param olen          输出数据长度
 */
void randomDigits(unsigned char *output, size_t len, size_t *olen)
{
    Time t;
    time_now(&t);
    u32 v = 0;
    v = (t.second << 8) + (t.minute << 4) + t.hour + t.minute + t.second + rand();
    *olen = 0;
    srand(v);
    int temp = rand();
    while (*olen < len)
    {
        output[*olen] = (temp % 10) + 0x30;
        temp >>= 1;
        *olen += 1;
    }
}
int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    randomBytes(output, len, olen);
    return 0;
}
/**
 * @brief 休眠 sec 秒
 * @param sec
 */
void sleep(int sec)
{
    tickCheck = 0;
    while (tickCheck < sec)
        os_dly_wait(1);
}
void clearInput(void)
{
    memset(strInput, 0, sizeof(strInput));
}
bool inputOn(void)
{
    if (flagInput == 2)
        return true;
    tickCheck = 0;
    flagInput = 2;
    if (appSet.scanner)
    {
        scanner_SwitchLed(1);
        // os_dly_wait(200);
        scanner_PhoneMode(1);
    }
    //	scanner_Activate();
    clearInput();
    return true;
}
bool inputOff(void)
{
    tickCheck = 0;
    flagInput = 0;
    if (appSet.scanner)
    {
        scanner_PhoneMode(0);
        scanner_SwitchLed(0);
        // os_dly_wait(200);
    }
    // scanner_Deactivate();
    return true;
}
u16 getBoxID(u8 side, u16 relay)
{
    return (2 - side) * 1000 + relay + 1;
}
u8 getSizeValue(int v)
{
    return (v >= 30) ? (v - 30) : (v - 10);
}
void getSizeText(u8 size, char *dst)
{
    switch (size)
    {
    case XS:
        memcpy(dst, "XS", 2);
        break;
    case S:
        memcpy(dst, "S", 1);
        break;
    case M:
        memcpy(dst, "M", 1);
        break;
    case L:
        memcpy(dst, "L", 1);
        break;
    case XL:
        memcpy(dst, "XL", 2);
        break;
    }
}
u32 timeToU32(Time *t)
{
    u32 time = 0;
    time = t->month * 100 + t->day;
    time = time * 10000 + t->hour * 100 + t->minute;
    return time;
}
void getTimeString(char *t, char *text)
{
    sprintf(text, "20%02d-%02d-%02d %02d:%02d:%02d.000", t[0], t[1], t[2], t[3], t[4], t[5]);
}
void getTimeString2(char *t, char *text)
{
    sprintf(text, "20%02d%02d%02d%02d%02d%02d000", t[0], t[1], t[2], t[3], t[4], t[5]);
}
void getSyncId(char *text)
{
    Time t;
    time_now(&t);
    char strTime[25] = {0};
    getTimeString2(&t, strTime);
    sprintf(text, "%s-%s", apmid, strTime); // apmid-time
}

void getReservationText(Event *p, char *text)
{
    Shipment *sp = &p->shipment;
    char strTime[25] = {0};
    getTimeString2(p->time, strTime);
    sprintf(text, "%s,%s,%s,%d,%d,20%s.000,%s", // TrackingId,PluId,Barcode,Pin1,Pin2,EventId,DoorId
            sp->plu, sp->barcode, sp->pin, p->eventType,
            sp->boxid, p->time, p->user);
}
void getIncidentText(Incident *p, char *text)
{
    sprintf(text, "%d,%d,20%s.000,%s", // TypeId,DoorId,Date,,CourierId
            p->type, p->boxid, p->time, p->info);
}
void getTechnicianAction(TechnicianAction *p, char *text)
{
    char strTime[25] = {0};
    getTimeString(p->time, strTime);
    sprintf(text, "%d,%s,20%s.000,", // ActionId,TechnicianId,Date,Info
            p->type, p->user, p->time);
}
void getLabelRuleText(LabelRule *p, char *text)
{
    sprintf(text, "%s,%s,%d,%s,%d,%d", // LabelRule,CodeType,ContentLength,Regex,LabelPositionBegin,LabelPositionEnd
            p->rulename, p->codetype, p->length, p->regex, p->pos0, p->pos1);
}
void *mymalloc(size_t s)
{
    void *p = mmgr1.malloc(s);
    if (p == NULL)
    {
        printf("malloc failed");
        return 0;
    }
    else
    {
        Mem_print("malloc 0x%08x, size 0x%04x\r\n", p, s);
        return p;
    }
}
void *mycalloc(size_t n, size_t s)
{
    void *p = mmgr1.calloc(n, s);
    if (p == NULL)
    {
        printf("calloc failed");
        return 0;
    }
    else
    {
        Mem_print("calloc 0x%08x,size 0x%04x\r\n", p, s);
        return p;
    }
}
void *myrealloc(void *p, size_t s)
{
    p = mmgr1.realloc(p, s);
    Mem_print("realloc 0x%08x,size 0x%04x\r\n", p, s);
    return p;
}
void myfree(void *p)
{
    Mem_print("free %08x\r\n", p);
    mmgr1.free(p);
}
