#include "bsp_button.h"

uint8_t btnDfu_Read(void) { return GPIO_ReadInputDataBit(BUTTON_PORT, BUTTON_PIN); }

void btnDfu_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
    GPIO_Init(BUTTON_PORT, &GPIO_InitStructure);
}

void btn_Config(void) 
{ 
    btnDfu_Config(); 
}
