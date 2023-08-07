#ifndef __BSP_ADC__
#define __BSP_ADC__

#include "platform.h"

extern void adc_Config(void);
extern u16 adc_Value(u8 ch);
#endif
