#ifndef __KERN_API__
#define __KERN_API__

#include "kern_json.h"
#include "w5500/mbedtlsClient.h"

typedef struct
{
    char token[500];
    char step;
    char errCount;
} Kern_Session;

extern char *apiurl;
extern void kernapi_initHttps(void);
extern Kern_Session *kernapi_new(void);
extern void kernapi_close(Kern_Session *s);
extern int kernapi_token(Kern_Session *s, char *content, int len);
extern int kernapi_settings(Kern_Session *s, char *content, int len);
extern int kernapi_sync(Kern_Session *s, char *entity, char *content, int len);
extern int kernapi_upload(Kern_Session *s, char *entity, char *content, int len);

extern int kernapi_shipments(Kern_Session *s, char *content, int len);
extern int kernapi_commands(Kern_Session *s, char *text, char *content, int len);
extern int kernapi_recommand(Kern_Session *s, char *datas, char *content, int len);
extern int kernapi_result(Kern_Session *s);
extern int kernapi_setEvent(Kern_Session *s, char *text);
extern int kernapi_setVersion(Kern_Session *s, char *text);
extern int kernapi_setBoxesStatus(Kern_Session *s, char *text);
extern int kernapi_setDoorsStatus(Kern_Session *s, char *text);
extern int kernapi_setSensorsStatus(Kern_Session *s, char *text);
extern int kernapi_setShipment(Kern_Session *s, char *text);

extern void kernapi_test(void);

#endif
