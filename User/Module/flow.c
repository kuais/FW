/**
 * @file flow.c
 * @author Kuais
 * @brief   业务流程处理
 * @version 0.1
 * @date 2022-07-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "flow.h"
#include "modDb.h"
#include "modScanner.h"
#include "modKern.h"
#include "modW5500.h"
#include "modRelayBoard.h"
#include "modTime.h"
#include "modBle.h"
#include "modFlash.h"
#include "modE2p.h"
#include "W5500/w5500.h"
#include "KuFrame/kuconvert.h"

bool sys_clear(void)
{ // 清除应用数据
    e2p_Clear();
    return file_init();
}
void checkTimeAlarm(void)
{
    Time t;
    time_now(&t);
    if (appFlag.synclater)
        time_addMinute(&t, 10); // Sync for epra parcel
    else
    {
        if (t.hour >= boardParams.rebootTime)
            time_addDay(&t, 1);
        t.hour = boardParams.rebootTime;
        t.minute = 0;
    }
    t.second = 0;
    time_SetAlarmAt(&t);
}
/**
 * @brief Set the Barcode object
 * 根据不同快递公司的规则 设置扫描仪
 * @param plu       快递公司名称
 * @param type      1 Delivery Carrier, 2 Pickup Client, 3 Delivery Client
 * @return char 0:失败； 1:成功
 */
static char setBarcodeRules(char *plu, int type)
{
    char ret = 0;
    if ((plu == NULL) && (type == SS_COUDO))
        goto exit; // 快递员投递时需要有快递公司
    /* 1. 关闭所有符号集 */
    scanner_AllRuleEnable(0);
    /* 2. 设置相应的符号集*/
    DWORD pos = 0;
    LabelRule rule;
    while (1)
    {
        LabelPlu *lblplu = db_getRuleOfPlu(plu, type, &pos);
        if (!lblplu)
            break;
        LabelRule *r = db_getLabelRule(lblplu->rulename);
        if (r)
        {
            if (strcmp(rule.codetype, r->codetype) != 0)
            { /* 不同类型 */
                if (ret)
                    scanner_RuleEnable(&rule); // 写入上一条规则
                ret = 1;
                strcpy(&rule.codetype, r->codetype);
                rule.pos0 = r->length;
                rule.pos1 = r->length;
            }
            else
            { /* 相同类型，条码长度变更 */
                if (rule.pos0 > r->length)
                    rule.pos0 = r->length;
                if (rule.pos1 < r->length)
                    rule.pos1 = r->length;
            }
            myfree(r);
            ret++;
        }
        myfree(lblplu);
        os_dly_wait(1);
    }
    if (ret)
        scanner_RuleEnable(&rule); // 写入最后一条规则
exit:
    return ret;
}
/**
 * @brief  检查输入的条码是否有效
 * @param plu   快递公司名称
 * @param type  1 Delivery Carrier, 2 Pickup Client, 3 Delivery Client
 * @return char 0:无效 其他:有效
 */
static char checkBarcodeRules(char *plu, int type)
{
    char ret = 0;
    if ((plu == NULL) && (type == BT_COUDO))
        goto exit; // 快递员投递时需要有快递公司
    int len = strlen(strInput);
    DWORD pos = 0;
    while (!ret)
    {
        LabelPlu *lblplu = db_getRuleOfPlu(plu, type, &pos);
        if (!lblplu)
            break;
        LabelRule *rule = db_getLabelRule(lblplu->rulename);
        if (rule)
        {
            ret = (rule->length == len);
            if (ret && (type == BT_CUSDO))
                memcpy(shipment->plu, lblplu->plu, 9);
            myfree(rule);
        }
        myfree(lblplu);
        os_dly_wait(1);
    }
exit:
    return ret;
}
void checkScanner()
{
    int count = 5;
    while (count-- > 0)
    {
        os_dly_wait(10);
        appSet.scanner = scanner_PhoneMode(0);
        if (appSet.scanner)
            break;
    }
}
u16 getBoxIdDropoff(bool isPreReserved, char size)
{
    u16 boxid = 0;
    char curSize = size;
    while (1)
    {
        boxid = getBoxIdToDeposit(shipment->plu, curSize);
        if (boxid > 0)
            break;
        // if (isPreReserved)
        //     break;
        curSize++;
        if (curSize > XL)
        {
            curSize = XS;
            boxid = getBoxIdToDeposit(shipment->plu, curSize);
            break;
        }
    }
    return boxid;
}
void flow_FreeAll(void)
{
    memset(&event, 0, sizeof(Event));
}
void flow_AlarmWakeup(void)
{ // 闹钟唤醒后续处理
    tickCheck = 0;
    if (appFlag.synclater)
        curFlow = SyncEpraFlow;
    else
    {
        bsp_Restart();
        curFlow = IdleFlow;
    }
}
void flow_AppSwitch(void)
{
    flash_setAppFlag(1 - flash_getAppFlag());
    bsp_Restart();
    curFlow = IdleFlow;
}
void flow_ClearData(void)
{
    sys_clear();
    bsp_Restart();
    curFlow = IdleFlow;
}
void flow_PowerSave(void)
{
    // 进入省电模式
    powerOff_Scanner;
    if (appSet.router)
    {
        appSet.router = 0;
        powerOff_Route;
    }
    if (appSet.relayboard)
    {
        appSet.relayboard = 0;
        powerOff_RelayBoard;
    }
    // 进入省电模式
    powerOff_Key1; // 全按键触发外部中断
    powerOff_Key2;
    powerOff_Key3;
    powerOff_W5500;
    /* 1. 断开蓝牙连接 */
    ble_Start();
    os_dly_wait(1000);
    while (!bleFlag.ready)
        os_dly_wait(10);
    //    w5500_SetOPMD(1);
    //    w5500_LowPower(true);
    checkTimeAlarm();
    printLog("Enter Power Save mode");
    powerOff_Led1;
    SVC_1_FunCall(); // 进入特权级省电
    printLog("Leave Power Save mode");
    power_Resume();
    checkScanner();
    inputOn(); // 允许输入
}
/**
 * @brief   同步所有内容时工作流程
 * @return char 下一个流程
 */
void flow_SyncAll(void)
{
    if (w5500_CheckConnect())
    { /* 网络连通，同步数据 */
        w5500_CheckUpdate();
        boardParams.debug = NETDEBUG;
    }
    if (appSet.network)
        w5500_SyncDown();
    tickCheck = 0;
    db_loadBoxList();
    boardParams.debug = NETDEBUG;
    powerOn_Scanner; // 扫描头上电
    // scanner_AllRuleEnable(0);
    checkScanner();
    if (appSet.scanner)
        setBarcodeRules(NULL, BT_CUSPU); // 设置扫描模式为 3.用户存件
    if (appFlag.firstsync)
    {
        appFlag.firstsync = 0;
        /* BLEID, BLENAME, BLEKEY, IPADDRESS, SUBNETMASK, GATEWAY */
        char text[LEN_INCIDENTINFO] = {0};
        char bleID[11] = {0};
        char bleName[33] = {0};
        char bleKey[33] = {0};
        e2p_GetBleID(bleID);
        e2p_GetBleName(bleName);
        int n = 0;
        for (int i = 0; i < 16; i++)
        {
            bleKey[n++] = toHex(boardParams.aesKey[i] >> 4);
            bleKey[n++] = toHex(boardParams.aesKey[i] & 0xF);
        }
        sprintf(text, "%s,%s,%s,%d.%d.%d.%d,%d.%d.%d.%d,%d.%d.%d.%d",
                bleID, bleName, bleKey,
                w5500cfg.lip[0], w5500cfg.lip[1], w5500cfg.lip[2], w5500cfg.lip[3],
                w5500cfg.sub[0], w5500cfg.sub[1], w5500cfg.sub[2], w5500cfg.sub[3],
                w5500cfg.gw[0], w5500cfg.gw[1], w5500cfg.gw[2], w5500cfg.gw[3]);
        printLog(text);
    }
    inputOn();
    curFlow = IdleFlow; // 进入待机模式
}
void flow_SyncEpra(void)
{
    printLog("Sync EPRA Begin");
    powerOn_Route;
    appSet.router = 1;
    w5500_Init(); // powerOn_W5500;
    while (1)
    {
        if (curFlow != SyncEpraFlow)
            return;
        if (tickCheck >= 60)
            goto error; // 60秒无数据交互
        if (!w5500_CheckConnect())
            goto error; // 网络连接断开
        if (w5500_GetToken())
        { // 检查并更新TOKEN
            if (((kernSession->errCount + 1) % 3) == 0)
                w5500_Init();
            if (kernSession->errCount >= 60)
                goto error; // 路由器超时未启动
            else
                os_dly_wait(500);
        }
        tickCheck = 0;
        if (kernSession->errCount >= 3)
            kern_ClearSession();
        else if (kernSession->step <= 7)
            w5500_UploadIncident();
        else if (kernSession->step <= 31)
            w5500_GetPrereservation();
        else if (kernSession->step <= 32)
            w5500_GetReservation();
        else
            goto exit;
    }
error:
    kern_NewSession();
exit:
    powerOff_Route;
    kernSession->step = 0;
    appFlag.synclater = 0;
    curFlow = PowerSaveFlow;
}

/**
 * @brief   待机时工作流程
 * @return char 下一个流程
 */
void flow_Idle(void)
{
    if (flagInput == 1)
    {
        inputOn(); // 允许输入
    }
    else if (flagInput == 2)
    {
        if (strlen(strInput) > 0)
        { // 停止输入5秒后，到LoginFlow检查
            if (tickCheck >= 5)
                curFlow = LoginFlow;
        }
        else if (tickCheck >= INPUT_EXIT_TICKCOUNT)
        {
            inputOff(); // 无输入10秒后关闭输入
        }
    }
    else if (flagInput == 4)
    {
        if (!bleFlag.connect || tickCheck >= INPUT_EXIT_TICKCOUNT)
        {
            inputOff(); // 无输入10秒后返回关闭输入
            printLog("BLE Handler End");
        }
    }
    else if (appSet.router)
    { // 路由器有通电, 上传Reservation,等
        if ((tickCheck >= 60) || (!appSet.network))
        {
            appSet.router = 0;
            powerOff_Route;
            kern_NewSession();
            return;
        }
        if (!w5500_CheckConnect())
        {
            if (curFlow == IdleFlow)
            {
                appSet.router = 0;
                powerOff_Route;
                kern_NewSession();
                tickCheck = 0;
            }
            return;
        }
        if (w5500_GetToken())
        { // 检查并更新TOKEN
            if (((kernSession->errCount + 1) % 3) == 0)
                w5500_Init();
            if (kernSession->errCount >= 60)
            { // 失败，进入省电
                kernSession->errCount = 0;
                appSet.router = 0;
                powerOff_Route;
            }
            else
            {
                os_dly_wait(500);
            }
            return;
        }
        tickCheck = 0;
        if (kernSession->errCount >= 3)
            kern_ClearSession();
        else if (kernSession->step <= 4)
            w5500_UploadLockStatus();
        else if (kernSession->step <= 5)
            w5500_UploadSensorStatus();
        else if (kernSession->step <= 6)
        {
            if (appSet.relayboard)
            {
                appSet.relayboard = 0;
                powerOff_RelayBoard;
            }
            w5500_UploadReservation();
        }
        else if (kernSession->step <= 7)
            w5500_UploadIncident();
        // else if (kernSession->step <= 8)
        //     w5500_UploadTechnicianAction();
        // else if (kernSession->step <= 21)
        //     w5500_SyncConf();
        else if (kernSession->step <= 31)
            w5500_GetPrereservation();
        else if (kernSession->step <= 32)
            w5500_GetReservation();
        else
        { // 无数据上传, 路由器断电，进入省电
            appSet.router = 0;
            powerOff_Route;
            kernSession->step = 0;
        }
    }
    else if (tickCheck >= POWER_SAVE_TICKCOUNT)
    { // 待机20秒后进入省电
        curFlow = PowerSaveFlow;
    }
}

void nextSize(void)
{
    if (shipment->size < XL)
        shipment->size++;
}
void checkIsNew(char status)
{
    if (shipment->status != status)
        flagNewBarcode = true; // 上一次操作不是投递,是新订单
    else if (strlen(shipment->barcode) == 0)
        flagNewBarcode = true; // 上一次条码为空,是新订单
    else if (strcmp(strInput, shipment->barcode) != 0)
        flagNewBarcode = true; // 条码与上次投递的不一致,是新订单
    else
        flagNewBarcode = false;
}
bool checkBarcode(char status)
{
    db_newIncident(IT_BarcodeRead, 0, strInput);
    char strTemp[LEN_INCIDENTINFO] = {0};
    sprintf(strTemp, "Check Barcode %s", strInput);
    printLog(strTemp);
    flagNewBarcode = true;
    shipment->onlyApp = false;
    if (status == SS_COUDO)
    {
        if (!checkPreReservationCourier(strInput)) // 检查是否是预订
        {                                          // 不是预定
            if (!db_getFlagOfPlu(shipment->plu))   // 检查plu标志
            {
                sprintf(strTemp, "Plu-%s not allow hotdrop", shipment->plu);
                printLog(strTemp);
                return false; // 不允许热投递
            }
            if (!checkBarcodeRules(shipment->plu, BT_COUDO)) // 比对规则
            {
                sprintf(strTemp, "LabelRule unmatched for type-%d, plu-%s", shipment->plu, BT_COUDO);
                printLog(strTemp);
                return false; // 条码不符合规则
            }
        }
    }
    else
    {
        if (!checkPreReservationCustomer(strInput)) // 检查是否是预订
        {                                           // 不是预定
            if (!checkBarcodeRules(NULL, BT_CUSDO)) // 比对规则，同时设置相应的Plu
            {
                sprintf(strTemp, "LabelRule unmatched for type-%d", BT_CUSDO);
                printLog(strTemp);
                return false; // 条码不符合规则
            }
            if (!db_getFlagOfPlu(shipment->plu)) // 检查plu标志
            {
                sprintf(strTemp, "Plu-%s not allow hotdrop", shipment->plu);
                printLog(strTemp);
                return false; // 不允许热投递
            }
        }
    }
    if (!flagPreReserved)
        checkIsNew(status);
    Shipment sp;
    int pos = db_getShipmentByBarcode(shipment->plu, strInput, &sp);
    if (pos >= 0)
    { // 订单已存在
        if (flagNewBarcode)
        {
            sprintf(strTemp, "Barcode %s is existed", strInput);
            printLog(strTemp);
            return false;
        }
        else
        { // 删除订单
            db_delShipmentByPos(pos);
            if (status == SS_COUDO)
            {
                event.eventType = RE_CourierPickup;
                sprintf(strTemp, "Courier %s pickup %s at box-%d", event.user, shipment->barcode, shipment->boxid);
                printLog(strTemp);
            }
            else
            {
                event.eventType = RE_CustomerPickup;
                sprintf(strTemp, "Customer pickup %s at box-%d", shipment->barcode, shipment->boxid);
                printLog(strTemp);
            }
            db_addEvent(&event);
            u16 relay, index;
            u8 side;
            side = 2 - (sp.boxid / 1000);
            relay = (sp.boxid % 1000) - 1;
            index = side * 256 + relay;
            boxFlagList[index].status = 0;
            db_saveBoxFlag(&boxFlagList[index], index);
        }
    }
    if (!flagPreReserved)
    {
        shipment->size = 0;                  // 从最小格口开始遍历
        strcpy(shipment->barcode, strInput); // 更新条码
        // if (!flagNewBarcode)
        //     nextSize();
    }
    return true;
}

void ChangeBox(void)
{
    /* 删除 Shipment*/
    u8 side = 2 - (shipment->boxid / 1000);
    u8 relay = (shipment->boxid % 1000) - 1;
    int index = side * 256 + relay;
    boxFlagList[index].status = 0;
    db_saveBoxFlag(&boxFlagList[index], index);
    db_delShipment(shipment);
    event.eventType = RE_CustomerPickup;
    db_addEvent(&event);
    char strTemp[LEN_INCIDENTINFO];
    sprintf(strTemp, "Ble pickup %s at box-%d, type-%d", shipment->pin, shipment->boxid, bleEventType);
    printLog(strTemp);
    /* 格口尺寸增加，重新投递 */
    bleEventType = Customer_DropOff_Hot;
    if (shipment->size < XL)
    { // 已是最大格口无法再增加
        shipment->size++;
    }
    shipment->boxid = getBoxIdDropoff(false, shipment->size);
}
bool Customer_BoxChange(Shipment *sp)
{
    Time t;
    time_now(&t);
    U32 time = timeToU32(&t);
    db_delResShipmentByTime(time);
    ResShipment m;
    DWORD pos = db_getResShipment(&m, 0, sp->barcode);
    if (pos == 0)
        return false;
    memcpy(shipment, sp, sizeof(Shipment));
    /* 删除 ResShipment*/
    db_delResShipmentByPos(pos - sizeof(ResShipment));
    ChangeBox();
    return true;
}
bool Customer_ReDeposit(Shipment *sp)
{
    Time t;
    time_now(&t);
    U32 time = timeToU32(&t);
    db_delResShipmentByTime(time);
    ResShipment m;
    DWORD pos = db_getResShipment(&m, 0, sp->barcode);
    if (pos == 0)
        return false;
    u8 side = m.boxIndex / 256;
    u8 relay = m.boxIndex % 256;
    u16 index = side * 256 + relay;
    unsigned char datas[1] = {0};
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(2000);
    }
    bool flag = relayBoard_GetSensorStatus(side, relay) == 0; // true:有物，false:无物
    if (!flag)
    { // 检测无物，更换格口
        /* 删除 ResShipment*/
        db_delResShipmentByPos(pos - sizeof(ResShipment));
        ChangeBox();
        if (shipment->boxid == 0)
        {
            datas[0] = 2; // 无格口分配
            ble_SendStatus(1, datas, 1);
            return true;
        }
        side = 2 - (shipment->boxid / 1000);
        relay = (shipment->boxid % 1000) - 1;
        index = side * 256 + relay;
    }
    char step = 0;
    while (1)
    {
        os_dly_wait(1);
        switch (step)
        { // 开启格口
        case 0:
        {
            relayBoard_Unlock(side, relay);
            tickCheck = 0;
            step = 1;
            break;
        }
        case 1:
        { // 检查格口状态
            if (tickCheck >= DOOR_OPEN_TICKCOUNT)
            { // 门未打开
                db_newIncident(IT_DoornotOpen, sp->boxid, "");
                step = 92;
            }
            // else if (relayBoard_GetLockStatus(side, relay) == 1) // Debug Code
            else if (relayBoard_GetLockStatus(side, relay) == 0)
            { // 门已打开
                os_dly_wait(500);
                step = 91;
            }
            break;
        }
        case 91:
        {
            if (!flag)
            {
                ResShipment resSp;
                resSp.type = 0;
                strcpy(resSp.barcode, shipment->barcode);
                strcpy(resSp.pin, shipment->pin);
                resSp.boxIndex = index;
                db_addResShipment(&resSp);
                db_addShipment(shipment);
            }
            event.eventType = RE_CustomerDeposited;
            db_addEvent(&event);
            boxFlagList[index].status = 1;
            db_saveBoxFlag(&boxFlagList[index], index);
            ble_SendStatus(0, datas, 0);
            char strTemp[LEN_INCIDENTINFO];
            if (flag)
                sprintf(strTemp, "Ble reopen %s at box-%d", sp->pin, sp->boxid);
            else
                sprintf(strTemp, "Ble dropoff %s at box-%d, type-%d:", shipment->pin, shipment->boxid, bleEventType);
            printLog(strTemp);
            step = 99;
            break;
        }
        case 92:
        {
            datas[0] = 2; // 无格口分配
            ble_SendStatus(1, datas, 1);
            step = 99;
            break;
        }
        case 99:
            return true;
        }
    }
}
bool Customer_ReCollect(Shipment *sp)
{
    Time t;
    time_now(&t);
    U32 time = timeToU32(&t);
    db_delResShipmentByTime(time);
    ResShipment m;
    if (db_getResShipment(&m, 1, sp->pin) == 0)
        return false;
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(2000);
    }
    u8 side = m.boxIndex / 256;
    u8 relay = m.boxIndex % 256;
    sp->boxid = getBoxID(side, relay);
    char step = 0;
    unsigned char datas[1] = {0};
    while (1)
    {
        os_dly_wait(1);
        switch (step)
        { // 开启格口
        case 0:
        {
            relayBoard_Unlock(side, relay);
            tickCheck = 0;
            step = 1;
            break;
        }
        case 1:
        { // 检查格口状态
            if (tickCheck >= DOOR_OPEN_TICKCOUNT)
            { // 门未打开
                db_newIncident(IT_DoornotOpen, sp->boxid, "");
                step = 92;
            }
            // else if (relayBoard_GetLockStatus(side, relay) == 1) // Debug Code
            else if (relayBoard_GetLockStatus(side, relay) == 0)
            { // 门已打开
                os_dly_wait(500);
                step = 91;
            }
            break;
        }
        case 91:
        {
            event.eventType = RE_CustomerPickup;
            db_addEvent(&event);
            ble_SendStatus(0, datas, 0);
            step = 99;
            break;
        }
        case 92:
        {
            datas[0] = 2; // 无格口分配
            ble_SendStatus(1, datas, 1);
            step = 99;
            break;
        }
        case 99:
        {
            char strTemp[LEN_INCIDENTINFO];
            sprintf(strTemp, "Customer reopen %s at box-%d", sp->pin, sp->boxid);
            printLog(strTemp);
            return true;
        }
        }
    }
}

/**
 * @brief
 *
 * @param id
 */
void flow_Login(void)
{
    char strTemp[LEN_INCIDENTINFO];
    sprintf(strTemp, "Check Login: %s", strInput);
    printLog(strTemp);
    char loginStep = 1;
    inputOff();
    memset(event.user, 0, sizeof(event.user)); // 清除之前登录的账号
    memset(shipment->plu, 0, sizeof(shipment->plu));

    while (1)
    {
        os_dly_wait(1);
        switch (loginStep)
        {
        case 1: // 3.1.Match courier id -> Courier Pickup
        {
            if (db_getPluOfCourier(strInput))
            {
                db_newIncident(IT_CourierLogin, 0, strInput);
                strncpy(event.user, strInput, 13);
                curFlow = CourierPickupFLow;
                return;
            }
            break;
        }
        case 2: // 3.2 Match pin -> Customer Pickup
        {
            if (checkPickupPin(strInput))
            {
                if (shipment->onlyApp)
                    break;
                curFlow = CustomerPickupFlow;
                return;
            }
            else
            {
                memcpy(shipment->pin, strInput, 6);
                if (Customer_ReCollect(shipment))
                {
                    if (appSet.relayboard)
                    {
                        appSet.relayboard = 0;
                        powerOff_RelayBoard;
                    }
                    flow_FreeAll();
                    curFlow = IdleFlow;
                    return;
                }
            }
            break;
        }
        case 3: // 3.3.Match Technician id -> Technician Unlock
        {
            char ret = db_checkTechnician(strInput);
            if (ret > 0)
            {
                db_newTechnicianAction(1, strInput);
                if (ret == 1)
                    curFlow = SuperTechnicianFlow; // 超级管理员 开启所有锁
                else if (ret == 2)
                    curFlow = TechnicianFlow; // 管理员 开启空闲锁
                return;
            }
            break;
        }
        case 4: // 3.4.Match Barcode with Labelrule -> Customer Drop off
        {       /* v0.1.3  Only allow customer drop-off via app */
            // if (checkBarcode(SS_CUSDO))
            // {
            //     curFlow = CustomerDropoffFlow; // 用户投递
            //     return;
            // }
            break;
        }
        default:
        {
			sprintf(strTemp, "Check Login Failed - %s", strInput);
            printLog(strTemp);
            db_newIncident(IT_CourierLoginFail, 0, strInput);
            if (appSet.scanner)
                scanner_Blink(5, 100);
            // 返回待机 等待用户输入
            inputOn();
            curFlow = IdleFlow;
            return;
        }
        }
        loginStep++;
    }
}
static void pickUpPlu(char *plu)
{
    char step = 0; // Pickup Step
    char retryCount = 0;
    int pos = 0;
    u16 relay, index;
    u8 side;

    char strTemp[LEN_INCIDENTINFO];
    Shipment *sp = NULL;
    while (1)
    {
        os_dly_wait(1);
        switch (step)
        {
        case 0:
        {
            sp = db_getShipmentReturn(plu, &pos);
            if (!sp)
                return; // Exit
            else
            {
                step = 1;
                side = 2 - (sp->boxid / 1000);
                relay = (sp->boxid % 1000) - 1;
                index = side * 256 + relay;
                retryCount = 0;
            }
            break;
        }
        case 1:
        {
            relayBoard_Unlock(side, relay);
            // if (relayBoard_watiLockOpen(side, relay) == 1) // Debug Code
            if (relayBoard_watiLockOpen(side, relay) == 0)
            { // 门已打开
                step = 11;
                flagInput = 3;
                strInput[0] = 0;
            }
            else // 门未打开,开启下一个
            {
                db_newIncident(IT_DoornotOpen, sp->boxid, "");
                sprintf(strTemp, "Courier %s pickup %s at box-%d Failed", event.user, sp->barcode, sp->boxid);
                printLog(strTemp);
                pos += sizeof(Shipment);
                step = 0;
            }
            break;
        }
        case 11:
        { // 开门成功，在30秒内检测关门
            if (tickCheck >= DOOR_CLOSE_TICKCOUNT)
            { // 30秒未关门, 默认成功
                db_newIncident(IT_DoornotClose, sp->boxid, "");
                step = 91;
            }
            else if (relayBoard_GetLockStatus(side, relay) == 1)
            { // 门已关
                if (relayBoard_GetSensorStatus(side, relay) == 1)
                { // 无物，取件成功
                    step = 91;
                }
                else
                { // 有物
                    retryCount++;
                    if (retryCount == 2)
                        step = 91;
                    else
                        step = 1;
                }
            }
            else if (flagInput == 0)
            {
                if (strInput[0] == '0')
                    step = 93;
                else
                    flagInput = 3;
                strInput[0] = 0;
            }
            break;
        }
        case 91:
        { // 操作成功
            memcpy(shipment, sp, sizeof(Shipment));
            myfree(sp);
            db_delShipment(shipment);
            event.eventType = RE_CourierPickup;
            db_addEvent(&event);
            shipment->status = SS_Normal;
            boxFlagList[index].status = 0;
            db_saveBoxFlag(&boxFlagList[index], index);
            sprintf(strTemp, "Courier %s pickup %s at box-%d Successfully", event.user, shipment->barcode, shipment->boxid);
            printLog(strTemp);
            flagInput = 0;
            step = 0;
            break;
        }
        case 93:
        {
            memcpy(shipment, sp, sizeof(Shipment));
            myfree(sp);
            db_delShipment(shipment);
            event.eventType = RE_NotCollectedCourier;
            db_addEvent(&event);
            shipment->status = SS_Normal;
            boxFlagList[index].status = 0;
            db_saveBoxFlag(&boxFlagList[index], index);
            sprintf(strTemp, "Courier %s pickup %s at box-%d, Parcel Missed!", event.user, shipment->barcode, shipment->boxid);
            printLog(strTemp);
            flagInput = 0;
            step = 0;
            break;
        }
        default:
            break;
        }
    }
}
void flow_CourierPickup(void)
{ // TODO 获取

    if (!appSet.router)
    { // 快递员流程里开启路由器
        appSet.router = 1;
        powerOn_Route;
    }
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(1000);
    }
    kernSession->step = 0;
    printLog("Courier Pickup Begin");
    if (appSet.scanner)
        scanner_Blink(2, 200);
    pickUpPlu(shipment->plu);
    printLog("Courier Pickup End");
    curFlow = CourierDropoffFlow;
}
void flow_CourierDropoff(void)
{ // 快递员投递流程
    if (flagInput == 0)
    {
        if (!setBarcodeRules(shipment->plu, BT_COUDO)) // 设置扫描模式为 1.快递员存件
            goto exit;                                 // 无规则匹配,退出投递
        if (appSet.scanner)
            scanner_Blink(2, 200);
        printLog("Courier Dropoff Begin");
        //        preFlow = CourierDropoffFlow;
        inputOn();
    }
    else if (flagInput == 2)
    {
        if (strlen(strInput) > 0)
        { // 停止输入5秒后，认为输入完毕，开始实际投递
            if (tickCheck >= 5)
            {
                curFlow = DropoffFlow;
                return;
            }
        }
        if (tickCheck >= INPUT_EXIT_TICKCOUNT)
        {
            printLog("Courier Dropoff End");
            goto exit;
        }
    }
    return;
exit: // 退出投递
    // powerOff_RelayBoard;
    inputOff();
    // scanner_AllRuleEnable(1);
    setBarcodeRules(NULL, BT_CUSPU); // 设置扫描模式为 3.用户存件
    flow_FreeAll();
    shipment->status = SS_Normal;
    printLog("Courier Flow Exit");
    tickCheck = 0;
    curFlow = SyncFlow;
    // curFlow = IdleFlow;
}
static void dropoff(char status)
{
    char step = 0; // DropOff Step
    char errCount = 1;
    u16 relay, index;
    u8 side;
    char strTemp[LEN_INCIDENTINFO];

    clearInput();
    shipment->status = status;
    while (1)
    {
        os_dly_wait(1);
        switch (step)
        {
        case 0:
        { // 分配格口
            shipment->boxid = getBoxIdDropoff(false, shipment->size);
            if (shipment->boxid == 0)
            {
                sprintf(strTemp, "No idle boxes for size %d", shipment->size);
                printLog(strTemp);
                step = 92; // 无可用格口
            }
            else
            {
                side = 2 - (shipment->boxid / 1000);
                relay = (shipment->boxid % 1000) - 1;
                index = side * 256 + relay;
                boxFlagList[index].used = boxFlagList[index].used ? 0 : 1;
                step = 1;
            }
            break;
        }
        case 1:
        { // 开启格口
            relayBoard_Unlock(side, relay);
            // if (relayBoard_watiLockOpen(side, relay) == 1) // Debug Code
            if (relayBoard_watiLockOpen(side, relay) == 0)
            { // 门已打开
                step = 11;
                flagInput = 3;
                strInput[0] = 0;
            }
            else
            { // 门未打开
                db_newIncident(IT_DoornotOpen, shipment->boxid, "");
                if (errCount == 0)
                    step = 92;
                else
                {
                    errCount--;
                    step = 0;
                }
            }
            tickCheck = 0;
            break;
        }
        case 11:
        { // 开门成功，在30秒内检测关门
            if (tickCheck >= DOOR_CLOSE_TICKCOUNT)
            { // 30秒未关门
                step = 93;
            }
            else if (relayBoard_GetLockStatus(side, relay) == 1)
            { // 门已关
                if (relayBoard_GetSensorStatus(side, relay) == 0)
                { // 检测有物，投递成功
                    step = 91;
                }
                else
                { // 无物
                    step = 31;
                    flagInput = 3;
                    strInput[0] = 0;
                }
            }
            else if (flagInput == 0)
            {
                if (strInput[0] == '0')
                    step = 94; // 按下0
                else
                    flagInput = 3;
                strInput[0] = 0;
            }
            break;
        }
        case 31:
        { // 更换格口
            if (tickCheck >= DOOR_CANCEL_TICKCOUNT)
            { // 10秒未收到指令, 默认认为投递成功
                step = 91;
            }
            else if (flagInput == 0)
            {
                if (strInput[0] == 0xA)
                    step = 91; // 按下Clear,  认为是投递成功
                else if (strInput[0] == '0')
                    step = 94; // 按下0， 取消投递
                else
                {
                    shipment->size = strInput[0] - 0x31;
                    step = 0;
                }
                strInput[0] = 0;
            }
            break;
        }
        case 91:
        { // 投递成功
            if (flagPreReserved)
                db_delPreReservation(shipment->barcode);
            else
            {
                size_t len;
                randomDigits(shipment->pin, 6, &len);
                shipment->pin[6] = 0;
            }
            db_addShipment(&event.shipment);
            if (status == SS_COUDO)
            {
                event.eventType = RE_CourierDeposited;
                sprintf(strTemp, "Courier %s dropoff %s at box-%d:%s", event.user, shipment->barcode, shipment->boxid, shipment->pin);
                printLog(strTemp);
            }
            else
            {
                event.eventType = RE_CustomerDeposited;
                sprintf(strTemp, "Customer dropoff %s at box-%d:%s", shipment->barcode, shipment->boxid, shipment->pin);
                printLog(strTemp);
            }
            db_addEvent(&event);
            boxFlagList[index].status = 1;
            db_saveBoxFlag(&boxFlagList[index], index);
            step = 99;
            break;
        }
        case 92:
        { // 投递失败
            printLog("Dropoff Failed.");
            step = 98;
            break;
        }
        case 93:
        { // 投递超时
            db_newIncident(IT_DoornotClose, shipment->boxid, "");
            printLog("Dropoff Timeout.");
            step = 98;
            break;
        }
        case 94:
        { // 投递取消
            db_newIncident((status == SS_COUDO) ? IT_AbortCourierDropoff : IT_AbortCustomerDropoff,
                           shipment->boxid, shipment->barcode);
            printLog("Dropoff Cancel.");
            step = 98;
            break;
        }
        case 98:
        { // 投递失败处理
            if (appSet.scanner)
                scanner_Blink(5, 100);
            inputOn();
            break;
        }
        case 99:
        { // 投递成功处理
            inputOn();
            return;
        }
        }
    }
}
/**
 * @brief 快递员投递
 */
void flow_Dropoff(void)
{
    inputOff();
    if (checkBarcode(SS_COUDO))
        dropoff(SS_COUDO);
    else
    {
        printLog("BarCode Not Valid");
        if (appSet.scanner)
            scanner_Blink(5, 100);
        inputOn();
    }
    curFlow = CourierDropoffFlow;
}
void flow_CustomerDropoff(void)
{
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(1000);
    }
    printLog("Customer Dropoff Begin");
    dropoff(SS_CUSDO);

    printLog("Customer Dropoff End");
    if (appSet.relayboard)
    {
        appSet.relayboard = 0;
        powerOff_RelayBoard;
    }
    flow_FreeAll();
    shipment->status = SS_Normal;
    curFlow = IdleFlow;
}

void flow_CustomerPickup(void)
{
    char step = 0; // PickUp Step
    u16 relay, index;
    u8 side;
    char strTemp[LEN_INCIDENTINFO];

    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(1000);
    }
    while (1)
    {
        os_dly_wait(1);
        switch (step)
        { // 开启格口
        case 0:
        {
            side = 2 - (shipment->boxid / 1000);
            relay = (shipment->boxid % 1000) - 1;
            index = side * 256 + relay;
            relayBoard_Unlock(side, relay);
            // if (relayBoard_watiLockOpen(side, relay) == 1) // Debug Code
            if (relayBoard_watiLockOpen(side, relay) == 0)
            { // 门已打开
                step = 91;
            }
            else
            { // 门未打开
                db_newIncident(IT_DoornotOpen, shipment->boxid, "");
                sprintf(strTemp, "Customer pickup %s at box-%d Failed", shipment->barcode, shipment->boxid);
                printLog(strTemp);
                step = 99;
            }
            break;
        }
        case 91:
        { // 用户取件成功
            ResShipment resSp;
            resSp.type = 1;
            strcpy(resSp.barcode, shipment->barcode);
            strcpy(resSp.pin, shipment->pin);
            resSp.boxIndex = index;
            db_addResShipment(&resSp);
            // shipment->status = SS_CUSPU;
            // db_setShipment(shipment);
            db_delShipment(shipment);
            event.eventType = RE_CustomerPickup;
            db_addEvent(&event);
            shipment->status = SS_Normal;
            boxFlagList[index].status = 0;
            db_saveBoxFlag(&boxFlagList[index], index);
            sprintf(strTemp, "Customer pickup %s at box-%d Successfully", shipment->barcode, shipment->boxid);
            printLog(strTemp);
            // bsp_BeepSome(2, 100);
            step = 99;
            break;
        }
        default:
        { // 返回继续扫码
            if (appSet.relayboard)
            {
                appSet.relayboard = 0;
                powerOff_RelayBoard;
            }
            flow_FreeAll();
            curFlow = IdleFlow;
            return;
        }
        }
    }
}

void flow_SuperTechnician(void)
{ // 超级管理员开启所有格口
    printLog("SuperTechnician Flow Begin");
    int i;
    char side, relay;
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(1000);
    }
    for (i = 0; i < boxCount; i++)
    {
        side = 2 - (boxList[i].id / 1000);
        relay = (boxList[i].id % 1000) - 1;
        relayBoard_Unlock(side, relay);
        os_dly_wait(1000);
    }
    flow_FreeAll();
    // 返回待机 等待用户输入
    tickCheck = 0;
    printLog("SuperTechnician Flow End");
    curFlow = IdleFlow;
}

void flow_TechnicianFlow(void)
{ // 普通管理员只开启空闲格口
    printLog("Technician Flow Begin");
    int i;
    char side, relay;
    u16 index;
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(1000);
    }
    for (i = 0; i < boxCount; i++)
    {
        side = 2 - (boxList[i].id / 1000);
        relay = (boxList[i].id % 1000) - 1;
        index = side * 256 + relay;
        if (boxFlagList[index].status == 0)
        {
            relayBoard_Unlock(side, relay);
            os_dly_wait(1000);
        }
    }
    flow_FreeAll();
    // 返回待机 等待用户输入
    printLog("Technician Flow End");
    tickCheck = 0;
    curFlow = IdleFlow;
}

void flow_BleOpenLock(void)
{
    char step = 0; //
    char side, relay;
    u16 index;
    unsigned char datas[1] = {0};
    memset(event.user, 0, sizeof(event.user));
    memset(shipment->plu, 0, sizeof(shipment->plu));
    char strTemp[LEN_INCIDENTINFO];

    sprintf(strTemp, "BleUnlock Pin:%s,Type:%d", shipment->pin, bleEventType);
    printLog(strTemp);
    if (bleEventType == Box_Change)
    {
        Shipment sp;
        int pos = db_getShipmentByBarcode("", shipment->barcode, &sp);
        if (pos < 0)
        {
            sprintf(strTemp, "Barcode %s is not existed", sp.barcode);
            printLog(strTemp);
            datas[0] = 2; // Pin/OrderId not found: 0x01
            ble_SendStatus(1, datas, 1);
        }
        else if (!Customer_BoxChange(&sp))
        {
            printLog("Exceed 3 minutes");
            datas[0] = 2;
            ble_SendStatus(1, datas, 1);
        }
        else
        {
            if (shipment->boxid == 0)
            {
                datas[0] = 2; // 无格口分配
                ble_SendStatus(1, datas, 1);
                goto exit;
            }
            side = 2 - (shipment->boxid / 1000);
            relay = (shipment->boxid % 1000) - 1;
            index = side * 256 + relay;
            boxFlagList[index].used = 1 - boxFlagList[index].used;
            shipment->size = boxFlagList[index].size;
            shipment->status = SS_Normal;
        }
    }
    else if (bleEventType == Courier_PickUp || bleEventType == Customer_PickUp)
    { // Pick Up
        Shipment *sp = db_getShipmentByPin(shipment->pin);
        if (sp == NULL)
        {
            if ((bleEventType == Customer_PickUp) && Customer_ReCollect(shipment))
            { // 3分钟内允许重复取件
                goto exit;
            }
            else
            {
                printLog("Pin not found");
                datas[0] = 1; // Pin/OrderId not found: 0x01
                ble_SendStatus(1, datas, 1);
            }
            goto exit;
        }
        else
        {
            memcpy(shipment, sp, sizeof(Shipment));
            myfree(sp);
            side = 2 - (shipment->boxid / 1000);
            relay = (shipment->boxid % 1000) - 1;
            index = side * 256 + relay;
        }
    }
    else
    { // Drop Off
        Shipment sp;
        int pos = db_getShipmentByBarcode("", shipment->barcode, &sp);
        if (pos >= 0)
        {
            if ((bleEventType == Customer_DropOff_Hot) || (bleEventType == Customer_DropOff))
            { // 3分钟内允许重复投递
                memcpy(shipment, &sp, sizeof(Shipment));
                if (Customer_ReDeposit(shipment))
                    goto exit;
            }
            sprintf(strTemp, "Barcode %s is existed", sp.barcode);
            printLog(strTemp);
            datas[0] = 2; // 存在相同订单
            ble_SendStatus(1, datas, 1);
            goto exit;
        }
        if (bleEventType == Customer_DropOff_Hot)
        {
            memcpy(strInput, shipment->barcode, 7);
            if (!checkBarcodeRules(NULL, BT_CUSDO)) // 比对规则，同时设置相应的Plu
            {
                printLog("BarCode Not Valid");
                datas[0] = 2; // 条码不符合规则
                ble_SendStatus(1, datas, 1);
                goto exit; // 条码不符合规则
            }
            printf("Plu:%s.\r\n", shipment->plu);
        }
        if (bleEventType == Courier_DropOff_Hot || bleEventType == Customer_DropOff_Hot)
        { // Hot dropoff 需要自动分配格口
            shipment->boxid = getBoxIdDropoff(false, shipment->size);
            if (shipment->boxid == 0)
            {
                datas[0] = 2; // 无格口分配
                ble_SendStatus(1, datas, 1);
                goto exit;
            }
        }
        side = 2 - (shipment->boxid / 1000);
        relay = (shipment->boxid % 1000) - 1;
        index = side * 256 + relay;
        boxFlagList[index].used = 1 - boxFlagList[index].used;
        shipment->size = boxFlagList[index].size;
        shipment->status = SS_Normal;
    }
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard;
        os_dly_wait(2000);
    }
    while (1)
    {
        os_dly_wait(1);
        switch (step)
        { // 开启格口
        case 0:
        {
            relayBoard_Unlock(side, relay);
            tickCheck = 0;
            step = 1;
            break;
        }
        case 1:
        { // 检查格口状态
            if (tickCheck >= DOOR_OPEN_TICKCOUNT)
            { // 门未打开
                db_newIncident(IT_DoornotOpen, shipment->boxid, "");
                step = 92;
            }
            // else if (relayBoard_GetLockStatus(side, relay) == 1) // Debug Code
            else if (relayBoard_GetLockStatus(side, relay) == 0)
            { // 门已打开
                if ((bleEventType == Customer_DropOff_Hot) || (bleEventType == Customer_DropOff) ||
                    (bleEventType == Courier_DropOff_Hot) || (bleEventType == Courier_DropOff))
                {
                    step = 11;
                    flagInput = 3;
                    strInput[0] = 0;
                }
                else
                {
                    os_dly_wait(500);
                    step = 91;
                }
            }
            break;
        }
        case 11:
        { // 开门成功，在30秒内检测关门
            if (tickCheck >= DOOR_CLOSE_TICKCOUNT)
            { // 30秒未关门
                step = 91;
            }
            else if (relayBoard_GetLockStatus(side, relay) == 1)
            { // 门已关
                step = 91;
            }
            else if (flagInput == 0)
            {
                // if (strInput[0] == '0')
                step = 94; // 按下0， 取消投递
                strInput[0] = 0;
            }
            break;
        }
        case 91:
        {
            if (bleEventType == Courier_PickUp || bleEventType == Customer_PickUp)
            {
                if (bleEventType == Customer_PickUp)
                {
                    ResShipment resSp;
                    resSp.type = 1;
                    strcpy(resSp.barcode, shipment->barcode);
                    strcpy(resSp.pin, shipment->pin);
                    resSp.boxIndex = index;
                    db_addResShipment(&resSp);
                }
                db_delShipment(shipment);
                event.eventType = (bleEventType == Courier_PickUp) ? RE_CourierPickup : RE_CustomerPickup;
                db_addEvent(&event);
                shipment->status = SS_Normal;
                boxFlagList[index].status = 0;
                db_saveBoxFlag(&boxFlagList[index], index);
                sprintf(strTemp, "Ble pickup %s at box-%d, type-%d:", shipment->pin, shipment->boxid, bleEventType);
                printLog(strTemp);
            }
            else if (bleEventType == Customer_DropOff || bleEventType == Customer_DropOff_Hot)
            {
                ResShipment resSp;
                resSp.type = 0;
                strcpy(resSp.barcode, shipment->barcode);
                strcpy(resSp.pin, shipment->pin);
                resSp.boxIndex = index;
                db_addResShipment(&resSp);
                shipment->status = SS_CUSDO;
                db_addShipment(shipment);
                event.eventType = RE_CustomerDeposited;
                db_addEvent(&event);
                boxFlagList[index].status = 1;
                db_saveBoxFlag(&boxFlagList[index], index);
                sprintf(strTemp, "Ble dropoff %s at box-%d, type-%d:", shipment->pin, shipment->boxid, bleEventType);
                printLog(strTemp);
            }
            else
            { // Courier_DropOff & Courier_DropOff_Hot
                shipment->status = SS_COUDO;
                event.eventType = RE_CourierDeposited;
            }
            ble_SendStatus(0, datas, 0);
            step = 99;
            break;
        }
        case 92:
        {
            datas[0] = 2; // 无格口分配
            ble_SendStatus(1, datas, 1);
            step = 99;
            break;
        }
        case 94:
        {
            datas[0] = 2; // 取消投递
            ble_SendStatus(1, datas, 1);
            if (bleEventType == Customer_DropOff || bleEventType == Customer_DropOff_Hot)
                db_newIncident(IT_AbortCustomerDropoff, shipment->boxid, shipment->barcode);
            else
                db_newIncident(IT_AbortCourierDropoff, shipment->boxid, shipment->barcode);
            printLog("Ble Dropoff Cancel.");
            step = 99;
            break;
        }
        case 99:
        {
            goto exit;
        }
        }
    }
exit:
    if (appSet.relayboard)
    {
        appSet.relayboard = 0;
        powerOff_RelayBoard;
    }
    flagInput = 0;
    clearInput();
    flow_FreeAll();
    curFlow = IdleFlow;
}
void flow_BleConfirm(void)
{
    char side, relay;
    u16 index;
    ble_SendStatus(0, 0, 0);
    side = 2 - (shipment->boxid / 1000);
    relay = (shipment->boxid % 1000) - 1;
    index = side * 256 + relay;
    db_addShipment(shipment);
    db_addEvent(&event);
    boxFlagList[index].status = 1;
    db_saveBoxFlag(&boxFlagList[index], index);
    char strTemp[LEN_INCIDENTINFO];
    sprintf(strTemp, "Ble dropoff %s at box-%d, type-%d.", shipment->pin, shipment->boxid, bleEventType);
    printLog(strTemp);
    curFlow = IdleFlow;
}
