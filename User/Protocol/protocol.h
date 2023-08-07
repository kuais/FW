#ifndef __PROTOCOL1_H
#define __PROTOCOL1_H

#include <stdint.h>
#include "KuFrame/kubuffer.h"

/******* Protocol BLE *******/
extern KuBuffer *pBle_Handle(uint8_t *bufRecv);
extern KuBuffer *pBle_dataOfStatus(unsigned char flag, unsigned char *datas, int len);

#endif
