
#ifndef __BSP_POWER__
#define __BSP_POWER__

#include "platform.h"

#define powerOn_BLE GPIO_SetBits(GPIOB, POWER_BLE_PIN)
#define powerOff_BLE GPIO_ResetBits(GPIOB, POWER_BLE_PIN)

#define powerOn_Route GPIO_SetBits(GPIOB, POWER_ROUTER_PIN)
#define powerOff_Route GPIO_ResetBits(GPIOB, POWER_ROUTER_PIN)

#define powerOn_Scanner GPIO_SetBits(GPIOB, POWER_UART1_PIN)
#define powerOff_Scanner GPIO_ResetBits(GPIOB, POWER_UART1_PIN)

#define powerOn_RelayBoard GPIO_SetBits(GPIOB, POWER_RELAYBOARD_PIN)
#define powerOff_RelayBoard GPIO_ResetBits(GPIOB, POWER_RELAYBOARD_PIN)

#define powerOn_Beep GPIO_SetBits(GPIOC, POWER_BEEP_PIN)
#define powerOff_Beep GPIO_ResetBits(GPIOC, POWER_BEEP_PIN)

#define powerOn_Led1 GPIO_SetBits(GPIOC, POWER_LED1_PIN)
#define powerOff_Led1 GPIO_ResetBits(GPIOC, POWER_LED1_PIN)

#define powerOn_Key1 GPIO_SetBits(GPIOC, POWER_KEYPAD_PIN1)
#define powerOff_Key1 GPIO_ResetBits(GPIOC, POWER_KEYPAD_PIN1)
#define powerOn_Key2 GPIO_SetBits(GPIOC, POWER_KEYPAD_PIN2)
#define powerOff_Key2 GPIO_ResetBits(GPIOC, POWER_KEYPAD_PIN2)
#define powerOn_Key3 GPIO_SetBits(GPIOB, POWER_KEYPAD_PIN3)
#define powerOff_Key3 GPIO_ResetBits(GPIOB, POWER_KEYPAD_PIN3)

#define powerOn_W5500 GPIO_SetBits(GPIOC, W5500_PINRST)
#define powerOff_W5500 GPIO_ResetBits(GPIOC, W5500_PINRST)

extern void power_Config(void);
extern void power_Normal(void);
extern void power_Resume(void);
#endif
