/**
 * @file modScanner.h
 * @author your name (you@domain.com)
 * @brief 霍尼韦尔扫描仪控制模块
 * @version 0.1
 * @date 2022-08-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __MODULE_SCANNNER__
#define __MODULE_SCANNER__

#include "main.h"

extern void scanner_Blink(char count, u16 interval);
extern void scanner_Handle(UART_T *port);
extern void scanner_Activate(void);
extern void scanner_Deactivate(void);
extern BOOL scanner_PhoneMode(char flag);
extern BOOL scanner_AllRuleEnable(char flag);
extern BOOL scanner_AimMode(char flag);
extern BOOL scanner_SwitchLed(char flag);
extern BOOL scanner_SwitchLight(char flag);
extern BOOL scanner_RuleEnable(LabelRule *rule);

#endif
