
#ifndef __BSP_KEYPAD__
#define __BSP_KEYPAD__

#include "platform.h"

extern void keypad_Config(void);
// extern void keypad_Power(U8 index, u8 flag);
extern U8 keypad_getInput(U8 index);
#endif
