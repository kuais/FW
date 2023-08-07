#ifndef __MODULE_KERN__
#define __MODULE_KERN__

#include "Kern/kern_api.h"

extern Kern_Session *kernSession;
extern bool flagVersion; // true:版本号已上传

extern void kern_ClearSession(void);
extern void kern_NewSession(void);
extern void kern_nextStep(Kern_Session *s);
extern void kern_HandleToken(Kern_Session *s, char *text);
extern void kern_HandleConf(Kern_Session *s, char *text);
extern void kern_HandleConfBox(Kern_Session *s, char *text);
extern void kern_HandleBoxPLU(Kern_Session *s, char *text);
extern void kern_HandleBoxDisaster(Kern_Session *s, char *text);
extern void kern_HandleCourier(Kern_Session *s, char *text);
extern void kern_HandleCourierPLU(Kern_Session *s, char *text);
extern void kern_HandlePLU(Kern_Session *s, char *text);
extern void kern_HandleTechnician(Kern_Session *s, char *text);
extern void kern_HandleLabelPLU(Kern_Session *s, char *text);
extern void kern_HandleLabelRule(Kern_Session *s, char *text);
extern void kern_HandlePreReservation(Kern_Session *s, char *text);
extern void kern_HandleReservation(Kern_Session *s, char *text);

extern void kern_DataOfVersion(char *v, char *buf);
extern void kern_DataOfBatteryLevel(char *v, char *buf);
extern void kern_DataOfLocksStatus(char *buf);
extern void kern_DataOfSensorsStatus(char *buf);
extern void kern_DataofUploadReservation(Event *ev, char *buf);
extern void kern_DataOfIncident(Incident *p, char *buf);
extern void kern_DataOfTechnicianAction(TechnicianAction *p, char *buf);

#endif
