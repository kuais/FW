#ifndef __MODULE_WEB__
#define __MODULE_WEB__

#include "main.h"
#include "w5500/mbedtlsClient.h"

typedef struct
{
    int size;
    int chksum;
    char ip[4];
    char *url;
    char *filename;
} WebFile;

typedef struct
{
    int size;
    int chksum;
    char ip[4];
    char url[200];
    char version[10];
} FwInfo;

/**
 * @brief Web处理函数  (缓存数据, 偏移，数据数, 其他参数)
 *
 */
typedef void (*Web_Process)(char *, size_t);

extern char *web_GetFileName(char *url);
extern bool web_GetServerIP(char *url, char *ip, bool isDns);

extern bool web_Download(http_session *http, WebFile *wf, char *buf, int lenBuf, Web_Process process);
extern bool web_DownLoad_HTTPS(http_session *http, WebFile *wf, char *buf, int lenBuf, Web_Process proc);

extern bool web_UpdateFW(http_session *http, FwInfo *fw, char *buf);
extern bool web_UpdateFW_HTTPS(http_session *http, FwInfo *fw, char *buf, int lenBuf);

#endif
