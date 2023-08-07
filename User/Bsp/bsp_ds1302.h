#ifndef __BSP_DS1302__
#define __BSP_DS1302__

#include "platform.h"

extern void ds1302_Config(void);
extern void ds1302_WriteBytes(U8 *writePoint);
extern void ds1302_ReadBytes(U8 *readPoint);

#endif
