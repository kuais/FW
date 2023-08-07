/**
 * @file modDb.c
 * @author Kuais (you@domain.com)
 * @brief   数据操作
 * @version 0.1
 * @date 2022-08-17
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "modDb.h"
#include "modFile.h"
#include "modTime.h"
#include "Kuframe/kustring.h"

void db_InitBoxFlag(void)
{
    boxFlagList = mycalloc(512, 1);
    // memset(boxFlagList, 0x10, 512);
    FIL file;
    int fd = file_openNew(&file, FN_BOXFLAG); // 重建BOXFLAG文件
    if (!fd)
        return;
    file_write(&file, boxFlagList, 512);
    file_close(&file);
}
/**
 * @brief 加载格口标志列表
 * 512字节 0-255 为左边格口； 256-511为右边格口
 */
void db_loadBoxFlagList(void)
{
    if (boxFlagList != NULL)
        return;
    int fd;
    FIL file;
    DWORD len;
    fd = file_openRead(&file, FN_BOXFLAG);
    if (!fd)
        db_InitBoxFlag();
    else
    {
        len = file_length(&file);
        if (len != 512)
        {
            file_close(&file);
            db_InitBoxFlag();
        }
        else
        {
            boxFlagList = mycalloc(512, 1);
            fd = file_read(&file, boxFlagList, 512);
            file_close(&file);
            if (!fd)
                db_InitBoxFlag();
        }
    }
}
void db_saveBoxFlagList(void)
{
    if (boxFlagList == NULL)
        return;
    int fd;
    FIL file;
    int ret;
    fd = file_openWrite(&file, FN_BOXFLAG);
    if (!fd)
        fd = file_openNew(&file, FN_BOXFLAG);
    ret = file_write(&file, boxFlagList, 512);
    file_close(&file);
    if (ret <= 0)
        dprintf("Save Box Flag to DB Failed");
}
/**
 * @brief   单独读取1个格口标志
 * @param data  格口标志保存地址
 * @param pos   位置
 */
void db_loadBoxFlag(char *data, u16 pos)
{
    int fd;
    FIL file;
    fd = file_openRead(&file, FN_BOXFLAG);
    if (!fd)
    {
        *data = 0;
        return;
    }
    file_move(&file, pos);
    file_read(&file, data, 1);
    file_close(&file);
}
/**
 * @brief 单独更新1个格口标志
 * @param data 要更新的格口标志
 * @param pos  位置
 */
void db_saveBoxFlag(char *data, u16 pos)
{
    int fd;
    FIL file;
    fd = file_openWrite(&file, FN_BOXFLAG);
    if (!fd)
        fd = file_openNew(&file, FN_BOXFLAG);
    file_move(&file, pos);
    file_write(&file, data, 1);
    file_close(&file);
}

/**
 * @brief 加载格口列表
 * boxCount 格口总数，boxList 格口列表
 */
void db_loadBoxList(void)
{
    if (boxList != NULL)
        return;
    FIL file;
    int fd = file_openRead(&file, FN_BOX); // 重建BOX文件
    if (!fd)
        goto exit;
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2;
    size_t size = sizeof(Box);
    boxCount = len / 2;
    boxList = mycalloc(boxCount, sizeof(U16));
    int i = 0;
    file_move(&file, 0);
    while (i < boxCount)
    {
        file_read(&file, &boxList[i], size);
        i++;
    }
exit2:
    file_close(&file);
exit:
    return;
}
bool db_addShipment(Shipment *p)
{
    int ret;
    size_t size = sizeof(Shipment);
    FIL file;
    int fd = file_openAppend(&file, FN_SHIPMENT);
    if (!fd)
        fd = file_openNew(&file, FN_SHIPMENT);
    ret = file_write(&file, p, size);
    file_close(&file);
    if (ret > 0)
        return true;
    dprintf("Add Shipment to DB Failed");
    return false;
}
void db_delShipment(Shipment *shipment)
{
    size_t size = sizeof(Shipment);
    int pos = file_find(FN_SHIPMENT, shipment, size);
    db_delShipmentByPos(pos);
}
void db_delShipmentByPos(int pos)
{
    size_t size = sizeof(Shipment);
    file_remove(FN_SHIPMENT, pos, size);
}
void db_setShipmentByPos(Shipment *shipment, DWORD pos)
{
    size_t size = sizeof(Shipment);
    FIL file;
    file_openWrite(&file, FN_SHIPMENT);
    file_move(&file, pos);
    file_write(&file, shipment, size);
    file_close(&file);
}
void db_setShipment(Shipment *p)
{
    int pos = db_findShipmentByBoxID(p->boxid);
    db_setShipmentByPos(p, pos);
}
/**
 * @brief 根据条码查找预订单
 * @param barcode   条码
 * @param sp        找到的订单数据
 * @return int      找到的订单在文件的位置
 */
int db_findPreReservationByBarcode(char *barcode, PreReservation *pr)
{
    int ret = -1;
    FIL file;
    int fd = file_openRead(&file, FN_PRERESER); // 读取Courier数据
    if (!fd)
        goto exit;
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    size_t size = sizeof(PreReservation);
    DWORD pos = 0;
    while (pos < len)
    {
        file_read(&file, pr, size);
        if (strcmp(barcode, pr->barcode) == 0)
        {
            ret = pos;
            break;
        }
        pos += size;
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
void db_delPreReservation(char *barcode)
{
    PreReservation m;
    int pos = db_findPreReservationByBarcode(barcode, &m);
    if (pos >= 0)
    {
        size_t size = sizeof(PreReservation);
        file_remove(FN_PRERESER, pos, size);
    }
}
bool db_addEvent(Event *p)
{
    int ret;
    size_t size = sizeof(Event);
    FIL file;
    int fd = file_openAppend(&file, FN_EVENT);
    if (!fd)
        fd = file_openNew(&file, FN_EVENT);
    Time t;
    time_now(&t);
    sprintf(p->time, "%02d-%02d-%02d %02d:%02d:%02d",
            t.year, t.month, t.day, t.hour, t.minute, t.second);
    ret = file_write(&file, p, size);
    file_close(&file);
    if (ret > 0)
        return true;
    dprintf("Add Event to DB Failed");
    return false;
}
bool db_delEvent(Event *event)
{
    size_t size = sizeof(Event);
    int pos = file_find(FN_EVENT, event, size);
    if (pos >= 0)
        return db_delEventByPos(pos);
    return true;
}
bool db_delEventByPos(DWORD pos)
{
    file_remove(FN_EVENT, pos, sizeof(Event));
    return true;
}
bool db_newTechnicianAction(char type, char *user)
{
    TechnicianAction m;
    m.type = type;
    memcpy(m.user, user, 13);
    return db_addTechnicianAction(&m);
}
bool db_addTechnicianAction(TechnicianAction *p)
{
    int ret;
    size_t size = sizeof(TechnicianAction);
    FIL file;
    int fd = file_openAppend(&file, FN_TECHACTION);
    if (!fd)
        fd = file_openNew(&file, FN_TECHACTION);
    Time t;
    time_now(&t);
    sprintf(p->time, "%02d-%02d-%02d %02d:%02d:%02d",
            t.year, t.month, t.day, t.hour, t.minute, t.second);
    ret = file_write(&file, p, size);
    file_close(&file);
    if (ret > 0)
        return true;
    dprintf("Add TechnicianAction in DB Failed");
    return false;
}
bool db_delTechnicianActionByPos(DWORD pos)
{
    file_remove(FN_TECHACTION, pos, sizeof(TechnicianAction));
    return true;
}
bool db_newIncident(char type, u16 boxid, char *info)
{
    Incident m;
    m.type = type;
    m.boxid = boxid;
    strcpy(m.info, info);
    return db_addIncident(&m);
}
bool db_addIncident(Incident *p)
{
    int ret;
    size_t size = sizeof(Incident);
    FIL file;
    int fd = file_openAppend(&file, FN_INCIDENT);
    if (!fd)
        fd = file_openNew(&file, FN_INCIDENT);
    Time t;
    time_now(&t);
    sprintf(p->time, "%02d-%02d-%02d %02d:%02d:%02d",
            t.year, t.month, t.day, t.hour, t.minute, t.second);
    ret = file_write(&file, p, size);
    file_close(&file);
    if (ret > 0)
        return true;
    dprintf("Add Incident to DB Failed");
    return false;
}
bool db_delIncidentByPos(DWORD pos)
{
    file_remove(FN_INCIDENT, pos, sizeof(Incident));
    return true;
}
bool db_addResShipment(ResShipment *p)
{
    size_t size = sizeof(ResShipment);
    FIL file;
    int fd = file_openAppend(&file, FN_RESSHIPMENT);
    if (!fd)
        fd = file_openNew(&file, FN_RESSHIPMENT);
    Time t;
    time_now(&t);
    time_addMinute(&t, 3);
    p->time = timeToU32(&t);
    file_write(&file, p, size);
    file_close(&file);
    return true;
}
bool db_delResShipmentByTime(u32 time)
{
    bool ret = true;
    FIL file;
    int fd = file_openRead(&file, FN_RESSHIPMENT); //
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    ResShipment m;
    size_t size = sizeof(ResShipment);
    DWORD pos = 0;
    file_move(&file, pos);
    while (1)
    {
        file_read(&file, &m, size);
        if ((m.time >= time))
        {
            if (pos == 0)
                break; // 所有记录都有效
            file_close(&file);
            // 删除之前的记录
            file_remove(FN_RESSHIPMENT, 0, pos);
            return true;
        }
        else if (pos == len)
        { // 所有记录都过期
            file_close(&file);
            // 删除所有记录
            file_truncate(FN_RESSHIPMENT, 0);
            return true;
        }
        pos += size;
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
bool db_delResShipmentByPos(DWORD pos)
{
    size_t size = sizeof(ResShipment);
    file_remove(FN_RESSHIPMENT, pos, size);
    return true;
}

DWORD db_getResShipment(ResShipment *p, char type, char *text)
{
    DWORD ret = 0;
    FIL file;
    int fd = file_openRead(&file, FN_RESSHIPMENT); // 读取Courier数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    DWORD pos = 0;
    size_t size = sizeof(ResShipment);
    file_move(&file, pos);
    while (pos < len)
    {
        pos += file_read(&file, p, size);
        if (p->type == type)
        {
            if (type == 0)
            {
                if (strcmp(p->barcode, text) == 0)
                {
                    ret = pos;
                    break;
                }
            }
            else
            {
                if (strcmp(p->pin, text) == 0)
                {
                    ret = pos;
                    break;
                }
            }
        }
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
int db_saveConfFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        if (strcmp(arr2[0], "SyncHourDay") == 0)
        {
            boardParams.rebootTime = atoi(arr2[1]);
        }
        else if (strcmp(arr2[0], "SetTime") == 0)
        { /* "8/31/2022 1:13:13 AM" */
            Time t;
            int arr3[6] = {0};
            int n = 0;
            for (int j = 0; j < strlen(arr2[1]); j++)
            {
                if (arr2[1][j] == '/' || arr2[1][j] == ' ' || arr2[1][j] == ':')
                {
                    n++;
                }
                else if ((arr2[1][j] == 'A') || (arr2[1][j] == 'P') || (arr2[1][j] == '.'))
                {
                    break;
                }
                else
                {
                    arr3[n] = arr3[n] * 10 + (arr2[1][j] - 0x30);
                }
            }
            t.year = arr3[2] - 2000;
            t.month = arr3[0];
            t.day = arr3[1];
            t.hour = arr3[3];
            t.minute = arr3[4];
            t.second = arr3[5];
            time_set(&t);
        }
        else if (strcmp(arr2[0], "SyncHours") == 0)
        {
            int hour = time_hour();
            int count3 = 0;
            char **arr3 = split(arr2[1], "|", &count3);
            boardParams.rebootTime = atoi(arr3[0]);
            for (int j = 1; j < count3; j++)
            {
                int v = atoi(arr3[j]);
                if (hour < v)
                {
                    boardParams.rebootTime = v;
                    break;
                }
            }
            myfree(arr3);
        }
        else if (strcmp(arr2[0], "EnableLOG") == 0)
        {
            appFlag.synclog = (strcmp(arr2[1], "true") == 0);
        }
        myfree(arr2);
    }
    myfree(arr1);
    ret = count1;
    return ret;
}
/**
 * @brief 服务器同步格口信息后存到文件中
 * "1001,13;1002,12;1003,12;1004,12;1005,11;1006,11;1007,11;2001,11;2002,13;2003,12;2004,12;2005,12;2006,11;2007,11;2008,11;2009,11"
 * @param input   格口信息字符串
 * @return int      0:失败 1：成功
 */
int db_saveBoxFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 新建TEMP文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加BOX文件
    if (!fd)
        goto exit;
    Box m;
    size_t size = sizeof(Box);
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        char side, relay;
        u16 index;
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        m.id = atoi(arr2[0]);
        side = 2 - (m.id / 1000);
        if (side > 1)
        {
            myfree(arr2);
            continue;
        }
        relay = (m.id % 1000) - 1;
        index = side * 256 + relay;
        boxFlagList[index].size = getSizeValue(atoi(arr2[1])); // 更新格口尺寸
        myfree(arr2);
        fd = file_write(&file, &m, size); // 更新Box文件
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save ConfBox to DB Failed.");
    return ret;
}
/**
 * @brief 保存快递员信息
 * "123456444555,1"
 * @param input
 * @return int  0:失败 1：成功
 */
int db_saveCourierFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 重建文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加
    if (!fd)
        goto exit;
    Courier m;
    size_t size = sizeof(Courier);
    memset(&m, 0, size);
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        strncpy(m.id, arr2[0], 12);
        strncpy(m.plu, arr2[1], 8);
        myfree(arr2);
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save Courier to DB Failed.");
    return ret;
}
/**
 * @brief   保存条码规则
 * "1zUPS,QRC,18,^[a-zA-Z0-9]{18},1,18,;
 *  infoNoticeUPS,QRC,12,^[a-zA-Z0-9]{12},1,12,",
 * @param input
 * @return int  0:失败 1：成功
 */
int db_saveLabelRuleFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 重建文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加
    if (!fd)
        goto exit;
    LabelRule m;
    size_t size = sizeof(LabelRule);
    memset(&m, 0, size);
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        strncpy(m.rulename, arr2[0], 29);
        strncpy(m.codetype, arr2[1], 3);
        m.length = atoi(arr2[2]);
        strncpy(m.regex, arr2[3], 255);
        m.pos0 = atoi(arr2[4]);
        m.pos1 = atoi(arr2[5]);
        myfree(arr2);
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save LabelRule to DB Failed.");
    return ret;
}
/**
 * @brief 保存管理员信息
 * 9897969897,PLU2;9897959897,PLU2
 * @param input
 * @return int  0:失败 1：成功
 */
int db_saveTechnicianFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 重建文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加
    if (!fd)
        goto exit;
    Technician m;
    size_t size = sizeof(Technician);
    memset(&m, 0, size);
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        strncpy(m.id, arr2[0], 12);
        m.type = atoi(arr2[1]);
        myfree(arr2);
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save Technician to DB Failed.");
    return ret;
}
/**
 * @brief 保存快递公司保留格口信息
 * "SwissPost,11,2; PLU2,12,2",
 * @param input
 * @return int  0:失败 1：成功
 */
int db_saveBoxPluFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 重建文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加
    if (!fd)
        goto exit;
    BoxPlu m;
    size_t size = sizeof(BoxPlu);
    m.plu[8] = 0;
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        strncpy(m.plu, arr2[0], 8);
        m.size = getSizeValue(atoi(arr2[1]));
        m.reserved = atoi(arr2[2]);
        myfree(arr2);
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save BoxPlu to DB Failed.");
    return ret;
}
/**
 * @brief 保存快递公司信息
 * "SwissPost,11,2; PLU2,12,2",
 * @param input
 * @return int  0:失败 1：成功
 */
int db_savePluFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 重建文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加
    if (!fd)
        goto exit;
    Plu m;
    size_t size = sizeof(Plu);
    m.plu[8] = 0;
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        strncpy(m.plu, arr2[0], 8);
        m.flag = !strcmp("True", arr2[1]);
        myfree(arr2);
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save Plu to DB Failed.");
    return ret;
}

/**
 * @brief 保存快递公司条码规则
 *
 * @param input
 * @return int  0:失败 1：成功
 */
int db_saveLabelPluFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    if (curPage == 0)
        fd = file_openNew(&file, FN_TEMP); // 重建文件
    else
        fd = file_openAppend(&file, FN_TEMP); // 继续添加B
    if (!fd)
        goto exit;
    LabelPlu m;
    size_t size = sizeof(LabelPlu);
    memset(&m, 0, size);
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        strncpy(m.rulename, arr2[0], 29);
        m.type = atoi(arr2[1]);
        strncpy(m.plu, arr2[2], 8);
        myfree(arr2);
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save LabelPlu to DB Failed.");
    return ret;
}

/**
 * @brief 保存预订
 * "UPS,2209060004,123456,1,2,",
 * @param input
 * @return int  0:失败 1：成功
 */
int db_savePreReservationFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    FIL file;
    int fd;
    fd = file_openAppend(&file, FN_PRERESER); // 继续添加
    if (!fd)
        fd = file_openNew(&file, FN_PRERESER);
    if (!fd)
        goto exit;
    PreReservation m;
    size_t size = sizeof(PreReservation);
    memset(&m, 0, size);
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        if (!fd)
            break;
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        if (strcmp(arr2[3], "1") == 0)
        { // 新增
            strncpy(m.plu, arr2[0], 8);
            strncpy(m.barcode, arr2[1], 29);
            strncpy(m.pin, arr2[2], 6);
            m.type = 2 - atoi(arr2[4]); // 1是用户， 2是快递员，需调整
            m.size = 0;
            if (strlen(arr2[5]) > 0)
                m.size = atoi(arr2[5]);
            m.onlyApp = !(strcmp(arr2[6], "False") == 0);
            fd = file_write(&file, &m, size);
        }
        else if (strcmp(arr2[3], "3") == 0)
        { // 取消
            file_close(&file);
            db_delPreReservation(arr2[1]);
            fd = file_openAppend(&file, FN_PRERESER); // 重新打开文件进行写入
        }
        myfree(arr2);
    }
    myfree(arr1);
    ret = count1;
    file_close(&file);
exit:
    if (ret == 0)
        dprintf("Save PreReservation to DB Failed.");
    return ret;
}

/**
 * @brief 更新订单状态
 *
 * @param input
 * @return int  0:失败 1：成功
 */
int db_setReservationFromKern(char *input)
{
    if (strlen(input) == 0)
        return -1;
    int ret = 0;
    Shipment m;
    int count1 = 0;
    char **arr1 = split(input, ";", &count1);
    for (int i = 0; i < count1; i++)
    {
        int count2 = 0;
        char **arr2 = split(arr1[i], ",", &count2);
        if (strcmp(arr2[3], "15") == 0)
        {
            int pos = db_getShipmentByBarcode(arr2[0], arr2[1], &m);
            if (pos >= 0)
            {
                m.status = SS_RETURN;
                db_setShipmentByPos(&m, pos);
            }
        }
        else if (strcmp(arr2[3], "66") == 0)
        {
            int pos = db_getShipmentByBarcode(arr2[0], arr2[1], &m);
            if (pos >= 0)
            {
                m.onlyApp = 1;
                db_setShipmentByPos(&m, pos);
            }
        }
        myfree(arr2);
    }
    myfree(arr1);
    ret = count1;
    return ret;
}

/**
 * @brief       管理员帐号验证
 * @param id    输入的帐号
 * @return char 0:失败, 1:超级管理员，2:管理员
 */
char db_checkTechnician(char *id)
{
    char ret = 0;
    if (strlen(id) != 12)
        goto exit; // 技术员id为12位字符
    FIL file;
    int fd = file_openRead(&file, FN_TECHNICIAN); // 打开文件读取
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    else
    {
        Technician *m;
        size_t size = sizeof(Technician);
        char text[size];
        int iret;
        file_move(&file, 0);
        while (len > 0)
        {
            iret = file_read(&file, text, size);
            m = (Technician *)text;
            if (strcmp(id, m->id) == 0)
            {
                ret = m->type;
                break;
            }
            len -= iret;
        }
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief           获取快递员所属快递公司
 * @param id        输入的帐号
 * @return char*    0: 未找到,不是快递员; 1:找到
 */
char db_getPluOfCourier(char *id)
{
    char ret = 0;
    if (strlen(id) != 12 && strlen(id) != 10)
        goto exit; // 快递员id为10位字符
    FIL file;
    int fd = file_openRead(&file, FN_COURIER); // 读取Courier数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    Courier m;
    size_t size = sizeof(Courier);
    DWORD pos = 0;
    file_move(&file, pos);
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        if (strcmp(id, m.id) == 0)
        {
            // curPlu = mymalloc(9 * sizeof(char));
            strcpy(shipment->plu, m.plu);
            ret = 1;
            break;
        }
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief           获取快递公司参数
 * @param plu       输入的快递公司
 * @return char     0: 不允许热投递 1：允许热投递
 */
char db_getFlagOfPlu(char *plu)
{
    char ret = 0;
    FIL file;
    int fd = file_openRead(&file, FN_PLU); // 读取Courier数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    Plu m;
    size_t size = sizeof(Plu);
    DWORD pos = 0;
    file_move(&file, pos);
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        if (strcmp(plu, m.plu) == 0)
            return m.flag;
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief 获取快递公司关联的LabelRule
 * @param plu       快递公司名称
 * @param type      1 Delivery Carrier, 2 Pickup Client, 3 Delivery Client
 * @param offset    要查询的起始位置
 * @return char*    LabelRule的名称
 */
LabelPlu *db_getRuleOfPlu(char *plu, char type, DWORD *offset)
{
    LabelPlu *ret = NULL;
    FIL file;
    int fd = file_openRead(&file, FN_LABELPLU); // 打开文件读取数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = *offset;
    if (len <= pos)
        goto exit2; // 无数据
    size_t size = sizeof(LabelPlu);
    file_move(&file, pos);
    ret = mycalloc(1, size);
    while (1)
    {
        if (pos >= len)
        {
            myfree(ret);
            ret = NULL;
            break;
        }
        pos += file_read(&file, ret, size);
        if (type == ret->type)
        {
            if (type == RT_CourierDO)
            {
                if (strcmp(plu, ret->plu) == 0)
                    break;
            }
            else
                break;
        }
    }
exit2:
    file_close(&file);
    *offset = pos;
exit:
    return ret;
}
/**
 * @brief 根据名称获取条码规则
 * @param rulename      规则名称
 * @return LabelRule*   规则详情
 */
LabelRule *db_getLabelRule(char *rulename)
{
    LabelRule *ret = NULL;
    if (rulename == NULL)
        goto exit; // 无效名称
    FIL file;
    int fd = file_openRead(&file, FN_LABELRULE); // 打开文件读取数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    DWORD pos = 0;
    size_t size = sizeof(LabelRule);
    file_move(&file, pos);
    ret = mycalloc(1, size);
    while (pos < len)
    {
        if (pos >= len)
        {
            myfree(ret);
            ret = NULL;
            break;
        }
        pos += file_read(&file, ret, size);
        if (strcmp(rulename, ret->rulename) == 0)
            break;
    }
exit2: // 关闭打开的文件
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief 获取Plu指定size已使用的格口数
 * @param plu 指定plu
 * @param size 指定尺寸
 * @return int 已使用格口数量
 */
int getBoxUsedCountOfPlu(char *plu, char size)
{
    int ret = 0;
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT);
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    Shipment m;
    size_t size0 = sizeof(Shipment);
    DWORD pos = 0;
    file_move(&file, pos);
    while (pos < len)
    {
        pos += file_read(&file, &m, size0);
        if (strcmp(m.plu, plu) == 0)
        {
            if (m.size == size)
                ret++;
        }
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief 获取指定尺寸格口需保留的数量
 * @param size  指定尺寸
 * @return int  需保留的数量
 */
int getBoxReservedCount(char *plu, char size)
{
    u16 ret = 0; // 需要保留的格口数
    FIL file;
    int fd = file_openRead(&file, FN_BOXPLU); // 读取Box数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    BoxPlu m;
    size_t size0 = sizeof(BoxPlu);
    DWORD pos = 0;
    file_move(&file, pos);
    while (pos < len)
    {
        pos += file_read(&file, &m, size0);
        if (strcmp(m.plu, plu) != 0)
        { // 只需要计算为其他公司保留的格口
            if (m.size == size)
            { // 只计算指定尺寸
                int usedCount = getBoxUsedCountOfPlu(m.plu, size);
                if (m.reserved > usedCount)
                    ret += (m.reserved - usedCount);
            }
        }
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief 为投递分配格口
 * @param plu
 * @param size 0:S；1:M; 2:L; 3:Else
 * @return u16 <1000:无效格口， 其他:格口id
 */
u16 getBoxIdToDeposit(char *plu, char size)
{
    dprintf("Begin getBoxIdToDeposit,Size-%d", size + 10);
    u16 i, ret = 0;
    u16 count = 0, index; // 总可用格口数
    u8 side, relay, isused;
    for (i = 0; i < boxCount; i++)
    {
        side = 2 - (boxList[i].id / 1000);
        relay = (boxList[i].id % 1000) - 1;
        index = side * 256 + relay;
        if ((boxFlagList[index].size == size) && (boxFlagList[index].status == 0))
        {
            if (ret == 0)
            {
                ret = boxList[i].id;
                isused = boxFlagList[index].used;
            }
            else if (boxFlagList[index].used != isused)
            { // 平均分配 分配格口时比较2个格口，如果两个格口used标志相同则分配前面的格口，否则分配后面的格口
                ret = boxList[i].id;
                isused = boxFlagList[index].used;
            }
            count++;
        }
    }
    if (count <= getBoxReservedCount(plu, size))
        ret = 0; // 可用格口比要保留的格口少，不可分配
    dprintf("End getBoxIdToDeposit: %d\r\n", ret);
    return ret;
}
/**
 * @brief 查找订单在文件位置
 * @param boxid   格口号
 * @return int -1:未找到 其他: 在文件中位置
 */
int db_findShipmentByBoxID(u16 boxid)
{
    int ret = -1;
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT); // 读取Courier数据
    if (!fd)
        goto exit;
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    DWORD pos = 0;
    Shipment m;
    size_t size = sizeof(Shipment);
    file_move(&file, pos);
    while (pos < len)
    {
        file_read(&file, &m, size);
        if (boxid == m.boxid)
        {
            ret = pos;
            break;
        }
        pos += size;
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
int db_findShipmentLast(Shipment *sp)
{
    int ret = -1;
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT); // 读取Courier数据
    if (!fd)
        goto exit;
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    size_t size = sizeof(Shipment);
    ret = len - size;
    file_move(&file, ret);
    file_read(&file, sp, size);
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief 根据快递公司和条码查找快件
 * @param plu       快递公司
 * @param barcode   条码
 * @param sp        找到的订单数据
 * @return int      找到的订单在文件的位置
 */
int db_getShipmentByBarcode(char *plu, char *barcode, Shipment *p)
{
    int ret = -1;
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT); // 读取Courier数据
    if (!fd)
        goto exit;
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    DWORD pos = 0;
    size_t size = sizeof(Shipment);
    file_move(&file, pos);
    while (pos < len)
    {
        file_read(&file, p, size);
        if (strcmp(barcode, p->barcode) == 0)
        {
            ret = pos;
            break;
        }
        pos += size;
    }
exit2:
    file_close(&file);
exit:
    return ret;
}

/**
 * @brief 获取要取件的订单
 * @param pin           取件码
 * @return Shipment*    订单
 */
Shipment *db_getShipmentByPin(char *pin)
{
    Shipment *ret = NULL;
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT); // 读取Courier数据
    if (!fd)
        goto exit;
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    size_t size = sizeof(Shipment);
    DWORD pos = 0;
    file_move(&file, pos);
    ret = mycalloc(1, size);
    while (1)
    {
        if (pos >= len)
        {
            myfree(ret);
            ret = NULL;
            break;
        }
        file_read(&file, ret, size);
        if (strcmp(ret->pin, pin) == 0)
        {
            if ((ret->status & 0xF) == SS_COUDO ||
                (ret->status & 0xF) == SS_EPRA ||
                (ret->status & 0xF) == SS_CUSDO // DEBUG 026
            )
                break;
        }
        pos += size;
    }
exit2:
    file_close(&file);
exit:
    return ret;
}
/**
 * @brief 获取快递公司待取回的订单
 * @param plu       快递公司
 * @param offset    文件查找起始位置
 * @return Shipment*    订单
 */
Shipment *db_getShipmentReturn(char *plu, DWORD *offset)
{
    Shipment *ret = NULL;
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT); // 读取Courier数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = *offset;
    if (len <= pos)
        goto exit2; // 无数据
    size_t size = sizeof(Shipment);
    file_move(&file, pos);
    ret = mycalloc(1, size);
    while (1)
    {
        if (pos >= len)
        {
            myfree(ret);
            ret = NULL;
            break;
        }
        file_read(&file, ret, size);
        if (strcmp(ret->plu, plu) == 0)
        {
            if (((ret->status & 0xF) == SS_CUSDO) ||
                ((ret->status & 0xF) == SS_RETURN))
                break;
        }
        pos += size;
    }
exit2:
    file_close(&file);
    *offset = pos;
exit:
    return ret;
}
/**
 * @brief 获取需要上传到服务器的event
 * @param ev        缓存数据对象
 * @return int      数据位置
 */
int db_getEventPost(Event *p)
{
    int pos = -1;
    FIL file;
    int fd = file_openRead(&file, FN_EVENT); // 读取Event数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    pos = 0;
    file_read(&file, p, sizeof(Event));
exit2:
    file_close(&file);
exit:
    return pos;
}
int db_getTechnicianActionPost(TechnicianAction *p)
{
    int pos = -1;
    FIL file;
    int fd = file_openRead(&file, FN_TECHACTION); // 读取Event数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    size_t size = sizeof(TechnicianAction);
    pos = 0;
    file_read(&file, p, size);
exit2:
    file_close(&file);
exit:
    return pos;
}
int db_getIncidentPost(Incident *p)
{
    int pos = -1;
    FIL file;
    int fd = file_openRead(&file, FN_INCIDENT); // 读取Event数据
    if (!fd)
        goto exit; // 无文件
    DWORD len = file_length(&file);
    if (len == 0)
        goto exit2; // 无数据
    size_t size = sizeof(Incident);
    pos = 0;
    file_read(&file, p, size);
exit2:
    file_close(&file);
exit:
    return pos;
}
char checkPickupPin(char *pin)
{
    char ret = 0;
    Shipment *sp = db_getShipmentByPin(pin);
    if (sp != NULL)
    {
        if (!sp->onlyApp)
        {
            memcpy(shipment, sp, sizeof(Shipment));
            ret = 1;
        }
        myfree(sp);
        ret = 1;
    }
    return ret;
}
char checkPreReservationCustomer(char *barcode)
{
    flagPreReserved = false;
    PreReservation m;
    int pos = db_findPreReservationByBarcode(barcode, &m);
    if (pos >= 0)
    {
        if (m.type == 1)
        {
            memcpy(shipment, &m, 47);
            shipment->onlyApp = m.onlyApp;
            flagPreReserved = true;
        }
    }
    return flagPreReserved;
}
char checkPreReservationCourier(char *barcode)
{
    flagPreReserved = false;
    PreReservation m;
    int pos = db_findPreReservationByBarcode(barcode, &m);
    if (pos >= 0)
    {
        if (m.type == 0)
        {
            memcpy(shipment, &m, 47);
            shipment->onlyApp = m.onlyApp;
            flagPreReserved = true;
        }
    }
    return flagPreReserved;
}
void db_printBoxList(void)
{
    int i;
    printf("\n==BoxList in Db:\r\n");
    for (i = 0; i < boxCount; i++)
    {
        u8 side = 2 - (boxList[i].id / 1000);
        u16 relay = (boxList[i].id % 1000) - 1;
        int index = side * 256 + relay;
        printf("%04d: size-%d,status-%d;", boxList[i].id, 10 + boxFlagList[index].size, boxFlagList[index].status);
    }
}
void db_printShipment(void)
{
    FIL file;
    int fd = file_openRead(&file, FN_SHIPMENT); // 读取Courier数据
    if (!fd)
        return; // 无文件
    DWORD len = file_length(&file);
    size_t size = sizeof(Shipment);
    DWORD pos = 0;
    char *p;
    Shipment m;
    file_move(&file, 0);
    printf("\n==Shipments in Db:\r\n");
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        p = (char *)&m;
        uart2_Send(p, size);
        printf(" BoxID:%d,Status:%d,onlyApp:%d\r\n", m.boxid, m.status, m.onlyApp);
    }
    file_close(&file);
}
void db_printEvent(void)
{
    FIL file;
    int fd = file_openRead(&file, FN_EVENT); // 读取Courier数据
    if (!fd)
        return; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = 0;
    size_t size = sizeof(Event);
    Event m;
    file_move(&file, 0);
    printf("\n==Reservations in Db:\r\n");
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        char text[200] = {0};
        getReservationText(&m, text);
        printf("%s\r\n", text);
    }
    file_close(&file);
}
void db_printIncident(void)
{
    FIL file;
    int fd = file_openRead(&file, FN_INCIDENT); // 读取Courier数据
    if (!fd)
        return; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = 0;
    size_t size = sizeof(Incident);
    Incident m;
    file_move(&file, 0);
    printf("\n==Incidents in Db:\r\n");
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        char text[200] = {0};
        getIncidentText(&m, text);
        printf("%s\r\n", text);
    }
    file_close(&file);
}

void db_printPrereservation(void)
{
    FIL file;
    int fd = file_openRead(&file, FN_PRERESER); // 读取Courier数据
    if (!fd)
        return; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = 0;
    PreReservation m;
    size_t size = sizeof(PreReservation);
    char *p;
    file_move(&file, 0);
    printf("\n==PreReservations in Db:\r\n");
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        p = (char *)&m;
        uart2_Send(p, size);
        printf("type:%s,onlyApp:%d\r\n", (m.type == 0) ? "cou" : "cus", m.onlyApp);
    }
    file_close(&file);
}
void db_printLabelRule(void)
{
    FIL file;
    int fd = file_openRead(&file, FN_LABELRULE); // 读取Courier数据
    if (!fd)
        return; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = 0;
    LabelRule m;
    size_t size = sizeof(LabelRule);
    file_move(&file, 0);
    printf("\n==LabelRules in Db:\r\n");
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        char text[200] = {0};
        getLabelRuleText(&m, text);
        printf("%s\r\n", text);
    }
    file_close(&file);
}
void db_printResShipment(void)
{
    FIL file;
    int fd = file_openRead(&file, FN_RESSHIPMENT); // 读取Courier数据
    if (!fd)
        return; // 无文件
    DWORD len = file_length(&file);
    DWORD pos = 0;
    ResShipment m;
    size_t size = sizeof(ResShipment);
    char *p;
    file_move(&file, 0);
    printf("\n==ResShipment in Db:\r\n");
    while (pos < len)
    {
        pos += file_read(&file, &m, size);
        p = (char *)&m;
        uart2_Send(p, size);
        printf("type:%d,time:%d\r\n", m.type, m.time);
    }
    file_close(&file);
}
void db_print(void)
{
    file_getFree();
    db_printBoxList();
    db_printShipment();
    db_printEvent();
    db_printIncident();
    db_printPrereservation();
    db_printLabelRule();
    db_printResShipment();
}
void db_initBoxList(void)
{
    FIL file;
    int fd = file_openNew(&file, FN_BOX); // 重建BOX文件
    if (!fd)
        return;
    size_t size = sizeof(Box);
    boxCount = 16;
    for (int i = 0; i < boxCount / 2; i++)
    {
        Box m;
        m.id = 1001 + i;
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    for (int i = 0; i < boxCount / 2; i++)
    {
        Box m;
        m.id = 2001 + i;
        fd = file_write(&file, &m, size);
        if (!fd)
            break;
    }
    file_close(&file);
}
void db_init(void)
{
    db_loadBoxFlagList();
}
