#ifndef __FLOW__
#define __FLOW__

#include "main.h"

extern char setBarcodeRules(char *plu, int type);

extern void flow_AlarmWakeup(void);
extern void flow_AppSwitch(void);
extern void flow_ClearData(void);
extern void flow_PowerSave(void);
extern void flow_SyncAll(void);
extern void flow_Idle(void);
extern void flow_Exit(void);

extern void flow_Login(void);
extern void flow_CourierPickup(void);
extern void flow_CourierDropoff(void);
extern void flow_Dropoff(void);
extern void flow_CustomerPickup(void);
extern void flow_CustomerDropoff(void);
extern void flow_SuperTechnician(void);
extern void flow_TechnicianFlow(void);

extern void flow_BleOpenLock(void);
extern void flow_BleConfirm(void);

#endif
