/**
 * @brief e2p模块
 * @author kuais (dlm321@126.com)
 * @version 1.0
 * @date 2021-07-13 08:16:42
 *
 */

#ifndef __MODULE_E2P__
#define __MODULE_E2P__

// #include "Kern\kern_json.h"
#include "bsp_e2p.h"

#define ADDR_LIMIT 0x8000 // AT24C256 256KB 有效地址空间为 0000-7FFF

/* 运行标志 */
#define ADDR_INITFLAG 0x0000 // 0：系统已初始化，1：系统未初始化

/* 设备参数 */
#define ADDR_APMID 0X0010   // APM-ID        10字节 0x0010-0x0019
#define ADDR_FLOOR 0X001F   // 锁板层数       1字节 0x1F
#define ADDR_BLEID 0X0020   // 蓝牙ID         10字节 0x0020-0x0029
#define ADDR_BLEFUNC 0x002A // 蓝牙功能开关    1字节 0x2A
#define ADDR_APIURL 0x0030  // API地址        96字节  0x0030-0x008F
#define ADDR_ALARM 0x0092   // 闹钟时间 6字节   0x0092-0097

/* 网络参数 */
#define ADDR_LOCALIP 0x0100    // 本地地址     4字节
#define ADDR_SUBMSK 0x0104     // 子网掩码     4字节
#define ADDR_GATEWAY 0x0108    // 网关         4字节
#define ADDR_REMOTEIP 0x0110   // 远端地址     4字节
#define ADDR_REMOTEPORT 0x0114 // 远端端口     4字节
#define ADDR_MACADDR 0x0120    // MAC地址      6字节 0120-0125

/* Others */
#define ADDR_AESKEY 0x0130  // 加密密钥 16字节   0130-013F
#define ADDR_BLENAME 0x0140 // 蓝牙名称 32字节   0140-015F

/****** 业务数据 ******/
/* 格口有效标志 0700-077F
 * 1格口2bit.总共需要 256 * 2 * 2 = 1024bit = 128Byte
 * 格口状态 2bit: 0：未定义, 1:Enabled, 2:Disabled, 3: Removed
 */
#define ADDR_BOXFLAG 0x0700    // out1: 0700-073F, out2: 0740-077F
#define ADDR_BOXFLAGEND 0x0780 //

/* 函数 */
extern void e2p_Init(void);
extern void e2p_Clear(void);

extern void e2p_GetFlag_Init(U8 *p);
extern void e2p_SetFlag_Init(U8 v);
extern void e2p_GetApmID(U8 *p);
extern void e2p_SetApmID(U8 *p);
extern void e2p_GetFloorCount(U8 *p);
extern void e2p_SetFloorCount(U8 v);
extern void e2p_GetBleID(U8 *p);
extern void e2p_SetBleID(U8 *p);
extern void e2p_GetBleFunc(U8 *p);
extern void e2p_SetBleFunc(U8 v);
extern void e2p_GetApiUrl(U8 *p);
extern void e2p_SetApiUrl(U8 *p);
extern void e2p_GetAesKey(U8 *p);
extern void e2p_SetAesKey(U8 *p);
extern void e2p_GetBleName(U8 *p);
extern void e2p_SetBleName(U8 *p);

extern void e2p_GetNetParams(U8 *p);
extern void e2p_SetNetParams(U8 *p);

extern void e2p_SetBoxFlag(U8 pSide, int pRelay, U8 v);
extern U8 e2p_GetBoxFlag(U8 pSide, int pRelay);

#endif
