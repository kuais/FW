#include "modW5500.h"
#include "KuFrame/kuutil.h"
#include "W5500/dns.h"
#include "W5500/socket.h"
#include "W5500/w5500.h"
#include "modE2p.h"
#include "modFile.h"
#include "modFlash.h"
#include "modKern.h"
#include "modRelayBoard.h"
#include "modTime.h"
#include "modDb.h"
#include "modWeb.h"

#define W5500DEBUG_1 0

#define MACAddr_MyTcpapp_3 *(unsigned int *)(0x1FFFF7E8);
#define MACAddr_MyTcpapp_4 *(unsigned int *)(0x1FFFF7EC);
#define MACAddr_MyTcpapp_5 *(unsigned int *)(0x1FFFF7F0);

W5500_Config w5500cfg;

U16 local_port = 5000;
E_Ipfrom ipfrom = IP_FROM_E2P;
/* 以下改成static 能否更省空间? */

bool w5500_CheckConnect(void)
{ // 检查物理连接
    uint16 count = 0;
    while (count < 10000) // 10秒未联机则退出
    {
        if (curFlow > IdleFlow)
            return false;
        u8 v = getPHYCFG();
        if (1 & v)
            return true;
        count += 100; // 100ms检查1次
        os_dly_wait(100);
    }
    printLog("No network");
    return false;
}
void w5500_SetOPMD(char v)
{ // 开启物理层手动设置
    u8 cfg = 0;
    v <<= 6;
    while (1)
    {
        os_dly_wait(10);
        cfg = getPHYCFG();
        if (!(cfg & 0b10000000))
            continue;
        if ((cfg & v) == v)
            return;
        else
        {
            cfg &= 0b00111111; // RST和OPMD脚置0，PHY重启
            cfg |= v;
            setPHYCFG(cfg);
            os_dly_wait(10);
            cfg |= 0b10000000; // RST 置1
            setPHYCFG(cfg);
        }
    }
}
void w5500_LowPower(bool v)
{
    u8 cfg = getPHYCFG();
    if (v)
        cfg &= 0b11110111;
    else
        cfg |= 0b00111000;
    setPHYCFG(cfg);
    os_dly_wait(10);
    cfg = getPHYCFG();
}
static void w5500_ConfigFromDefine(void)
{
    int SerialID[3];
    SerialID[0] = MACAddr_MyTcpapp_3;
    SerialID[1] = MACAddr_MyTcpapp_4;
    SerialID[2] = MACAddr_MyTcpapp_5;

    /* MAC */
    w5500cfg.mac[0] = 0x00;
    w5500cfg.mac[1] = 0x08;
    w5500cfg.mac[2] = 0xdc;
    w5500cfg.mac[3] = (SerialID[0] & 0x000000ff) + ((SerialID[0] >> 8) & 0x000000ff) + ((SerialID[0] >> 16) & 0x000000ff) + ((SerialID[0] >> 24) & 0x000000ff);
    w5500cfg.mac[4] = (SerialID[1] & 0x000000ff) + ((SerialID[1] >> 8) & 0x000000ff) + ((SerialID[1] >> 16) & 0x000000ff) + ((SerialID[1] >> 24) & 0x000000ff);
    w5500cfg.mac[5] = (SerialID[2] & 0x000000ff) + ((SerialID[2] >> 8) & 0x000000ff) + ((SerialID[2] >> 16) & 0x000000ff) + ((SerialID[2] >> 24) & 0x000000ff);

    /* Local IP */
    w5500cfg.lip[0] = 192;
    w5500cfg.lip[1] = 168;
    w5500cfg.lip[2] = 1;
    w5500cfg.lip[3] = 188;
    /* Subnet Mask */
    w5500cfg.sub[0] = 255;
    w5500cfg.sub[1] = 255;
    w5500cfg.sub[2] = 255;
    w5500cfg.sub[3] = 0;
    /* GateWay */
    w5500cfg.gw[0] = 192;
    w5500cfg.gw[1] = 168;
    w5500cfg.gw[2] = 1;
    w5500cfg.gw[3] = 1;
    /* DNS */
    w5500cfg.dns[0] = 8;
    w5500cfg.dns[1] = 8;
    w5500cfg.dns[2] = 8;
    w5500cfg.dns[3] = 8;
}
static void w5500_ConfigFromDhcp(void) {}
static void w5500_ConfigFromE2p(void)
{
    U8 datas[12];
    e2p_GetNetParams(datas);
    /* Local IP */
    w5500cfg.lip[0] = datas[0];
    w5500cfg.lip[1] = datas[1];
    // datas[2] = 8;
    w5500cfg.lip[2] = datas[2];
    w5500cfg.lip[3] = datas[3];
    /* Subnet Mask */
    w5500cfg.sub[0] = datas[4];
    w5500cfg.sub[1] = datas[5];
    w5500cfg.sub[2] = datas[6];
    w5500cfg.sub[3] = datas[7];
    /* GateWay */
    // datas[10] = 8;
    w5500cfg.gw[0] = datas[8];
    w5500cfg.gw[1] = datas[9];
    w5500cfg.gw[2] = datas[10];
    w5500cfg.gw[3] = datas[11];
}
static void w5500_InitConfig_Net(void)
{
    w5500_ConfigFromDefine();
    if (ipfrom == IP_FROM_E2P)
        w5500_ConfigFromE2p();
    else if (ipfrom == IP_FROM_DHCP)
        w5500_ConfigFromDhcp();

    /*将IP配置信息写入W5500相应寄存器*/
    setSHAR(w5500cfg.mac);
    setSIPR(w5500cfg.lip);
    setSUBR(w5500cfg.sub);
    setGAR(w5500cfg.gw);
}
static void w5500_InitConfig_Socket(void)
{
    socket_buf_init(0, SOCKET_BUFSIZETX, SOCKET_BUFSIZERX);
    //  socket_buf_init(1, SOCKET_BUFSIZETX, SOCKET_BUFSIZERX);
    //	socket_buf_init(2, SOCKET_BUFSIZETX, SOCKET_BUFSIZERX);
    //	socket_buf_init(3, SOCKET_BUFSIZETX, SOCKET_BUFSIZERX);
}

void w5500_Init(void)
{
    apmid = (char *)boardParams.apmID;
    apiurl = (char *)boardParams.apiurl;
    w5500_Reset();
    w5500_InitConfig_Net();
    w5500_InitConfig_Socket();

    /* 检查配置 */
    //    printf("\r\n");
    //	getSHAR(w5500cfg.mac);
    //    printf("Mac          : %02X.%02X.%02X.%02X.%02X.%02X\r\n", w5500cfg.mac[0], w5500cfg.mac[1], w5500cfg.mac[2], w5500cfg.mac[3], w5500cfg.mac[4],
    //    w5500cfg.mac[5]);
    getSIPR(w5500cfg.lip);
    printf("Local IP          : %d.%d.%d.%d\r\n", w5500cfg.lip[0], w5500cfg.lip[1], w5500cfg.lip[2], w5500cfg.lip[3]);
    getSUBR(w5500cfg.sub);
    printf("Local Mask        : %d.%d.%d.%d\r\n", w5500cfg.sub[0], w5500cfg.sub[1], w5500cfg.sub[2], w5500cfg.sub[3]);
    getGAR(w5500cfg.gw);
    printf("Local Gateway: %d.%d.%d.%d\r\n", w5500cfg.gw[0], w5500cfg.gw[1], w5500cfg.gw[2], w5500cfg.gw[3]);
    printf("Local Port   : %d \r\n", local_port);
}
/**
 * @brief 更新固件
 * @param jo
 * @param buf
 * @param lenBuf
 * @return true     已更新
 * @return false    未更新
 */
static bool w5500_UpdateFw(FwInfo *fw, char *buf, int lenBuf)
{
    bool ret = false;
    http_session *http = http_new(1024);
    int iret = http_resolveAddress(http, fw->url, HTTP_GET);
    if (iret != HTTP_OK)
        goto exit;
    if (http->isHttps)
    {
        if (!dns_gethostbyname(http->host, http->serverip))
            goto exit;
        ret = web_UpdateFW_HTTPS(http, fw, buf, lenBuf);
    }
    else
    {
        char serverIP[4] = {115, 28, 175, 80};
        memcpy(http->serverip, serverIP, 4);
        ret = web_UpdateFW(http, fw, buf);
    }
exit:
    http_close(http);
    return ret;
}

#define URL_FWInFO "https://kernboardfunctions.azurewebsites.net/api/Ffirmware/%s"
#define URL_FWInFOEnd "https://kernboardfunctions.azurewebsites.net/api/FfirmwareEnd/%s/%s"
void w5500_CheckUpdate(void)
{
    char serverIP[4] = {115, 28, 175, 80};
    char content[2600];
    cJSON *jsonObj = NULL;
    boardParams.debug = true;
    char url[200];
    int ret = 0;
    bool isHttps = false;
    sprintf(url, URL_FWInFO, boardParams.apmID);
    http_session *http = http_new(1024);
    https_printf("GET url: %s\n", url);
    http_resolveAddress(http, url, HTTP_GET);
    isHttps = http->isHttps;
    http->errCount = 6;
    while (http->errCount)
    {
        if (isHttps)
        {
            if (dns_gethostbyname(http->host, serverIP))
            {
                memcpy(http->serverip, serverIP, 4);
                mbedtls_session *https = mbedtls_new(CA_SWISS); // CA_PUDOUPDATEES
                https->http = http;
                ret = mbedtls_request(https, content, sizeof(content));
                if (ret > 0)
                {
                    char *p = strstr(content, "{");
                    ret = strlen(p);
                    memcpy(content, p, ret);
                    jsonObj = cJSON_Parse(content);
                }
                mbedtls_close(https);
            }
        }
        else
        {
            if (strlen(http->serverip) == 0)
            {
                memcpy(http->serverip, serverIP, 4);
                http_headerAdd(http, "Content-Type:%s\r\n", "text/html");
            }
            if (http_connect(http) == HTTP_OK)
            {
                ret = http_request(http, content);
                if (ret > 0)
                {
                    char *p = strstr(content, "{");
                    ret = strlen(p);
                    memcpy(content, p, ret);
                    jsonObj = cJSON_Parse(content);
                }
            }
        }
        if (jsonObj)
            break;
        http->errCount--;
    }
    appSet.network = (http->errCount > 0);
    http_close(http);
    if (jsonObj)
    {
        if (cJSON_GetObjectItem(jsonObj, "flag")->valueint)
        {
            bool flagUpdated = false;
            FwInfo fw;
            cJSON *jsonFw;
            if (flash_getAppFlag() == 0)
                jsonFw = cJSON_GetObjectItem(jsonObj, "FW2");
            else
                jsonFw = cJSON_GetObjectItem(jsonObj, "FW1");
            fw.version[0] = 0;
            if (jsonFw && jsonFw->type == cJSON_Object)
            {
                strcpy(fw.version, cJSON_GetObjectItem(jsonFw, "version")->valuestring);
                fw.version[6] = 0;
                if (strcmp(fw.version, VERSION) != 0)
                {
                    sscanf(cJSON_GetObjectItem(jsonObj, "ip")->valuestring, "%d.%d.%d.%d", &fw.ip[0], &fw.ip[1], &fw.ip[2], &fw.ip[3]);
                    fw.size = cJSON_GetObjectItem(jsonFw, "size")->valueint;
                    fw.chksum = cJSON_GetObjectItem(jsonFw, "chksum")->valueint;
                    strcpy(fw.url, cJSON_GetObjectItem(jsonFw, "url")->valuestring);
                    cJSON_Delete(jsonObj);
                    printLog("Start update firmware...");
                    if (w5500_UpdateFw(&fw, content, sizeof(content)))
                    {
                        /* 切换程序空间，重启进入新程序 */
                        flash_setAppFlag(1 - flash_getAppFlag());
                        if (!flagUpdated)
                            flagUpdated = true;
                    }
                }
            }
            if (flagUpdated)
            {
                if (isHttps)
                {
                    http_session *http = http_new(1024);
                    memcpy(http->serverip, serverIP, 4);
                    if (strlen(fw.version) > 0)
                        sprintf(url, URL_FWInFOEnd, boardParams.apmID, fw.version);
                    https_printf("GET url: %s\n", url);
                    http_resolveAddress(http, url, HTTP_GET);
                    mbedtls_session *https = mbedtls_new(CA_SWISS); // CA_PUDOUPDATEES
                    https->http = http;
                    mbedtls_request(https, content, sizeof(content));
                    mbedtls_close(https);
                    http_close(http);
                }
                printLog("Update Firmware Finished");
                bsp_Restart();
                return;
            }
            else
                printf("No Updated\r\n");
        }
        cJSON_Delete(jsonObj);
    }
}

#define LENCONTENT 0x800

bool w5500_GetToken(void)
{
    if (strlen(kernSession->token) > 0)
        return false;
    kernSession->step = 0;
    char content[LENCONTENT] = {0};
    if (kernapi_token(kernSession, content, LENCONTENT) > 0)
        kern_HandleToken(kernSession, content);
    return true;
}
void w5500_UploadLockStatus(void)
{
    kernSession->step = 4;
    char content[LENCONTENT] = {0};
    kern_DataOfLocksStatus(content);
    int ret = kernapi_upload(kernSession, "LockState", content, LENCONTENT);
    if (ret > 0 || ret == -422 || ret == -500)
        kern_nextStep(kernSession);
    else if (ret == -401)
        kern_NewSession();
    else if (ret <= -400 && kernSession->errCount == 3)
        kern_nextStep(kernSession);
}
void w5500_UploadSensorStatus(void)
{
    kernSession->step = 5;
    char content[LENCONTENT] = {0};
    kern_DataOfSensorsStatus(content);
    int ret = kernapi_upload(kernSession, "SensorState", content, LENCONTENT);
    if (ret > 0 || ret == -422 || ret == -500)
        kern_nextStep(kernSession);
    else if (ret == -401)
        kern_NewSession();
    else if (ret <= -400 && kernSession->errCount == 3)
        kern_nextStep(kernSession);
}
/**
 * @brief   同步快件信息
 */
void w5500_UploadReservation(void)
{
    kernSession->step = 6;
    Event *ev = mymalloc(sizeof(Event));
    int pos = db_getEventPost(ev);
    if (pos < 0)
    { // 无数据
        myfree(ev);
        kern_nextStep(kernSession);
        return;
    }
    char content[LENCONTENT] = {0};
    kern_DataofUploadReservation(ev, content);
    myfree(ev);
    int ret = kernapi_upload(kernSession, "Reservation", content, LENCONTENT);
    if (ret > 0 || ret == -422 || ret == -500)
    {
        kernSession->errCount = 0;
        db_delEventByPos(pos); // 删除已上传的Event
        appFlag.synclater = 1;
    }
    else if (ret == -401)
        kern_NewSession();
    else if (ret <= -400 && kernSession->errCount == 3)
    {
        kernSession->errCount = 0;
        db_delEventByPos(pos); // 删除已上传的Event
    }
}
/**
 * @brief   同步错误记录
 */
void w5500_UploadIncident(void)
{
    kernSession->step = 7;
    Incident *p = mymalloc(sizeof(Incident));
    int pos = db_getIncidentPost(p);
    if (pos < 0)
    { // 无数据
        myfree(p);
        kern_nextStep(kernSession);
        return;
    }
    char content[LENCONTENT] = {0};
    kern_DataOfIncident(p, content);
    myfree(p);
    int ret = kernapi_upload(kernSession, "Incident", content, LENCONTENT);
    if (ret > 0 || ret == -422 || ret == -500)
    {
        kernSession->errCount = 0;
        db_delIncidentByPos(pos); // 删除已上传的Incident
    }
    else if (ret == -401)
        kern_NewSession();
    else if (ret <= -400 && kernSession->errCount == 3)
    {
        kernSession->errCount = 0;
        db_delIncidentByPos(pos); // 删除已上传的Incident
    }
}
/**
 * @brief 上传管理员操作记录
 */
void w5500_UploadTechnicianAction(void)
{
    kernSession->step = 8;
    TechnicianAction *p = mymalloc(sizeof(TechnicianAction));
    int pos = db_getTechnicianActionPost(p);
    if (pos < 0)
    { // 无数据
        myfree(p);
        kern_nextStep(kernSession);
        return;
    }
    char content[LENCONTENT] = {0};
    kern_DataOfTechnicianAction(p, content);
    myfree(p);
    int ret = kernapi_upload(kernSession, "TechnicianAction", content, LENCONTENT);
    if (ret > 0 || ret == -422 || ret == -500)
    {
        kernSession->errCount = 0;
        db_delTechnicianActionByPos(pos); // 删除已上传的TechnicianAction
    }
    else if (ret == -401)
        kern_NewSession();
    else if (ret <= -400 && kernSession->errCount == 3)
    {
        kernSession->errCount = 0;
        db_delTechnicianActionByPos(pos); // 删除已上传的TechnicianAction
    }
}

void w5500_SyncConf(void)
{
    kernSession->step = 21;
    char content[LENCONTENT] = {0};
    int ret = kernapi_sync(kernSession, "Conf", content, LENCONTENT);
    if (ret > 0)
        kern_HandleConf(kernSession, content);
    else if (ret == -401)
        kern_NewSession();
    else if (ret <= -400 && kernSession->errCount == 3)
        kern_nextStep(kernSession);
}
/**
 * @brief 同步预订订单信息
 */
void w5500_GetPrereservation(void)
{
    kernSession->step = 31;
    char content[LENCONTENT] = {0};
    int ret = kernapi_sync(kernSession, "PreReservation", content, LENCONTENT);
    if (ret > 0)
        kern_HandlePreReservation(kernSession, content);
    else if (ret == -401)
        kern_NewSession();
    else if (ret == -422 || ret == -500)
        kern_nextStep(kernSession);
    else if (ret <= -400 && kernSession->errCount == 3)
        kern_nextStep(kernSession);
}
/**
 * @brief 同步订单更新信息
 */
void w5500_GetReservation(void)
{
    kernSession->step = 32;
    char content[LENCONTENT] = {0};
    int ret = kernapi_sync(kernSession, "Reservation", content, LENCONTENT);
    if (ret > 0)
        kern_HandleReservation(kernSession, content);
    else if (ret == -401)
        kern_NewSession();
    else if (ret == -422 || ret == -500)
        kern_nextStep(kernSession);
    else if (ret <= -400 && kernSession->errCount == 3)
        kern_nextStep(kernSession);
}

/**
 * @brief 从服务器同步数据
 */
void w5500_SyncDown(void)
{
    char content[LENCONTENT];
    char bakStep = 0;
    int apiRet = 0;
    bool flagError = false;
    printLog("Sync Begin");
    kern_NewSession();
    while (1)
    {
        memset(content, 0, LENCONTENT);
        os_dly_wait(1);
        switch (kernSession->step)
        {
        case 0: // Token
        {
            apiRet = kernapi_token(kernSession, content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleToken(kernSession, content);
            break;
        }
        case 1: // Post Version
        {
            kern_DataOfVersion(VERSION, content);
            apiRet = kernapi_upload(kernSession, "Version", content, LENCONTENT);
            if (apiRet > 0)
                kern_nextStep(kernSession);
            break;
        }
        case 2: // Post BatteryLevel
        {
            kern_DataOfBatteryLevel(bsp_Voltage(), content);
            apiRet = kernapi_upload(kernSession, "BatteryLevel", content, LENCONTENT);
            if (apiRet > 0)
                kern_nextStep(kernSession);
            break;
        }
        case 3: // Post TechnicianAction
        {
            size_t size = sizeof(TechnicianAction);
            TechnicianAction *p = mymalloc(size);
            int pos = db_getTechnicianActionPost(p);
            if (pos < 0)
            { // 无数据
                myfree(p);
                kern_nextStep(kernSession);
            }
            else
            {
                kern_DataOfTechnicianAction(p, content);
                myfree(p);
                apiRet = kernapi_upload(kernSession, "TechnicianAction", content, LENCONTENT);
                if (apiRet > 0)
                {
                    kernSession->errCount = 0;
                    db_delTechnicianActionByPos(pos); // 删除已上传的TechnicianAction
                }
            }
            break;
        }
        case 21: // Sync Conf
        {
            apiRet = kernapi_sync(kernSession, "Conf", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleConf(kernSession, content);
            break;
        }
        case 22: // Sync ConfBox
        {
            apiRet = kernapi_sync(kernSession, "ConfBox", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleConfBox(kernSession, content);
            break;
        }
        case 23: // Sync BoxPlu
        {
            apiRet = kernapi_sync(kernSession, "BoxConfigurationPLU", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleBoxPLU(kernSession, content);
            break;
        }
        case 24: // Sync PLU
        {
            apiRet = kernapi_sync(kernSession, "Plu", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandlePLU(kernSession, content);
            break;
        }
        case 25: // Sync Courier
        {
            apiRet = kernapi_sync(kernSession, "Courier", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleCourier(kernSession, content);
            break;
        }
        case 26: // Sync CourierPlu
        {
            apiRet = kernapi_sync(kernSession, "CourierPlu", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleCourierPLU(kernSession, content);
            break;
        }
        case 27: // Sync LabelRule
        {
            apiRet = kernapi_sync(kernSession, "LabelRule", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleLabelRule(kernSession, content);
            break;
        }
        case 28: // Sync LabelPLU
        {
            apiRet = kernapi_sync(kernSession, "LabelPLU", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleLabelPLU(kernSession, content);
            break;
        }
        case 29: // Sync technician
        {
            apiRet = kernapi_sync(kernSession, "technician", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleTechnician(kernSession, content);
            break;
        }
        case 30:
        {
            apiRet = kernapi_sync(kernSession, "BoxConfigurationDisaster", content, LENCONTENT);
            if (apiRet > 0)
                kern_HandleBoxDisaster(kernSession, content);
            break;
        }
        // case 31: // Sync PreReservation
        // {
        //     if (kernapi_sync(kernSession, "PreReservation", content, LENCONTENT) > 0)
        //         kern_HandlePreReservation(kernSession, content);
        //     break;
        // }
        // case 32: // Sync Reservation
        // {
        //     if (kernapi_sync(kernSession, "Reservation", content, LENCONTENT) > 0)
        //         kern_HandleReservation(kernSession, content);
        //     break;
        // }
        default:
            goto exit; // 退出
        }

        if (kernSession->step == 0)
        {
            if (((kernSession->errCount - 3) % 3) == 0)
                w5500_Init();
            if (kernSession->errCount >= 6)
            {
                flagError = true;
                goto exit;
            }
        }
        else if (apiRet == -401)
        { // 过期
            bakStep = kernSession->step;
            kern_NewSession();
        }
        else if (apiRet == -422 || apiRet == -500)
        { // 格式错误
            kern_nextStep(kernSession);
        }
        else if (kernSession->errCount >= 3)
        {
            if (apiRet <= -400)
            {
                flagError = true;
                kern_nextStep(kernSession);
            }
            else
            { /* 其他网络错误 */
                w5500_Init();
                bakStep = kernSession->step; // 重试后继续传输
                kern_ClearSession();
            }
        }
        else if (kernSession->errCount == 0)
        {
            /* 流程调整 */
            if (bakStep != 0)
            {
                kernSession->step = bakStep;
                bakStep = 0;
            }
            // if (kernSession->step == 2)
            //     kernSession->step = 29;
            //            if (kernSession->step == 2)
            //                kernSession->step++;
            else if (kernSession->step == 3)
                kernSession->step = 21;
            else if (kernSession->step == 25)
                kernSession->step = 26; // 跳过Courier
        }
    }
exit:
    kernSession->step = 0;
    kernSession->errCount = 0;
    printLog("Sync End");
    db_newIncident(flagError ? IT_SyncFailed : IT_SyncFunish, 0, "");
}
