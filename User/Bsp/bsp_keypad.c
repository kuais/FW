#include "bsp_keypad.h"

void keypad_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = KEYPAD_INPUT2;
    GPIO_Init(KEYPAD_PORT1, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = KEYPAD_INPUT1;
    GPIO_Init(KEYPAD_PORT2, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = KEYPAD_INPUT3 | KEYPAD_INPUT4;
    GPIO_Init(KEYPAD_PORT3, &GPIO_InitStructure);
}

static U8 keypad_Input(U8 index)
{
    switch (index)
    {
    case 2:
        return GPIO_ReadInputDataBit(KEYPAD_PORT1, KEYPAD_INPUT2);
    case 3:
        return GPIO_ReadInputDataBit(KEYPAD_PORT3, KEYPAD_INPUT3);
    case 4:
        return GPIO_ReadInputDataBit(KEYPAD_PORT3, KEYPAD_INPUT4);
    default:
        return GPIO_ReadInputDataBit(KEYPAD_PORT2, KEYPAD_INPUT1);
    }
}

U8 keypad_getInput(U8 index)
{
    int i, j;
    i = j = 0;
    while (i++ < 4)
    {
        if (index == 2)
        { // 特殊处理第1列
            if (keypad_Input(i) == 0)
            {
                if (i == 1)
                {
                    if (keypad_Input(2) == 0)
                        i = 2;
                    if (keypad_Input(3) == 0)
                        i = 3;
                }
                break;
            }
        }
        else
        {
            if (keypad_Input(i) == 0)
                break;
        }
    }
    if (i > 4)
        return 0;				// 未检测到按键
    /* 防抖动*/
    while (j++ < 5)
    {
        os_dly_wait(10);
        if (keypad_Input(i) != 0)
            return 0;
    }
    return 3 * i - index;
}
