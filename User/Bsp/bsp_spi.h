#ifndef __BSP_SPI__
#define __BSP_SPI__

#include "RTE_Device.h"
#include "platform.h"

#if RTE_SPI1
extern void spi1_Config(void);
extern void spi1_CsOn(void);
extern void spi1_CsOff(void);
extern u8 spi1_Write(u8 data);
extern u8 spi1_Read(void);
#endif // RTE_SPI1

#if RTE_SPI2
extern void spi2_Config(void);
extern void spi2_CsOn(void);
extern void spi2_CsOff(void);
extern u8 spi2_Write(u8 data);
extern u8 spi2_Read(void);
#endif // RTE_SPI2

#if RTE_SPI3

#endif // RTE_SPI3

#endif
