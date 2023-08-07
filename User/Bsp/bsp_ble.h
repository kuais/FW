#ifndef __BSP_BLE__
#define __BSP_BLE__

#include "bsp_uart.h"
#include "platform.h"

#define ble_Send uart5_Send

enum
{
    BLE_STEPINIT,
    BLE_STEPTEST,
    BLE_STEPTESTRET,
	BLE_STEPMAC,
    BLE_STEPMACRET,
	BLE_STEPADV,
    BLE_STEPADVRET,
    BLE_STEPIDLE,
    BLE_STEPSTOP
};

extern volatile char bleStep;

extern void ble_Config(void);
extern void ble_Reset(void);

#endif
