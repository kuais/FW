#include "modKern.h"
#include "Kuframe/kuutil.h"
#include "Kuframe/kustring.h"
#include "W5500/utility.h"
#include "modE2p.h"
#include "modRelayBoard.h"
#include "modTime.h"
#include "modDb.h"
#include <stdlib.h>

Kern_Session *kernSession;
bool flagVersion = false; // 版本已更新标志

void kern_ClearSession(void)
{ /* 清除toke和https,继续错误计数 */
    memset(kernSession->token, 0, sizeof(kernSession->token));
    kernapi_initHttps();
    kernSession->step = 0;
}
void kern_NewSession(void)
{
    if (kernSession == NULL)
        kernSession = kernapi_new();
    kern_ClearSession();
    kernSession->errCount = 0;
}
void kern_nextStep(Kern_Session *s)
{
    curPage = 0;
    tickCheck = 0;
    s->step++;
    s->errCount = 0;
}
void kern_HandleToken(Kern_Session *s, char *text)
{ // 获取Token
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj)
    {
        strcpy(s->token, Kern_GetToken(jsonObj));
        cJSON_Delete(jsonObj);
        kern_nextStep(s);
    }
    else
        s->errCount++;
}
void kern_HandleConf(Kern_Session *s, char *text)
{ // 获取设置参数
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        //        char text[500] = {0};
        //        sprintf(text, "%s;SyncHours,1|12|22;", res.data);
        //        int ret = db_saveConfFromKern(text);
        int ret = db_saveConfFromKern(res.data);
        if (ret < res.pagesize)
            kern_nextStep(s);
        else
            curPage++;
    }
    else
        s->errCount++;
    cJSON_Delete(jsonObj);
}
void kern_HandleConfBox(Kern_Session *s, char *text)
{ // 同步格口列表
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_saveBoxFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_BOX);
            file_rename(FN_TEMP, FN_BOX);
            db_saveBoxFlagList();
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
        s->errCount++;
    cJSON_Delete(jsonObj);
}
void kern_HandleBoxPLU(Kern_Session *s, char *text)
{ // 同步预留格口
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_saveBoxPluFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_BOXPLU);
            file_rename(FN_TEMP, FN_BOXPLU);
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
        s->errCount++;
    cJSON_Delete(jsonObj);
}
void kern_HandleBoxDisaster(Kern_Session *s, char *text)
{ // 同步空闲格口
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        kern_nextStep(s);
    }
    else
        s->errCount++;
    cJSON_Delete(jsonObj);
}

void kern_HandlePLU(Kern_Session *s, char *text)
{ // 同步快递公司
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_savePluFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_PLU);
            file_rename(FN_TEMP, FN_PLU);
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandleCourier(Kern_Session *s, char *text)
{ // 同步快递员清单
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        // TODO
        kern_nextStep(s);
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandleCourierPLU(Kern_Session *s, char *text)
{ // 同步快递公司与快递员关系
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_saveCourierFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_COURIER);
            file_rename(FN_TEMP, FN_COURIER);
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandleLabelRule(Kern_Session *s, char *text)
{ // 同步
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_saveLabelRuleFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_LABELRULE);
            file_rename(FN_TEMP, FN_LABELRULE);
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandleLabelPLU(Kern_Session *s, char *text)
{ // 同步
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_saveLabelPluFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_LABELPLU);
            file_rename(FN_TEMP, FN_LABELPLU);
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandleTechnician(Kern_Session *s, char *text)
{ // 同步管理员清单
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_saveTechnicianFromKern(res.data);
        if (ret < res.pagesize)
        {
            file_delete(FN_TECHNICIAN);
            file_rename(FN_TEMP, FN_TECHNICIAN);
            kern_nextStep(s);
        }
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandlePreReservation(Kern_Session *s, char *text)
{ // 同步
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_savePreReservationFromKern(res.data);
        if (ret < res.pagesize)
            kern_nextStep(s);
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}
void kern_HandleReservation(Kern_Session *s, char *text)
{ // 同步
    Kern_Result res;
    cJSON *jsonObj = cJSON_Parse(text);
    if (jsonObj && Kern_ParseResult(jsonObj, &res))
    {
        int ret = db_setReservationFromKern(res.data);
        if (ret < res.pagesize)
            kern_nextStep(s);
        else
            curPage++;
    }
    else
    {
        s->errCount++;
    }
    cJSON_Delete(jsonObj);
}

static void kern_Pack(char *text, char *buf)
{
    cJSON *jObj = cJSON_CreateObject();
    cJSON_AddItemToObject(jObj, "data", cJSON_CreateString(text));
    cJSON_AddItemToObject(jObj, "error", cJSON_CreateString(""));
    char *str = cJSON_Print(jObj);
    cJSON_Delete(jObj);
    strcpy(buf, str);
    myfree(str);
}
void kern_DataOfVersion(char *ver, char *buf)
{
    sprintf(buf, "%s,%s", ver, ver);
    kern_Pack(buf, buf);
}
void kern_DataOfBatteryLevel(char *v, char *buf)
{
    sprintf(buf, "%d", v);
    kern_Pack(buf, buf);
}
void kern_DataOfLocksStatus(char *buf)
{
    Time t;
    time_now(&t);
    sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d.000,",
            t.year, t.month, t.day, t.hour, t.minute, t.second);
    u16 relay;
    u8 side;
    for (int i = 0; i < boxCount; i++)
    {
        side = 2 - (boxList[i].id / 1000);
        relay = (boxList[i].id % 1000) - 1;
        sprintf(buf, "%s%d", buf, 1 - relayBoard_GetLockStatus(side, relay));
    }
    kern_Pack(buf, buf);
}
void kern_DataOfSensorsStatus(char *buf)
{
    // char text[100] = {0};
    Time t;
    time_now(&t);
    sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d.000,",
            t.year, t.month, t.day, t.hour, t.minute, t.second);
    u16 relay;
    u8 side;
    for (int i = 0; i < boxCount; i++)
    {
        side = 2 - (boxList[i].id / 1000);
        relay = (boxList[i].id % 1000) - 1;
        sprintf(buf, "%s%d", buf, 1 - relayBoard_GetSensorStatus(side, relay));
    }
    kern_Pack(buf, buf);
}
void kern_DataofUploadReservation(Event *p, char *buf)
{
    char text[200] = {0};
    getSyncId(buf);
    getReservationText(p, text);
    sprintf(buf, "%s,%s", buf, text);
    kern_Pack(buf, buf);
}
void kern_DataOfIncident(Incident *p, char *buf)
{
    char text[200];
    getSyncId(buf);
    getIncidentText(p, text);
    sprintf(buf, "%s,%s", buf, text);
    kern_Pack(buf, buf);
}
void kern_DataOfTechnicianAction(TechnicianAction *p, char *buf)
{
    char text[200];
    getSyncId(buf);
    getTechnicianAction(p, text);
    sprintf(buf, "%s,%s", buf, text);
    kern_Pack(buf, buf);
}
