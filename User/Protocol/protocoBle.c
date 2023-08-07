/**
 * @file protocoBle.c
 * @author your name (you@domain.com)
 * @brief Kern 蓝牙通讯协议
 * @version 0.1
 * @date 2022-11-07
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "protocol.h"
#include "KuFrame/kuutil.h"
#include "Module/modTime.h"
#include "Module/modRelayBoard.h"
#include "Module/modDb.h"
#include "Module/mode2p.h"
#include "Module/modCrypto.h"

enum _BLE_CMD
{
    GET_ART = 0x00,
    OPEN_LOCK = 0x10,
    GET_RELAY_STATUS,
    GET_SENSOR_STATUS,
    GET_ALL_RELAY_STATUS,
    GET_ALL_SENSOR_STATUS,
    GET_POWER_STATUS,
    GET_ICCARD_DATA,
    GET_VERSION,
    OPEN_LOCK_V2 = 0x30,
    GET_EVENTS,
    SET_AES_KEY = 0x40,
    SET_TIME,
    CONFIRM_ACTION,
    REMAP_TERMINAL = 0x50,
    GET_BOX_STATUS,
    SET_BOX_STATUS,
    GET_PARCELS,
    CLEAN_MEMORY,
    STATUS,
    GET_BOXID = 0x65,
    DISCONNECT = 0x70,
    INPUT_TOKEN = 0x76,
    TOKEN_UPDATE
};

static unsigned char NEW_ART[2];
static KuBuffer *tempBuf = 0;

static void encrypt(uint8_t *src, uint8_t *dst)
{ /* 加密 PAYLOAD 3<-->18,16byte */
    encrypt_AES_ECB(src, dst, boardParams.aesKey, 128);
}
static void decrypt(uint8_t *src, uint8_t *dst)
{ /* 解密 PAYLOAD 3<-->18,16byte */
    decrypt_AES_ECB(src, dst, boardParams.aesKey, 128);
}
static void pack(char *datas)
{
    encrypt(&datas[3], &datas[3]);
    /* 累加和 */
    datas[19] = getSum(datas, 0, 19);
}

KuBuffer *pBle_dataOfStatus(unsigned char flag, unsigned char *datas, int len)
{
    KuBuffer *buf = buffer_new(20);
    if (!datas)
    {
        switch (flag)
        {
        case 0:
            datas = "SUCCESS";
            break;
        case 1:
            datas = "ERROR";
            break;
        case 2:
            datas = "WRONG_CODE";
            break;
        case 3:
            datas = "BLOCKED";
            break;
        case 5:
            datas = "CONTAINS_PICKUP";
            break;
        case 6:
            datas = "NO_RETURN";
            break;
        case 7:
            datas = "NOT_SUPPORTED";
            break;
        }
    }
    buf->datas[0] = 0xA1;
    buf->datas[1] = 0x55;
    buf->datas[2] = (2 + len) << 4;
    buf->datas[3] = NEW_ART[0];
    buf->datas[4] = NEW_ART[1];
    buf->datas[5] = STATUS;
    buf->datas[6] = flag;
    memcpy((void *)&buf->datas[7], (unsigned char *)datas, 12);
    pack(buf->datas);
    buf->count = 20;
    return buf;
}
/*
 *  GET_ART 指令，返回新ART
 */
KuBuffer *pBle_GetART(void)
{
    int ART_TEMP;
    size_t len;
    unsigned char datas[12] = {0};
    printf("Ble_GetART\r\n");
    randomBytes(datas, 12, &len);
    ART_TEMP = getSum((char *)datas, 0, 12);
    NEW_ART[0] = (ART_TEMP >> 8);
    NEW_ART[1] = ART_TEMP;
    revertArr(datas, 12);
    return pBle_dataOfStatus(0, datas, 0);
}
KuBuffer *pBle_GetBoxID(void)
{
    return pBle_dataOfStatus(0, boardParams.apmID, 10);
}
KuBuffer *pBle_OpenLock(uint8_t *input)
{
    printf("Ble_OpenLock\r\n");
    if (!appSet.relayboard)
    {
        powerOn_RelayBoard;
        appSet.relayboard = 1;
        os_dly_wait(1500);
    }
    relayBoard_Unlock(input[6], input[7] - 1);
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	GET_RELAY_STATUS 指令, 获取继电器状态   0:关；1:开
 */
KuBuffer *pBle_GetRelayStatus(uint8_t *input)
{
    unsigned char datas[1];
    printf("Ble_GetRelayStatus\r\n");
    datas[0] = 1 - relayBoard_GetLockStatus(input[6], input[7] - 1);
    return pBle_dataOfStatus(0, datas, 0);
}
/*
 *	GET_SENSOR_STATUS 指令, 获取传感器状态  0:无物；1:有物
 */
KuBuffer *pBle_GetSensorStatus(uint8_t *input)
{
    unsigned char datas[1];
    printf("Ble_GetSensorStatus\r\n");
    datas[0] = 1 - relayBoard_GetSensorStatus(input[6], input[7] - 1);
    return pBle_dataOfStatus(0, datas, 0);
}
/*
 *	GET_ALL_RELAY_STATUS指令, 获取所有继电器状态，分3帧返回
 */
KuBuffer *pBle_GetAllRelayStatus(uint8_t *input)
{
    return pBle_dataOfStatus(0, 0, 0);
    // unsigned char datas[12] = {0};
    // unsigned char side = input[6];
    // unsigned char i, j, n; // i = 继电器， j = datas序号, n = 总数据序号
    // U8 i1, i2, b;
    // KuBuffer *buf;
    // printf("Ble_GetAllRelayStatus\r\n");
    // U8 *p = relayBoard[side].relayStatus;
    // i = j = n = 0;
    // while (1)
    // {
    //     i1 = (i * 2) / 8;
    //     i2 = (i * 2) % 8;
    //     b = 1 - getBit(p[i1], i2, 1);
    //     b <<= (i % 8);
    //     datas[j] += b;
    //     i++;
    //     if ((i % 8) == 0)
    //     {
    //         j++;
    //         n++;
    //     }
    //     if (n == 32)
    //     { // 总共发送32字节
    //         buf = pBle_dataOfStatus(0, datas, j);
    //         ble_Send(buf->datas, buffer_dataCount(buf));
    //         break;
    //     }
    //     else if (j == 12)
    //     { // 每次最多发送12个字节
    //         buf = pBle_dataOfStatus(0, datas, j);
    //         ble_Send(buf->datas, buffer_dataCount(buf));
    //         memset(datas, 0, 12);
    //         j = 0;
    //         os_dly_wait(100);
    //     }
    // }
    // buffer_free(buf);
    // return buffer_new(0);
}
/*
 *	GET_ALL_SENSOR_STATUS 获取所有传感器状态
 */
KuBuffer *pBle_GetAllSensorStatus(uint8_t *input)
{ // TODO
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	GET_POWER_STATUS 获取掉电状态 0:掉电 1:正常
 */
KuBuffer *pBle_GetPowerStatus(void)
{ // TODO
    unsigned char datas[1];
    datas[0] = 1; //  (power_fall == 0xAA) ? 0 : 1;
    return pBle_dataOfStatus(0, datas, 0);
}
/*
 *	GET_ICCARD_DATA 获取检测到的IC卡号
 */
KuBuffer *pBle_GetRfidData(void)
{ // TODO
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	GET_VERSION 获取固件版本
 */
KuBuffer *pBle_GetVersion(void)
{
    unsigned char datas[12] = {0};
    int v = IVERSION, i = 11;
    while (v > 0)
    {
        datas[i--] = v % 0x100;
        v >>= 8;
    }
    return pBle_dataOfStatus(0, datas, 12);
}
/*
 *	Description:
 * 		Client requests to terminal to open a box. In this case, the command contains the type of process and
 *		Pin/OrderId to identify the process and allow the operation of offline terminals.
 */
KuBuffer *pBle_OpenLockV2(uint8_t *input)
{
    unsigned char side, relay;
    printf("Ble_OpenLockV2\r\n");
    bleEventType = input[8];
    // bleEventType = Customer_DropOff_Hot; // Debug Code
    memset(shipment->barcode, 0, 30);
    memcpy(shipment->barcode, (const void *)&input[9], 6);
    memcpy(shipment->pin, (const void *)&input[9], 6);
    switch (input[8])
    {
    case Courier_DropOff:
    case Customer_DropOff:
    {
        side = input[6];
        relay = input[7] - 1;
        shipment->boxid = (side + 1) * 1000 + relay + 1;
        break;
    }
    case Courier_PickUp:
    case Customer_PickUp:
        break;
    case Courier_DropOff_Hot:
    case Customer_DropOff_Hot:
    {
        shipment->size = 0;
        break;
    }
    case Box_Change:
    {
        break;
    }
    }
    // }
    curFlow = BleOpenLockFlow;
    return buffer_new(0);
}
/*
 *	GET_EVENTS
 */
KuBuffer *pBle_GetEvent(void)
{
    return pBle_dataOfStatus(1, 0, 0);
}

/*
 *	SET_AES_KEY 设置AES KEY
 */
KuBuffer *pBle_SetAesKey(uint8_t *input)
{
    printf("Ble_SetAesKey\r\n");
    if (tempBuf == 0)
    {
        tempBuf = buffer_new(16);
        buffer_put(tempBuf, &input[6], 13);
        return buffer_new(0);
    }
    else
    {
        buffer_put(tempBuf, &input[6], 3);
        memcpy(boardParams.aesKey, buffer_get(tempBuf, 0), 16);
        e2p_SetAesKey(boardParams.aesKey);
        buffer_free(tempBuf);
        tempBuf = 0;
        return pBle_dataOfStatus(0, 0, 0);
    }
}
KuBuffer *pBle_SetTime(uint8_t *input)
{
    printf("Ble_SetTime\r\n");
    Time t;
    t.year = (input[6] - 0x30) * 10 + (input[7] - 0x30);
    t.month = (input[8] - 0x30) * 10 + (input[9] - 0x30);
    t.day = (input[10] - 0x30) * 10 + (input[11] - 0x30);
    t.hour = (input[12] - 0x30) * 10 + (input[13] - 0x30);
    t.minute = (input[14] - 0x30) * 10 + (input[15] - 0x30);
    t.second = (input[16] - 0x30) * 10 + (input[17] - 0x30);
    time_set(&t);
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	CONFIRM_ACTION
 *	type:	Courier Drop Off: 		0x00.
 *			Customer Drop Off: 		0x01.
 *			Courier Pick Up: 		0x02 (Returned/Expired).
 *			Customer Pick Up: 		0x03.
 *			Box Change: 			0x04.
 * 	pin:	OrderID
 *  error:  0: completed, 1: Pin/OrderId not found, 2:Others
 */
KuBuffer *pBle_ConfirmAction(uint8_t *input)
{
    printf("Ble_ConfirmAction\r\n");
    if (memcmp(shipment->pin, &input[7], 6) != 0)
    {
        unsigned char datas[1] = {1};
        return pBle_dataOfStatus(1, datas, 1);
    }
    else
    {
        if ((input[6] == 0) && (shipment->status == SS_COUDO))
        {
            curFlow = BleConfirmFlow;
            return buffer_new(0);
        }
        else
            return pBle_dataOfStatus(0, 0, 0);
    }
}
/*
 *	REMAP_TERMINAL
 */
KuBuffer *pBle_RemapTeminal(void)
{
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	GET_BOX_STATUS
 */
KuBuffer *pBle_GetBoxStatus(void)
{
    unsigned char datas[12] = {0};
    return pBle_dataOfStatus(1, datas, 1);
}
/*
 *	SET_BOX_STATUS
 */
KuBuffer *pBle_SetBoxStatus(void)
{
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	GET_PARCELS
 */
KuBuffer *pBle_GetParcel(void)
{
    return pBle_dataOfStatus(1, 0, 0);
}
/*
 *	CLEAN_MEMORY
 */
KuBuffer *pBle_CleanMemory(void)
{
    printf("Ble_CleanMemory\r\n");
    curFlow = ClearDataFlow;
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	DISCONNECT 断开连接
 */
KuBuffer *pBle_Disconnect(void)
{
    return pBle_dataOfStatus(0, 0, 0);
}
/*
 *	INPUT_TOKEN 指令，透传给 PC端
 *	0X24 + TOKEN (13个字节） ----》DLL ----》客人
 */
KuBuffer *pBle_InputToken(void)
{
    return pBle_dataOfStatus(0, 0, 0);
}

/*
 *	TOKEN_UPDATE
 */
KuBuffer *pBle_TokenUpdate(void)
{
    // TODO TOKEN_UPDATE
    return pBle_dataOfStatus(0, 0, 0);
}

KuBuffer *pBle_Handle(uint8_t *bufRecv)
{
    int sum, sum2;
    printf("BLE Handler\r\n");
    decrypt(&bufRecv[3], &bufRecv[3]);
    /* 校验 AES-KEY */
    sum = (bufRecv[3] << 8) + bufRecv[4];
    if (bufRecv[5] == GET_ART)
    {
        sum2 = getSum(boardParams.aesKey, 0, 16);
        if (sum != sum2)
        { // AES-KEY 不匹配
            NEW_ART[0] = bufRecv[3];
            NEW_ART[1] = bufRecv[4];
            return pBle_dataOfStatus(1, 0, 0);
        }
    }
    else
    {
        sum2 = (NEW_ART[0] << 8) + NEW_ART[1];
        if (sum != sum2)
        { // ART不匹配
            return pBle_dataOfStatus(1, 0, 0);
        }
    }

    /* 处理命令 */
    switch (bufRecv[5])
    {
    case GET_ART:
        return pBle_GetART();
    case GET_BOXID:
        return pBle_GetBoxID();
    case OPEN_LOCK:
        return pBle_OpenLock(bufRecv);
    case GET_RELAY_STATUS:
        return pBle_GetRelayStatus(bufRecv);
    case GET_SENSOR_STATUS:
        return pBle_GetSensorStatus(bufRecv);
    case GET_ALL_RELAY_STATUS:
        return pBle_GetAllRelayStatus(bufRecv);
    case GET_ALL_SENSOR_STATUS:
        return pBle_GetAllSensorStatus(bufRecv);
    case GET_POWER_STATUS:
        return pBle_GetPowerStatus();
    case GET_ICCARD_DATA:
        return pBle_GetRfidData();
    case GET_VERSION:
        return pBle_GetVersion();
    case OPEN_LOCK_V2:
        return pBle_OpenLockV2(bufRecv);
    case GET_EVENTS:
        return pBle_GetEvent();
    case SET_AES_KEY:
        return pBle_SetAesKey(bufRecv);
    case SET_TIME:
        return pBle_SetTime(bufRecv);
    case CONFIRM_ACTION:
        return pBle_ConfirmAction(bufRecv);
    case REMAP_TERMINAL:
        return pBle_RemapTeminal();
    case GET_BOX_STATUS:
        return pBle_GetBoxStatus();
    case SET_BOX_STATUS:
        return pBle_SetBoxStatus();
    case GET_PARCELS:
        return pBle_GetParcel();
    case CLEAN_MEMORY:
        return pBle_CleanMemory();
    case STATUS:
        return pBle_dataOfStatus(0, 0, 0);
    case DISCONNECT:
        return pBle_Disconnect();
    case INPUT_TOKEN:
        return pBle_InputToken();
    case TOKEN_UPDATE:
        return pBle_TokenUpdate();
    default:
        return pBle_dataOfStatus(1, 0, 0);
    }
}
