#ifndef __BSP_USB__
#define __BSP_USB__

#include "platform.h"

extern void usb_Init(void);
extern void usb_Config(void);
extern void usb_Clock_Config(void);
extern void usb_Interrupts_Config(void);
extern void usb_Cable_Config(FunctionalState NewState);
extern void usb_SerialNum(void);

#endif
