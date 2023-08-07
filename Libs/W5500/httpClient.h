#ifndef __HTTPCLIENT_H
#define __HTTPCLIENT_H

#include "main.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_PATCH "PATCH"

#define HTTP_DEBUG 0

#if HTTP_DEBUG
#define http_print printf
#else
#define http_print
#endif

#ifndef http_malloc
#define http_malloc mymalloc
#endif

#ifndef http_calloc
#define http_calloc mycalloc
#endif

#ifndef http_realloc
#define http_realloc myrealloc
#endif

#ifndef http_free
#define http_free myfree
#endif

#ifndef http_strdup
#define http_strdup strdup
#endif

#ifndef http_delay
#define http_delay os_dly_wait
#endif

enum E_HTTP_STATUS
{
    HTTP_OK,
    HTTP_ERROR,
    HTTP_TIMEOUT,
    HTTP_NOMEM,
    HTTP_NOSOCKET,
    HTTP_NOBUFFER,
    HTTP_CONNECTFAILED,
    HTTP_DISCONNECT,
    HTTP_FILEERROR,
};

typedef struct
{
    size_t size;   /* maximum support header size */
    size_t length; /* content header buffer size */
    char *buffer;
} http_header;

typedef struct
{
    int socket;
    char *host; /* server host */
    char *port;
    char serverip[4];
    bool isHttps;       /* HTTPS connect */
    http_header header; /* webclient response header information */
    char *body;
    char errCount;
} http_session;

extern http_session *http_new(size_t headerSize);
extern void http_close(http_session *session);

extern int http_connect(http_session *session);
extern int http_disconnect(http_session *session);
extern int http_request(http_session *session, char *output);
extern int http_requestNoRet(http_session *session);
extern int http_resolveAddress(http_session *session, const char *url, const char *method);
extern int http_headerAdd(http_session *session, const char *fmt, ...);
extern int http_headerDel(http_session *session, char *item);
extern void http_initHeader(http_session *session);
extern char *http_prepareSend(http_session *session, int *buf);
extern int http_getCode(char *pBuf);
extern char *http_getField(char *pBuf, char *item);

#endif
