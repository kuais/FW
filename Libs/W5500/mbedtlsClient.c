#include "mbedtlsClient.h"
#include "mbedtls/error.h"
#include "mbedtls/ssl.h"
#include "socket.h"
#include "utility.h"
#include <RTL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void _ssl_debug(void *ctx, int level, const char *file, int line, const char *str) { mbedtls_printf("%s:%04d: %s", file, line, str); }

mbedtls_session *mbedtls_new(const char *ca)
{
    mbedtls_session *session = (mbedtls_session *)http_malloc(sizeof(mbedtls_session));
    session->ca = ca;
    mbedtls_ssl_init(&session->ssl);
    mbedtls_ssl_config_init(&session->conf);
    mbedtls_x509_crt_init(&session->cacert);
    mbedtls_ctr_drbg_init(&session->ctr_drbg);
    mbedtls_entropy_init(&session->entropy);

    if (mbedtls_setup(session) != HTTP_OK)
    {
        mbedtls_close(session);
        session = NULL;
    }
    return session;
}

void mbedtls_close(mbedtls_session *session)
{
    if (session == NULL)
        return;
    session->http = NULL;
    session->ca = NULL;
    mbedtls_ssl_free(&session->ssl);
    mbedtls_ssl_config_free(&session->conf);
    mbedtls_x509_crt_free(&session->cacert);
    mbedtls_ctr_drbg_free(&session->ctr_drbg);
    mbedtls_entropy_free(&session->entropy);
    http_free(session);
}
void mbedtls_reset(mbedtls_session *session) { mbedtls_ssl_session_reset(&(session->ssl)); }

int mbedtls_handshake(mbedtls_session *session)
{
    int ret = HTTP_OK;
    https_printf("https_handshake\n");
    while ((ret = mbedtls_ssl_handshake(&session->ssl)) != HTTP_OK)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            https_printf("https_handshake failed!\n");
            break;
        }
    }
    return ret;
}
int mbedtls_setup(mbedtls_session *session)
{
    int ret = HTTP_OK;
    const char *pers = "ssl_client1";
    https_printf("https_setup\n");
    if (ret = mbedtls_ctr_drbg_seed(&session->ctr_drbg, mbedtls_entropy_func, &session->entropy, (const unsigned char *)pers, strlen(pers)) != HTTP_OK)
    {
        https_printf("ctr_drbg_seed failed!\n");
    }
    else if (ret = mbedtls_x509_crt_parse(&session->cacert, (const unsigned char *)session->ca, strlen(session->ca) + 1) != HTTP_OK)
    {
        https_printf("x509_crt_parse failed!\n");
    }
    else if ((ret = mbedtls_ssl_config_defaults(&session->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != HTTP_OK)
    {
        https_printf("ssl_config_defaults failed!\n");
    }
    else
    {
#if defined(MBEDTLS_SSL_SESSION_TICKETS)
        mbedtls_ssl_conf_session_tickets(&session->conf, MBEDTLS_SSL_SESSION_TICKETS_ENABLED);
#endif
        mbedtls_ssl_conf_authmode(&session->conf, MBEDTLS_SSL_VERIFY_NONE);
        mbedtls_ssl_conf_ca_chain(&session->conf, &session->cacert, NULL);
        mbedtls_ssl_conf_rng(&session->conf, mbedtls_ctr_drbg_random, &session->ctr_drbg);
        mbedtls_ssl_conf_dbg(&session->conf, _ssl_debug, NULL);
#ifdef MBEDTLS_DEBUG_C
        mbedtls_debug_set_threshold(3);
#endif
        if ((ret = mbedtls_ssl_setup(&session->ssl, &session->conf)) != HTTP_OK)
            https_printf("ssl_setup failed!\n");
    }
    return ret;
}

// bool ishandshake = false;
/**
 * @brief 建立https连接
 * @param session
 * @return int 0：成功， 其他失败
 */
int mbedtls_connect(mbedtls_session *session)
{
    int ret = HTTP_OK;
    if (ret = http_connect(session->http) != HTTP_OK)
        goto exit;
    if ((ret = mbedtls_ssl_set_hostname(&session->ssl, (const char *)&session->http->host)) != HTTP_OK)
    {
        https_printf("ssl_set_hostname failed!\n");
        goto exit;
    }
    mbedtls_ssl_set_bio(&session->ssl, (void *)session->http->socket, socket_send, socket_recv, NULL);
    // if (!ishandshake)
    if ((ret = mbedtls_handshake(session)) != HTTP_OK)
        goto exit;
    // ishandshake = true;
//    os_dly_wait(1);
//    if ((ret = mbedtls_ssl_get_verify_result(&session->ssl)) != HTTP_OK)
//    {
//        char vrfy_buf[512];
//        https_printf("x509_crt_verify_info failed\n");
//        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", ret);
//        https_printf("%s\n", vrfy_buf);
//        goto exit;
//    }
exit:
    return ret;
}

// int mbedtls_connect(mbedtls_session *session)
// {
//     int ret = HTTP_OK;
//     https_printf("https_connect\n");
//     if (ret = http_connect(session->http) != HTTP_OK)
//         goto exit;
//     if (ret = mbedtls_setup(session) != HTTP_OK)
//         goto exit;
//     ret = mbedtls_handshake(session);
// //    os_dly_wait(1);
// //    if ((ret = mbedtls_ssl_get_verify_result(&session->ssl)) != HTTP_OK)
// //    {
// //        char vrfy_buf[512];
// //        https_printf("x509_crt_verify_info failed\n");
// //        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", ret);
// //        https_printf("%s\n", vrfy_buf);
// //        goto exit;
// //    }
// exit:
//     return ret;
// }

/**
 * @brief https发送数据
 * @param session
 * @return int 写入的数据长度
 */
int mbedtls_send(mbedtls_session *session)
{
    int ret = 0;
    int len = 0;
    char *pBuf = http_prepareSend(session->http, &len); // 从http_session组装出实际发送的数据
    if (len == 0)
        ret = -HTTP_ERROR;
    else
    {
        while ((ret = mbedtls_ssl_write(&session->ssl, pBuf, len)) <= 0)
        {
            // os_dly_wait(1);
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                https_printf("ssl_write returned -0x%x\n", -ret);
                ret = -HTTP_ERROR;
                break;
            }
        }
        http_free(pBuf);
    }
    //	https_printf("https_sent %X!\n", ret);
    return ret;
}

/**
 * @brief               Https接收数据
 * @param session
 * @param content
 * @param maxlen
 * @return int          接收的数据长度,  <0  非200的 Http Code
 */
int mbedtls_receive(mbedtls_session *session, char *content, int maxlen)
{
    char *pBuf;
    int ret = 0;
    while (1)
    {
        http_delay(100);
        ret = mbedtls_ssl_read(&session->ssl, (content + ret), maxlen);
        if (ret > 0)
        {
            https_printf("%s\r\n", content);
            int code = http_getCode(content);
            if (code != 200 && code != 206)
                return -code;
            char *p = http_getField(content, "Content-Length:");
            if (p != NULL)
            {
                pBuf = strstr(p, "\r\n\r\n") + 4;
                ret = atoi(p);
            }
            else
            {
                pBuf = strstr(content, "\r\n\r\n") + 4;
                ret = strlen(pBuf);
            }
            memcpy(content, pBuf, ret);
            content[ret + 1] = 0;
            break;
        }
        else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            https_printf("ssl_read returned -0x%x\n", -ret);
            break;
        }
    }
    return ret;
}

/**
 * @brief           1次https请求
 * @param session   会话
 * @param output    输出数据缓冲区
 * @param lenOutput 输出数据缓冲区大小
 * @return int      <=0: failed, >0 收到的数据数
 */
int mbedtls_request(mbedtls_session *session, char *output, int maxlen)
{
    if (mbedtls_connect(session) != HTTP_OK)
        return -HTTP_CONNECTFAILED;
    // os_dly_wait(1);
    https_printf("https_sendStart!\n");
    if (mbedtls_send(session) <= 0)
        return -HTTP_ERROR;
    // os_dly_wait(1);
    https_printf("https_receiveStart!\n");
    http_delay(10);
    return mbedtls_receive(session, output, maxlen);
}

/**
 * @brief 1次https请求, 只发不收
 * @param session
 * @return int >0:成功 其他:失败
 */
int mbedtls_requestNoRet(mbedtls_session *session)
{
    if (mbedtls_connect(session) != HTTP_OK)
        return -HTTP_CONNECTFAILED;
    https_printf("https_sendStart!\n");
    return mbedtls_send(session);
}
