#include "modE2p.h"
#include "KuFrame/kuutil.h"
#include "main.h"
#include <stdio.h>

#define APIURL "https://internalapinoscreen-dev.azurewebsites.net"

void e2p_Init(void)
{
    //	U8 temp[100] = {0};
    /* 初始化硬件参数 */
    e2p_SetApiUrl((U8 *)APIURL); // 默认api地址
    //	e2p_GetApiUrl(temp);
    os_dly_wait(10);
    e2p_SetApmID("UPS0001"); // 默认ApmID UPS0001
    //	e2p_GetApmID(temp);
    //	os_dly_wait(5);
    e2p_SetBleID("5979909315"); // 默认蓝牙ID
    //	e2p_GetBleID(temp);
    e2p_SetFloorCount(12); // 默认层数12
    //	e2p_GetFloorCount(temp);
    e2p_SetBleFunc(1); // 默认开启蓝牙功能
    //	e2p_GetBleFunc(temp);
    /* 清空数据区 */
    e2p_Clear();
    /* 初始化网络参数 */
    U8 datas[12] = {192, 168, 8, 188, 255, 255, 255, 0, 192, 168, 8, 1};
    e2p_SetNetParams(datas);
    /* 初始化 AES-KEY */
    U8 arr[16] = {0x04, 0x91, 0xDD, 0xD1, 0x9B, 0x34, 0xF7, 0xD6, 0x89, 0xC1, 0x29, 0x48, 0x39, 0x56, 0x91, 0x85};
    e2p_SetAesKey(arr);
    /* 置初始化标志 */
    e2p_SetFlag_Init(0x0);
}
void e2p_Clear(void)
{
}

void e2p_GetFlag_Init(U8 *p) { *p = e2p_ReadByte(ADDR_INITFLAG); }
void e2p_SetFlag_Init(U8 v) { e2p_WriteByte(ADDR_INITFLAG, v); }
void e2p_GetApmID(U8 *p) { e2p_ReadBytes(ADDR_APMID, p, 10); }
void e2p_SetApmID(U8 *p) { e2p_WriteBytes(ADDR_APMID, p, 10); }
void e2p_GetFloorCount(U8 *p) { *p = e2p_ReadByte(ADDR_FLOOR); }
void e2p_SetFloorCount(U8 v) { e2p_WriteByte(ADDR_FLOOR, v); }
void e2p_GetBleID(U8 *p) { e2p_ReadBytes(ADDR_BLEID, p, 10); }
void e2p_SetBleID(U8 *p) { e2p_WriteBytes(ADDR_BLEID, p, 10); }
void e2p_GetBleFunc(U8 *p) { *p = e2p_ReadByte(ADDR_BLEFUNC); }
void e2p_SetBleFunc(U8 v) { e2p_WriteByte(ADDR_BLEFUNC, v); }
void e2p_GetApiUrl(U8 *p) { e2p_ReadBytes(ADDR_APIURL, p, 96); }
void e2p_SetApiUrl(U8 *p) { e2p_WriteBytes(ADDR_APIURL, p, 96); }
void e2p_GetAesKey(U8 *p)
{
    // U8 arr[16] = {0x04, 0x91, 0xDD, 0xD1, 0x9B, 0x34, 0xF7, 0xD6, 0x89, 0xC1, 0x29, 0x48, 0x39, 0x56, 0x91, 0x85};
    // U8 arr[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    // e2p_SetAesKey(arr);
    e2p_ReadBytes(ADDR_AESKEY, p, 16);
}
void e2p_SetAesKey(U8 *p) { e2p_WriteBytes(ADDR_AESKEY, p, 16); }
void e2p_GetBleName(U8 *p) { e2p_ReadBytes(ADDR_BLENAME, p, 32); }
void e2p_SetBleName(U8 *p) { e2p_WriteBytes(ADDR_BLENAME, p, 32); }
void e2p_GetNetParams(U8 *p) { e2p_ReadBytes(ADDR_LOCALIP, p, 12); }
void e2p_SetNetParams(U8 *p) { e2p_WriteBytes(ADDR_LOCALIP, p, 12); }

/**
 * @brief           设置格口状态
 * @param pSide     输出口 0/1
 * @param pRelay    继电器号，从0开始
 * @param v         格口状态: 0：Invalid, 1:Idle, 2:Busy, 3: Fault
 */
void e2p_SetBoxFlag(U8 pSide, int pRelay, U8 v)
{
    int e1, e2, data;
    u16 addr;
    pRelay *= 2;
    e1 = pRelay / 8;
    e2 = pRelay % 8;
    addr = ADDR_BOXFLAG + pSide * 0x40 + e1;
    e2p_ReadBytes(addr, (U8 *)&data, 1);
    data = setBit(data, e2, v, 2);
    e2p_WriteBytes(addr, (U8 *)&data, 1);
}
/**
 * @brief           获取格口状态
 * @param pSide     输出口 0/1
 * @param pRelay    继电器号，从0开始
 * @param p         格口状态: 0：未定义, 1:enable, 2:disable, 3: removed
 */
U8 e2p_GetBoxFlag(U8 pSide, int pRelay)
{
    int e1, e2, data;
    u16 addr;
    pRelay *= 2;
    e1 = pRelay / 8;
    e2 = pRelay % 8;
    addr = ADDR_BOXFLAG + pSide * 0x40 + e1;
    e2p_ReadBytes(addr, (U8 *)&data, 1);
    return getBit(data, e2, 2);
}
