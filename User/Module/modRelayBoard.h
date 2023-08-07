#ifndef __MODULE_RELAYBOARD__
#define __MODULE_RELAYBOARD__

#include "main.h"

#define RELAYCOUNT 240 // 单边最大继电器数
#define RELAYDATACOUNT (RELAYCOUNT * 2 + 7) / 8

typedef struct
{
    U8 errcount;
    U8 relayStatus[RELAYDATACOUNT];
} RelayBoard;

extern RelayBoard relayBoard[2];

extern void relayBoard_Init(void);

/**
 * @brief 接收处理
 * @param port 接收端口
 */
extern void relayBoard_Handle(UART_T *port);
/**
 * @brief 发送处理
 */
extern void relayBoard_Task(void);
/**
 * @brief 获取继电器状态
 * @param side IO口
 */
extern void relayBoard_Status(U8 side);
/**
 * @brief 开启继电器
 * @param side  IO口
 * @param relay 继电器号 0 - 255
 */
extern void relayBoard_Unlock(U8 side, U16 relay);
/**
 * @brief 获取指定继电器的锁状态
 * @param side IO口
 * @param relay 继电器号 0 - 255
 * @return U8  0:open， 1：close
 */
extern U8 relayBoard_GetLockStatus(U8 side, U16 relay);
/**
 * @brief 获取指定继电器的传感器状态
 * @param side IO口
 * @param relay 继电器号 0 - 255
 * @return U8  0:detect， 1：no-detect
 */
extern U8 relayBoard_GetSensorStatus(U8 side, U16 relay);

extern void relayBoard_GetAllLockStatus(U8 side, char *t);
extern void relayBoard_GetAllSensorStatus(U8 side, char *t);
extern U8 relayBoard_watiLockOpen(U8 side, U16 relay);

#endif
