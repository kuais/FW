#ifndef __KERN_JSON__
#define __KERN_JSON__

#include "cjson/cJSON.h"
#include <stdbool.h>

typedef struct
{
    char *data;
    char *error;
    int pagesize;
} Kern_Result;
typedef struct
{
    char *set_time;
    short seconds_time_limit_close_door;
} Kern_Setting;
typedef struct
{
    char *id;
    char *state;
    char *type;
    char *barcode;
    char *pin;
    bool no_sensor;
    char *box_id;
} Kern_Shipment;
typedef struct
{
    int box_id;
    char size;
    //    char *type;
    char side;
    char relay;
} Kern_Box;
typedef struct
{
    char *id;
    char *apm_id;
    char *shipment_id;
    char *type;
    char *box_id;
    char *command_id;
    char *execution_time;
    char *sensor_status;
    char *door_status;
} Kern_Event;
typedef struct
{
    char *id;
    char *apm_id;
    char *type;
    char *box_id;
    char *shipment_id;
    char *shipment_state;
    char *shipment_info;
    char *door_state;
    char *execution_time;
    char *result;
    unsigned char side;
    unsigned char relay;
} Kern_Command;

extern char *trimEnter(char *c);
extern char *Kern_GetToken(cJSON *jobj);
extern bool Kern_ParseResult(cJSON *jobj, Kern_Result *result);
extern void Kern_ParseSetting(cJSON *jobj, Kern_Setting *setting);
extern void Kern_ParseShipment(cJSON *jobj, Kern_Shipment *cmd);
extern void Kern_ParseBox(cJSON *jobj, Kern_Box *box);
extern void Kern_ParseCommand(cJSON *jobj, Kern_Command *cmd);
extern void Kern_DeparseEvent(cJSON *jobj, Kern_Event *ev);
#endif
