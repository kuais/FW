#include "bsp_ble.h"
#include "bsp_power.h"

volatile char bleStep = BLE_STEPSTOP;

void ble_Reset(void)
{
    if (bleStep == BLE_STEPSTOP)
        return;
    powerOff_BLE;
    os_dly_wait(200);
    powerOn_BLE;
    printf("BLE RESET");
}
void ble_Config(void)
{
    PWR_BackupAccessCmd(ENABLE); /* 允许修改RTC和后备寄存器*/
    RCC_LSEConfig(RCC_LSE_OFF);  /* 关闭外部低速时钟,PC14+PC15可以用作普通IO*/
    BKP_TamperPinCmd(DISABLE);   /* 关闭入侵检测功能,PC13可以用作普通IO*/
    // GPIO_InitTypeDef GPIO_InitStructure;
    // /* RESET */
    // GPIO_InitStructure.GPIO_Pin = BLE_PINRESET;
    // GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    // GPIO_Init(BLE_PORT, &GPIO_InitStructure);
}
