
#ifndef __MODULE_BLE__
#define __MODULE_BLE__

#include "main.h"

extern void ble_Init(void);
extern void ble_Start(void);
extern void ble_SendStatus(u8 flag, u8 *datas, int len);
/**
 * @brief 接收处理
 * @param port 接收端口
 */
extern void ble_Handle(UART_T *port);

/**
 * @brief 发送任务
 */
extern void ble_Task(void);
#endif
