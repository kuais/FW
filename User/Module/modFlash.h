#ifndef __MODULE_FLASH__
#define __MODULE_FLASH__

#include "stm32f10x_flash.h"

extern void flash_read(uint32_t address, uint8_t *buffer, uint32_t count);
extern void flash_write(uint32_t address, uint8_t *buffer, uint32_t count);
extern void flash_writeApp(uint32_t offset, uint8_t *buffer, uint16_t count);
extern void flash_setAppFlag(uint8_t value);
extern void flash_updateApp(uint8_t *buffer);

extern uint8_t flash_getAppFlag(void);
extern uint32_t flash_getAppAddress(void);
extern uint32_t flash_getOffsetAddress(void);
extern uint32_t flash_getUpdateBaseAddress(void);

#endif
