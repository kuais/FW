#ifndef __BSP_E2P__
#define __BSP_E2P__

#include "platform.h"

extern void e2p_Config(void);
extern U8 e2p_ReadByte(U16 address);
extern void e2p_WriteByte(U16 address, U8 data);
extern void e2p_ReadBytes(U16 address, U8 *pData, U16 byteCount);
extern void e2p_WriteBytes(U16 address, U8 *pData, U16 byteCount);

#endif
