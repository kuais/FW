#include "modRelayBoard.h"
#include "KuFrame/kuconvert.h"
#include "KuFrame/kuutil.h"

#define RB_DEBUG 0
#define RB_OUT0 &g_tUart4
#define RB_OUT1 &g_tUart3

#define RB_CMDSTATUS 0xF2
#define RB_CMDUNLOCK 0xFA

#define RB_LENSTATUS 83

static U8 _Count;
RelayBoard relayBoard[2];

static U8 relayBoard_PackCmd(U8 *p, U8 len)
{
    U8 n = len;
    for (int i = 1; i < len; i++)
        p[n++] = ~p[i];
    p[n++] = (p[0] << 4) + (p[0] >> 4);
    return n;
}
static void relayBoard_CmdOfStatus(U8 *p)
{
    U8 n = 0;
    p[n++] = RB_CMDSTATUS;
    p[n++] = 0x00;
    relayBoard_PackCmd(p, n);
}
static void relayBoard_CmdOfUnlock(U16 relay, U8 *p)
{
    U8 n = 0;
    p[n++] = RB_CMDUNLOCK;
    p[n++] = (relay / boardParams.floorCount) + 1;
    p[n++] = (relay % boardParams.floorCount) + 1;
    relayBoard_PackCmd(p, n);
}

static void relayBoard_ParseRelayInfo(U8 side, U8 *datas, U8 len)
{
    U8 bitCount = boardParams.floorCount * 2; // 有效bit数,1个继电器2bit
    U8 byteCount = (bitCount / 8 + 1);        // 1块锁板占用字节数

    U8 *p = relayBoard[side].relayStatus;
    U8 n1, n2, m1, m2, v;
    int i;
    for (i = 0; i < RELAYCOUNT; i++)
    {
        n1 = (i / boardParams.floorCount) * byteCount;
        if (n1 >= len)
            break; // 超过数据长度
        n2 = (i % boardParams.floorCount) * 2;
        n1 = n1 + n2 / 8;
        n2 = n2 % 8;
        m1 = (i * 2) / 8;
        m2 = (i * 2) % 8;
        v = getBit(datas[n1], n2, 1);
        p[m1] = setBit(p[m1], m2, v, 1);
        v = getBit(datas[n1], n2 + 1, 1);
        p[m1] = setBit(p[m1], m2 + 1, v, 1);
    }
}

/**
 * @brief 获取锁状态 0:开，1：关
 * @param side
 * @param relay
 * @return U8
 */
U8 relayBoard_GetLockStatus(U8 side, U16 relay)
{
    U8 *p = relayBoard[side].relayStatus;
    U8 i1 = (relay * 2) / 8;
    U8 i2 = (relay * 2) % 8;
    return (U8)getBit(p[i1], i2, 1);
}
/**
 * @brief 获取传感器状态 0:有物 1：无物
 * @param side
 * @param relay
 * @return U8
 */
U8 relayBoard_GetSensorStatus(U8 side, U16 relay)
{
    U8 *p = relayBoard[side].relayStatus;
    U8 i1 = (relay * 2) / 8;
    U8 i2 = (relay * 2) % 8 + 1;
    return (U8)getBit(p[i1], i2, 1);
}

void relayBoard_GetAllLockStatus(U8 side, char *t)
{
    U8 *p = relayBoard[side].relayStatus;
    U8 i, i1, i2, i3, b, v = 0;
    for (i = 0; i < 128; i++)
    {
        i1 = (i * 2) / 8;
        i2 = (i * 2) % 8;
        i3 = i % 8;
        b = 1 - getBit(p[i1], i2, 1);
        /* 继电器小的在低位 */
        // b <<= i3;
        // v += b;
        /* 继电器小的在高位 */
        v <<= 1;
        v += b;
        if (i3 == 7)
        {
            *t++ = toHex(v / 16);
            *t++ = toHex(v % 16);
            v = 0;
        }

        // *t = 1 - getBit(p[i1], i2, 1) + 0x30;
        // t++;
    }
}
void relayBoard_GetAllSensorStatus(U8 side, char *t)
{
    U8 *p = relayBoard[side].relayStatus;
    U8 i, i1, i2, i3, b, v = 0;
    for (i = 0; i < 128; i++)
    {
        i1 = (i * 2) / 8;
        i2 = (i * 2) % 8 + 1;
        i3 = i % 8;
        b = 1 - getBit(p[i1], i2, 1);
        /* 继电器小的在低位 */
        // b <<= i3;
        // v += b;
        /* 继电器小的在高位 */
        v <<= 1;
        v += b;
        if (i3 == 7)
        {
            *t++ = toHex(v / 16);
            *t++ = toHex(v % 16);
            v = 0;
        }
        // *t = 1 - getBit(p[i1], i2, 1) + 0x30;
        // t++;
    }
}

void relayBoard_Send(U8 side, U8 *cmd, U16 len)
{
    UART_T *port = (side == 0) ? RB_OUT0 : RB_OUT1;
    uart_Send(port, cmd, len);
}
void relayBoard_Status(U8 side)
{
    U8 cmd[4];
    if (relayBoard[side].errcount < 9)
    {
        relayBoard[side].errcount++;
        if (relayBoard[side].errcount == 2)
        { // 2次传输未收到回复
            memset(relayBoard[side].relayStatus, 0xFF, RELAYDATACOUNT);
            relayBoard[side].errcount = 9;
        }
    }
    relayBoard_CmdOfStatus(cmd);
    relayBoard_Send(side, cmd, 4);
}
void relayBoard_Unlock(U8 side, U16 relay)
{
    U8 cmd[6];
    relayBoard_CmdOfUnlock(relay, cmd);
    relayBoard_Send(side, cmd, 6);
}
/**
 * @brief 3秒内查询锁状态
 * @param side  位置        0：out1; 1:out2
 * @param relay 继电器号    0-255
 * @return U8   0: 开启； 1：未开启
 */
U8 relayBoard_watiLockOpen(U8 side, U16 relay)
{
    U8 status = 1;
    U8 count = 10; // 10 * 500ms = 5s
    U8 statusBak = 1;
    while (count-- > 0)
    {
        status = relayBoard_GetLockStatus(side, relay);
        if (status == 0)
        {
            if (statusBak == 0)
                break;
        }
        if (statusBak != status)
            statusBak = status;
        os_dly_wait(500);
    }
    return status;
}

void relayBoard_Init(void)
{
    _Count = 0;
    memset(relayBoard[0].relayStatus, 0xFF, RELAYDATACOUNT);
    memset(relayBoard[1].relayStatus, 0xFF, RELAYDATACOUNT);
}
void relayBoard_Handle(UART_T *port)
{
    U8 side = (port == RB_OUT0) ? 0 : 1;
    KuBuffer *buf = port->bufRx;
    U16 len0 = buffer_dataCount(buf);
    if (len0 == 0)
        return;
    U8 *p = buffer_get(buf, 0);
#if RB_DEBUG
    printf("rb-%d recv: ", side);
    for (int i = 0; i < len0; i++)
        printf("%c", *(p + i));
    printf("\r\n");
#endif
    int len;
    while (len0 > 0)
    {
        switch (p[0])
        {
        case RB_CMDSTATUS:
        {
            len = RB_LENSTATUS;
            if (len0 < len)
                return;
            relayBoard[side].errcount = 0; // 收到回复，清errcount
            relayBoard_ParseRelayInfo(side, p + 1, len - 3);
            break;
        }
        default:
        {
            len = 1; // TODO 测试下 len = len0 是否有问题
            break;
        }
        }
        buffer_remove(buf, len);
        len0 -= len;
    }
}

void relayBoard_Task(void)
{
    _Count++;
    if (_Count < 3)
        return;
    for (int i = 0; i < 2; i++)
        relayBoard_Status(i);
    _Count = 0;
}
