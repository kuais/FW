#ifndef __MBEDTLSCLIENT_H
#define __MBEDTLSCLIENT_H

#include "httpClient.h"
#include "mbedtls/certs.h"
#include "mbedtls/config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"
#include "mbedtls/entropy.h"

typedef struct
{
    http_session *http;
    const unsigned char *ca;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
} mbedtls_session;

extern void https_printf(const char *fmt, ...);
// extern mbedtls_session *mbedtls_new(http_session *http, const char *ca);
extern mbedtls_session *mbedtls_new(const char *ca);
extern void mbedtls_close(mbedtls_session *session);
extern void mbedtls_reset(mbedtls_session *session);
extern int mbedtls_request(mbedtls_session *session, char *output, int maxlen);
extern int mbedtls_requestNoRet(mbedtls_session *session);
extern int mbedtls_setup(mbedtls_session *session);
extern int mbedtls_handshake(mbedtls_session *session);
extern int mbedtls_connect(mbedtls_session *session);
extern int mbedtls_send(mbedtls_session *session);
extern int mbedtls_receive(mbedtls_session *session, char *content, int maxlen);
#endif
