#include "kern_api.h"
#include "main.h"
#include "os.h"
#include "w5500/dns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APIKEY "26111B43-A3CA-402D-A8E7-4E04EB2807AC"

char *apiurl;
char *apmid;
static char serverIP[4] = {0, 0, 0, 0};
static mbedtls_session *https = NULL;

static bool initHttps(http_session *http)
{
    if (https == NULL)
    {
        if (!dns_gethostbyname(http->host, serverIP))
            return false;
        https = mbedtls_new(CA_SWISS);
    }
    else if (http->isHttps)
    {
        mbedtls_reset(https);
    }
    https->http = http;
    memcpy(http->serverip, serverIP, 4);
    return true;
}
static int request(http_session *http, char *out, int lenOut)
{
    if (!initHttps(http))
        return 0;
    int ret = (http->isHttps) ? mbedtls_request(https, out, lenOut) : http_request(http, out);
    if (ret > 0)
    {
        char *p = strstr(out, "{");
        ret = strlen(p);
        memcpy(out, p, ret);
    }
    return ret;
}
static int requestNoRet(http_session *http)
{
    if (!initHttps(http))
        return 0;
    return (http->isHttps) ? mbedtls_requestNoRet(https) : http_requestNoRet(http);
}

static int kernapi_get(Kern_Session *s, char *url, char *datas, char *out, int lenOut)
{
    int ret = HTTP_OK;
    https_printf("GET url: %s\n Body: %s\n", url, datas ? datas : "");
    http_session *http = http_new(1024);
    ret = http_resolveAddress(http, url, HTTP_GET);
    if (ret == HTTP_OK)
    {
        http_headerAdd(http, "Authorization: Bearer %s\r\n", s->token);
        // http_headerAdd(http, "Connection: Keep-Alive\r\n");
        if (datas != 0)
        {
            http_headerAdd(http, "Content-Type:%s\r\n", "application/json");
            http_headerAdd(http, "Content-Length:%d\r\n", strlen(datas));
            http->body = datas;
        }
        // mbedtls_session *https = mbedtls_new(ca);
        ret = request(http, out, lenOut);
    }
    http_close(http);
    if (ret <= 0)
        s->errCount++;
    return ret;
}

static int kernapi_patch(Kern_Session *s, char *url, char *datas)
{
    int ret = HTTP_OK;
    https_printf("PATCH url: %s\n Body: %s\n", url, datas ? datas : "");
    http_session *http = http_new(1024);
    ret = http_resolveAddress(http, url, HTTP_PATCH);
    if (ret == HTTP_OK)
    {
        http_headerAdd(http, "Authorization: Bearer %s\r\n", s->token);
        // http_headerAdd(http, "Connection: Keep-Alive\r\n");
        if (datas != 0)
        {
            http_headerAdd(http, "Content-Type:%s\r\n", "application/json");
            http_headerAdd(http, "Content-Length:%d\r\n", strlen(datas));
            http->body = datas;
        }
        ret = requestNoRet(http);
    }
    http_close(http);
    if (ret <= 0)
        s->errCount++;
    return ret;
}
static int kernapi_post(Kern_Session *s, char *url, char *datas, char *out, int lenOut)
{
    int ret = 0;
    https_printf("POST url: %s\r\n Body: %s\r\n", url, datas ? datas : "");
    http_session *http = http_new(1024);
    ret = http_resolveAddress(http, url, HTTP_POST);
    if (ret == 0)
    {
        http_headerAdd(http, "Authorization: Bearer %s\r\n", s->token);
        // http_headerAdd(http, "Connection: Keep-Alive\r\n");
        if (datas != 0)
        {
            http_headerAdd(http, "Content-Type:%s\r\n", "application/json");
            http_headerAdd(http, "Content-Length:%d\r\n", strlen(datas));
            http->body = datas;
        }
        ret = request(http, out, lenOut);
    }
    http_close(http);
    if (ret <= 0)
        s->errCount++;
    return ret;
}
static int kernapi_postNoRet(Kern_Session *s, char *url, char *datas)
{
    int ret = HTTP_OK;
    https_printf("POST url: %s\r\n Body: %s\r\n", url, datas ? datas : "");
    http_session *http = http_new(1024);
    ret = http_resolveAddress(http, url, HTTP_POST);
    if (ret == HTTP_OK)
    {
        http_headerAdd(http, "Authorization: Bearer %s\r\n", s->token);
        // http_headerAdd(http, "Connection: Keep-Alive\r\n");
        if (datas != 0)
        {
            http_headerAdd(http, "Content-Type:%s\r\n", "application/json");
            http_headerAdd(http, "Content-Length:%d\r\n", strlen(datas));
            http->body = datas;
        }
        ret = requestNoRet(http);
    }
    http_close(http);
    if (ret <= 0)
        s->errCount++;
    return ret;
}
void kernapi_initHttps(void)
{
    mbedtls_close(https);
    https = NULL;
}
Kern_Session *kernapi_new(void)
{
    return (Kern_Session *)mymalloc(sizeof(Kern_Session));
}
void kernapi_close(Kern_Session *s)
{
    if (s != NULL)
        myfree(s);
}
int kernapi_token(Kern_Session *s, char *content, int len)
{
    char url[200];
    sprintf(url, "%s/token?apmid=%s", apiurl, apmid);
    return kernapi_post(s, url, NULL, content, len);
}
int kernapi_sync(Kern_Session *s, char *entity, char *content, int len)
{
    char url[200];
    sprintf(url, "%s/Sync?entity=%s&pagenumber=%d", apiurl, entity, curPage + 1);
    return kernapi_get(s, url, NULL, content, len);
}
int kernapi_upload(Kern_Session *s, char *entity, char *content, int len)
{
    char url[200];
    sprintf(url, "%s/Sync?entity=%s", apiurl, entity);
    return kernapi_post(s, url, content, content, len);
}
