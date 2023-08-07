#ifndef __BSP_SPIFLASH__
#define __BSP_SPIFLASH__

#include "platform.h"

extern void sf_Config(void);
extern void sf_EraseSector(u32 addr);
extern void sf_EraseChip(void);
extern u16 sf_ReadBytes(u32 addr, u8 *buf, u16 len);
extern u16 sf_WriteBytes(u32 addr, u8 *buf, u16 len);
extern u16 sf_WritePage(u32 addr, u8 *buf, u16 len);

#endif
