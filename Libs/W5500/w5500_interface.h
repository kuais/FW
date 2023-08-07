#ifndef _W5500_INTERFACE_
#define _W5500_INTERFACE_

#include <stdint.h>

extern void w5500_WriteByte(uint32_t addr, uint8_t data);
extern uint8_t w5500_ReadByte(uint32_t addr);
extern void w5500_WriteBytes(uint32_t addr, uint8_t *buf, uint16_t len);
extern uint16_t w5500_ReadBytes(uint32_t addr, uint8_t *buf, uint16_t len);

#endif
