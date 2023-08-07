#ifndef __MODULE_DB__
#define __MODULE_DB__

#include "main.h"
/* Sync download */
extern int db_saveConfFromKern(char *input);
extern int db_saveBoxFromKern(char *input);
extern int db_saveBoxPluFromKern(char *input);
extern int db_savePluFromKern(char *input);
extern int db_saveCourierFromKern(char *input);
extern int db_saveLabelPluFromKern(char *input);
extern int db_saveLabelRuleFromKern(char *input);
extern int db_saveTechnicianFromKern(char *input);
extern int db_savePreReservationFromKern(char *input);
extern int db_setReservationFromKern(char *input);
/* Sync upload */
extern int db_getEventPost(Event *ev);
extern int db_getIncidentPost(Incident *p);
extern int db_getTechnicianActionPost(TechnicianAction *p);

extern void db_loadBoxList(void);
extern void db_loadBoxFlagList(void);
extern void db_saveBoxFlagList(void);
extern void db_loadBoxFlag(char *data, u16 pos);
extern void db_saveBoxFlag(char *data, u16 pos);
extern bool db_addShipment(Shipment *shipment);
extern void db_delShipment(Shipment *shipment);
extern void db_delShipmentByPos(int pos);
extern void db_setShipment(Shipment *shipment);
extern bool db_addEvent(Event *event);
extern bool db_delEvent(Event *event);
extern bool db_delEventByPos(DWORD pos);
extern void db_delPreReservation(char *barcode);
extern bool db_newTechnicianAction(char type, char *user);
extern bool db_addTechnicianAction(TechnicianAction *p);
extern bool db_delTechnicianActionByPos(DWORD pos);
extern bool db_newIncident(char type, u16 boxid, char *info);
extern bool db_addIncident(Incident *p);
extern bool db_delIncidentByPos(DWORD pos);
extern bool db_addResShipment(ResShipment *p);
extern bool db_delResShipmentByTime(u32 time);
extern bool db_delResShipmentByPos(DWORD pos);
extern DWORD db_getResShipment(ResShipment *p, char type, char *text);

extern char db_checkTechnician(char *id);
extern char db_getPluOfCourier(char *id);

extern char db_getFlagOfPlu(char *plu);
extern int db_findShipmentByBoxID(u16 boxid);
extern int db_findShipmentLast(Shipment *sp);
extern int db_getShipmentByBarcode(char *plu, char *barcode, Shipment *sp);
extern Shipment *db_getShipmentByPin(char *pin);
extern Shipment *db_getShipmentReturn(char *plu, DWORD *offset);

extern LabelPlu *db_getRuleOfPlu(char *plu, char type, DWORD *offset);
extern LabelRule *db_getLabelRule(char *rulename);
extern int getBoxUsedCountOfPlu(char *plu, char size);
extern int getBoxReservedCount(char *plu, char size);
extern u16 getBoxIdToDeposit(char *plu, char size);
extern char checkPickupPin(char *id);
extern char checkPreReservationCourier(char *barcode);
extern char checkPreReservationCustomer(char *barcode);

extern void db_init(void);
extern void db_print(void);

#endif
