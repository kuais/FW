#ifndef __OS__
#define __OS__

#include "RTL.h"
#define OS_FOREVER 0xFFFF
#define os_delay(x) os_dly_wait(x)

extern OS_SEM semaphore_uart1;
extern OS_SEM semaphore_uart2;
extern OS_SEM semaphore_uart3;
extern OS_SEM semaphore_uart4;
extern OS_SEM semaphore_uart5;

extern OS_MUT mutex_e2p;

#endif
