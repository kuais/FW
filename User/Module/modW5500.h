#ifndef __MODULE_W5500__
#define __MODULE_W5500__

#include "main.h"

#define SOCKET_BUFSIZETX 2 /* Socket发送区大小 单位:KBytes */
#define SOCKET_BUFSIZERX 8 /* Socket接收区大小 单位:KBytes */

typedef enum
{
    IP_FROM_DEFINE,
    IP_FROM_DHCP,
    IP_FROM_E2P
} E_Ipfrom;

extern uint16_t local_port;

extern void w5500_SetOPMD(char);
extern bool w5500_CheckConnect(void);
extern void w5500_LowPower(bool v);
extern void w5500_Init(void);
extern void w5500_CheckUpdate(void);
extern void w5500_SyncDown(void);

extern bool w5500_GetToken(void);
extern void w5500_UploadLockStatus(void);
extern void w5500_UploadSensorStatus(void);
extern void w5500_UploadReservation(void);
extern void w5500_UploadIncident(void);
extern void w5500_UploadTechnicianAction(void);

extern void w5500_SyncConf(void);
extern void w5500_GetPrereservation(void);
extern void w5500_GetReservation(void);
extern void w5500_Handle(void);

#endif
