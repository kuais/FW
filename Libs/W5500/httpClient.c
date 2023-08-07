/**
 * @brief
 * @author kuais (dlm321@126.com)
 * @version 1.0
 * @date 2021-03-08 09:34:56
 * @note
 * 流程:
 *      域名解析得到remoteIP->SocketClient连接remoteIP->发送Http报文(Head + Body)
 */

#include "httpClient.h"
#include "os.h"
#include "socket.h"
#include "w5500.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define HEADER_BUFSZ 4096
#define RESPONSE_BUFSZ 4096
#define DEFAULT_TIMEOUT 6

extern int fromHexes(char *c, int count);
extern int dns_gethostbyname(unsigned char *url, unsigned char *remoteip);

// struct in_addr
//{
//     uint32_t s_addr;
// };
// struct sockaddr_in
//{
//     uint8_t sin_len;
//     uint8_t sin_family;
//     uint16_t sin_port;
//     struct in_addr sin_addr;
//     char sin_zero[8];
// };
// struct sockaddr
//{
//     uint8_t sa_len;
//     uint8_t sa_family;
//     char sa_data[14];
// };
// struct addrinfo
//{
//     int ai_flags;             /* Input flags. */
//     int ai_family;            /* Address family of socket. */
//     int ai_socktype;          /* Socket type. */
//     int ai_protocol;          /* Protocol of socket. */
//     size_t ai_addrlen;        /* Length of socket address. */
//     struct sockaddr *ai_addr; /* Socket address of socket. */
//     char *ai_canonname;       /* Canonical name of service location. */
//     struct addrinfo *ai_next; /* Pointer to next in list. */
// };

static int _socket = 0;

/**
 * @brief 解析地址
 * @param session
 * @param url
 * @param method
 * @return int 0：成功， 其他： 失败
 */
int http_resolveAddress(http_session *session, const char *url, const char *method)
{
    int len_hostaddr, len_url, len_req;
    char *p_path, *p_port, *p_hostaddr = 0;
    char strTemp[256] = "";

    len_url = strlen(url);
    session->isHttps = false;
    if (strncmp(url, "http://", 7) == 0)
        p_hostaddr = (char *)(url + 7);
    else if (strncmp(url, "https://", 8) == 0)
    {
        session->isHttps = true;
        p_hostaddr = (char *)(url + 8);
    }
    else
        return -HTTP_ERROR;

    p_path = strstr(p_hostaddr, "/"); // 查找下一个"/"的位置
    if (p_path)
    {
        len_req = strlen(p_path); // 字符串结束符占1个字节
        strncpy(strTemp, p_path, len_req);
    }
    else
        strcpy(strTemp, "/");
    http_headerAdd(session, "%s %s HTTP/1.1\r\n", method, strTemp);

    p_port = strstr(p_hostaddr, ":");
    /* resolve port */
    if (p_port)
    {
        int len_port = 0;
        if (p_path)
            len_port = p_path - p_port - 1;
        else
            len_port = strlen(p_port + 1);
        session->port = (char *)http_malloc(len_port + 1);
        strncpy(session->port, p_port + 1, len_port);
        session->port[len_port] = '\0';
    }
    else
    {
        session->port = (char *)http_strdup(session->isHttps ? "443" : "80");
    }
    /* ipv4 or domain. */
    {
        if (p_port)
            len_hostaddr = p_port - p_hostaddr;
        else if (p_path)
            len_hostaddr = p_path - p_hostaddr;
        else
            len_hostaddr = strlen(p_hostaddr);

        if ((len_hostaddr < 1) || (len_hostaddr > len_url))
            return -HTTP_ERROR;
    }
    /* get host address ok. */
    {
        session->host = (char *)http_malloc(len_hostaddr + 1);
        strncpy(session->host, p_hostaddr, len_hostaddr);
        session->host[len_hostaddr] = '\0';
        http_headerAdd(session, "Host: %s:%s\r\n", session->host, session->port);
    }
    return HTTP_OK;
}

void http_initHeader(http_session *session)
{
    session->header.length = 0;
    memset(session->header.buffer, 0, session->header.size);
}
http_session *http_new(size_t headerSize)
{
    http_session *session = (http_session *)http_malloc(sizeof(http_session));
    //    _socket++;
    //    if (_socket == 1000)
    //        _socket = 0;
    _socket = 0;
    session->socket = _socket;
    session->serverip[0] = 0;
    session->host = NULL;
    session->port = NULL;
    session->isHttps = false;
    session->header.size = headerSize;
    session->header.buffer = (char *)http_malloc(headerSize * sizeof(char));
    session->body = NULL;
    session->errCount = 0;
    http_initHeader(session);
    return session;
}
void http_close(http_session *session)
{
    if (session == NULL)
        return;
    socket_close(session->socket);
    if (session->header.buffer)
    {
        http_free(session->header.buffer);
        session->header.buffer = NULL;
    }
    if (session->host)
    {
        http_free(session->host);
        session->host = NULL;
    }
    if (session->port)
    {
        http_free(session->port);
        session->port = NULL;
    }
    http_free(session);
    session = NULL;
}

/**
 * @brief           添加项到头部
 * @param session   当前对话
 * @param item      要添加的项
 * @param value     要添加项的值
 * @return int      >=0: 头部的长度， <0: error
 */
int http_headerAdd(http_session *session, const char *fmt, ...)
{
    int32_t length;
    va_list args;

    va_start(args, fmt);
    length = vsnprintf(session->header.buffer + session->header.length, session->header.size - session->header.length, fmt, args);
    va_end(args);
    if (length < 0)
        return -HTTP_ERROR;
    session->header.length += length;
    /* check header size */
    if (session->header.length >= session->header.size)
        return -HTTP_ERROR;
    return length;
}
int http_headerDel(http_session *session, char *item)
{
    int len = session->header.length;
    char *ptr = strstr(session->header.buffer, item);
    if (ptr != 0)
    {
        char *ptrEnd = strstr(ptr, "\r\n") + 2;
        strcpy(ptr, ptrEnd);
        len = len - (ptrEnd - ptr);
        session->header.length = len;
    }
    return len;
}

int http_getCode(char *pBuf)
{
    char code[4] = {0};
    char *p0 = NULL;
    p0 = strstr(pBuf, "HTTP/1.1 ");
    if (p0 == NULL)
        return 0;
    p0 += 9;
    memcpy(code, p0, 3);
    return atoi(code);
}

/**
 * @brief           获取指定项的对应值的起始位置
 * @param pBuf      源
 * @param item      指定项
 * @return char*    对应的值
 */
char *http_getField(char *pBuf, char *item)
{
    char *p0, *p1 = NULL;
    p0 = strstr(pBuf, item);
    if (p0 == NULL)
        return p1;
    p1 = strstr(p0, ":");
    p1++;
    while (*p1 && (*p1 == ' ' || *p1 == '\t'))
    { // 跳过空白字符
        p1++;
    }
    return p1;
}

int http_connect(http_session *session)
{
    U8 status = socket_status(session->socket);
    if (status != SOCK_ESTABLISHED)
    {
        https_printf("http_connect\n");
        local_port++;
        if (local_port == 60000)
            local_port = 5000;
        socket_new(session->socket, Sn_MR_TCP, local_port, 0);
        if (!socket_connect(session->socket, session->serverip, atoi(session->port)))
            return HTTP_CONNECTFAILED;
    }
    return HTTP_OK;
}
int http_disconnect(http_session *session)
{
    U8 status = socket_status(session->socket);
    if (status == SOCK_ESTABLISHED)
    {
        socket_close(session->socket);
        https_printf("http_disconnect\n");
    }
    return HTTP_OK;
}

char *http_prepareSend(http_session *session, int *len)
{
    int lenBody = 0;
    char *buf;
    *len = session->header.length + 2;
    if (session->body)
    {
        lenBody = strlen(session->body);
        *len += lenBody;
    }
    buf = (char *)mymalloc((*len) * sizeof(char));
    if (buf == NULL)
        return 0;
    memcpy(buf, session->header.buffer, session->header.length);
    buf[session->header.length] = 0x0D;
    buf[session->header.length + 1] = 0x0A;
    if (session->body)
        memcpy(buf + session->header.length + 2, session->body, lenBody);
    return buf;
}
/**
 * @brief           发送数据
 * @param session
 * @return int      0：失败， 其他： 发送数据的长度
 */
int http_send(http_session *session)
{
    int ret = 0;
    int len = 0;
    char *pBuf = http_prepareSend(session, &len);
    if (len > 0)
    {
        https_printf(pBuf);
        https_printf("\r\n");
        ret = socket_send(session->socket, pBuf, len);
        http_free(pBuf);
    }
    return ret;
}
/**
 * @brief           接收数据
 * @param session
 * @param content
 * @return int      接收的数据长度
 */
int http_receive(http_session *session, char *content)
{
    // char buffer[2048] = {0};
    char *pBuf;
    int ret, len0;
    int timeout = 3000;
    if (getSn_IR(session->socket) & Sn_IR_CON)
        setSn_IR(session->socket, Sn_IR_CON); /*清除接收中断标志*/
    ret = len0 = 0;

    while (timeout > 0)
    {
        http_delay(100);
        len0 = getSn_RX_RSR(session->socket);
        if (len0 == 0)
        { // 所有数据接收完
            if (ret > 0)
            {
                https_printf(content);
                https_printf("\r\n");
                pBuf = http_getField(content, "Content-Length:");
                ret = atoi(pBuf);
                pBuf = strstr(pBuf, "\r\n\r\n") + 4;
                memcpy(content, pBuf, ret);
                content[ret + 1] = 0;
                break;
            }
            timeout -= 100;
        }
        else
            ret += socket_recv(session->socket, (content + ret), len0);
    }
    return ret;
}

/**
 * @brief           send GET request to http server and get response header.
 * @param session   会话
 * @param output    输出数据缓冲区
 * @param lenOutput 输出数据缓冲区大小
 * @return int      <=0: failed, >0 收到的数据数
 */
int http_request(http_session *session, char *output)
{
    if (http_connect(session) != HTTP_OK)
        return -HTTP_ERROR;
    https_printf("http_sendStart!\n");
    if (http_send(session) <= 0)
        return -HTTP_ERROR;
    https_printf("http_receiveStart!\n");
    http_delay(100);
    return http_receive(session, output);
}
int http_requestNoRet(http_session *session)
{
    if (http_connect(session) != HTTP_OK)
        return -HTTP_ERROR;
    https_printf("http_sendStart!\n");
    return http_send(session);
}
