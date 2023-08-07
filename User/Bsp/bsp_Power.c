#include "bsp_power.h"
#include "module/modBle.h"
#include "main.h"

extern volatile char curFlow;
extern void sysClk_Config(void);
extern void ble_Reset(void);
extern void exti_Switch(int flag);

extern volatile u8 tickCheck;

void power_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = POWER_BLE_PIN | POWER_ROUTER_PIN | POWER_KEYPAD_PIN3 | POWER_UART1_PIN | POWER_RELAYBOARD_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = POWER_BEEP_PIN | POWER_KEYPAD_PIN1 | POWER_KEYPAD_PIN2 | POWER_LED1_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	powerOff_Scanner;

	powerOn_BLE;
	powerOn_Route;
	appSet.router = 1;
	powerOn_RelayBoard;
	appSet.relayboard = 1;
	appSet.network = 0;
	appSet.scanner = 0;
}

void power_Normal(void)
{
	sysClk_Config(); // 停机后使用8M内部时钟，唤醒后需重新配置系统时钟
	exti_Switch(0);	 // 从外部中断过来的，不需要进入软中断处理

	// #ifdef FUNCTION_WATCHDOG
	//         iwdg_Start();
	// #endif
}
void power_Resume(void)
{
	tickCheck = 0;
	if (curFlow == AlarmWakeupFlow)
		return;
	powerOn_Scanner; // 还没进省电就取消
	powerOn_Led1;
	w5500_Init(); // powerOn_W5500;
	//    w5500_SetOPMD(1);
	//    w5500_LowPower(false);
	if (curFlow == PowerSaveFlow)
		curFlow = IdleFlow;
}
