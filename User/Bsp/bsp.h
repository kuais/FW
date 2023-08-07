/***
 ** SPI2:   W5500
 ** UART2:  PC
 ** UART3:  Output2
 ** UART4:  Output1
 **
 ***/

#ifndef __BSP__
#define __BSP__

#include "bsp_adc.h"
#include "bsp_ble.h"
#include "bsp_e2p.h"
#include "bsp_exti.h"
#include "bsp_keypad.h"
#include "bsp_power.h"
#include "bsp_rtc.h"
#include "bsp_spi.h"
#include "bsp_spiflash.h"
#include "bsp_uart.h"
#include "bsp_w5500.h"

extern void __svc(1) SVC_1_FunCall(void);
extern void __svc(2) SVC_2_FunCall(void);

extern void bsp_Beep(int ms);
extern void bsp_BeepSome(int count, int ms);
extern void bsp_Config(void);
extern void bsp_delay(int n);
// extern void bsp_Reboot(void);
extern void bsp_SystemReset(void);
extern void bsp_Restart(void);
extern u16 bsp_Voltage(void);
extern void sysClk_Config(void);
extern void iwdg_Feed(void);
extern void iwdg_Start(void);

#endif
