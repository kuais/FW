#include "modWeb.h"
#include "modFlash.h"
#include "KuFrame/kuutil.h"

extern int dns_gethostbyname(unsigned char *url, unsigned char *remoteip);

char *web_GetFileName(char *url)
{
    return strrchr(url, '/') + 1;
}
bool web_GetServerIP(char *url, char *ip, bool isDns)
{
    if (strlen(ip))
        return true; // ip已经获取过
    if (isDns)
        return dns_gethostbyname(url, ip);
    else
        return sscanf(url, "%d.%d.%d.%d", ip, ip + 1, ip + 2, ip + 3) == 4;
}

/**
 * @brief 下载文件并存到FLash
 * @return true
 * @return false
 */
bool web_Download(http_session *http, WebFile *wf, char *buf, int lenBuf, Web_Process proc)
{
    int pos, chksum, lenBlock;
    pos = chksum = 0;
    lenBlock = 2048;
    while (1)
    {
		tickCheck = 0;
        http_headerDel(http, "Range");
        if (pos + lenBlock >= wf->size)
            lenBlock = wf->size - pos;
        http_headerAdd(http, "Range: bytes=%d-%d\r\n", pos, pos + lenBlock - 1);
        if (http_request(http, buf) != lenBlock)
            return 0;
        /* 计算累加和 */
        chksum += getSum(buf, 0, lenBlock);
        chksum &= 0xFF;
        /* 写入文件 */
        proc(buf, lenBlock);
        /* 传输下一帧 */
        pos += lenBlock;
		printf("Process %d / %d", pos, wf->size);
        if (pos == wf->size)
            break; // 传输完毕
        os_dly_wait(1);
    }
    return chksum == wf->chksum; // 返回校验结果
}

bool web_DownLoad_HTTPS(http_session *http, WebFile *wf, char *buf, int lenBuf, Web_Process proc)
{
    int pos, chksum, lenBlock;
    pos = chksum = 0;
    lenBlock = 2048; // F103 Flash 单页为2048字节
    int errCount = 0;
    mbedtls_session *https = mbedtls_new(CA_SWISS); // CA_PUDOUPDATEES
    https->http = http;
    while (1)
    {
		tickCheck = 0;
        mbedtls_reset(https);
        http_disconnect(http);
        http_headerDel(http, "Range");
        if (pos + lenBlock >= wf->size)
            lenBlock = wf->size - pos;
        http_headerAdd(http, "Range: bytes=%d-%d\r\n", pos, pos + lenBlock - 1);
        if (mbedtls_request(https, buf, lenBuf) != lenBlock)
        {
            errCount++;
            if (errCount >= 3)
                goto error;
        }
        else
        {
            errCount = 0;
            /* 计算累加和 */
            chksum += getSum(buf, 0, lenBlock);
            chksum &= 0xFF;
            //		printf("chksum: %d\r\n", chksum);
            /* 写入文件 */
			proc(buf, lenBlock);
            /* 传输下一段 */
            pos += lenBlock;
			printf("Process %d / %d", pos, wf->size);
            if (pos == wf->size)
                break; // 传输完毕
        }
        os_dly_wait(1);
    }
error:
    mbedtls_close(https);
    return chksum == wf->chksum;;
}

bool web_UpdateFW(http_session *http, FwInfo *fw, char *buf)
{
    int pos, chksum, lenBlock;
    pos = chksum = 0;
    lenBlock = 2048;

    if (http_connect(http) != HTTP_OK)
        goto error;
    while (1)
    {
        http_headerDel(http, "Range");
        if (pos + lenBlock >= fw->size)
            lenBlock = fw->size - pos;
        http_headerAdd(http, "Range: bytes=%d-%d\r\n", pos, pos + lenBlock - 1);
        if (http_request(http, buf) != lenBlock)
            return 0;
        /* 计算累加和 */
        chksum += getSum(buf, 0, lenBlock);
        chksum &= 0xFF;
        /* 写入新程序空间 */
        flash_writeApp(pos, buf, lenBlock);
        /* 传输下一段 */
        pos += lenBlock;
        if (pos == fw->size)
            break; // 传输完毕
        os_dly_wait(1);
    }
    return chksum == fw->chksum; // 返回校验结果
error:
    return false;
}
bool web_UpdateFW_HTTPS(http_session *http, FwInfo *fw, char *buf, int lenBuf)
{
    bool ret = false;
    int pos, chksum, lenBlock;
    pos = chksum = 0;
    lenBlock = 2048; // F103 Flash 单页为2048字节
    int errCount = 0;
    mbedtls_session *https = mbedtls_new(CA_SWISS); // CA_PUDOUPDATEES
    https->http = http;
    while (1)
    {
        mbedtls_reset(https);
        http_disconnect(http);
        http_headerDel(http, "Range");
        if (pos + lenBlock >= fw->size)
            lenBlock = fw->size - pos;
        http_headerAdd(http, "Range: bytes=%d-%d\r\n", pos, pos + lenBlock - 1);
        if (mbedtls_request(https, buf, lenBuf) != lenBlock)
        {
            errCount++;
            if (errCount >= 3)
                goto error;
        }
        else
        {
            errCount = 0;
            /* 计算累加和 */
            chksum += getSum(buf, 0, lenBlock);
            chksum &= 0xFF;
            //		printf("chksum: %d\r\n", chksum);
            /* 写入新程序空间 */
            flash_writeApp(pos, buf, lenBlock);
            /* 传输下一段 */
            pos += lenBlock;
            if (pos == fw->size)
                break; // 传输完毕
        }
        os_dly_wait(1);
    }
    ret = (chksum == fw->chksum); // 返回校验结果
error:
    mbedtls_close(https);
    return ret;
}
