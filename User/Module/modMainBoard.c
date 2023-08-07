#include "modMainBoard.h"
#include "KuFrame/kuconvert.h"
#include "modBle.h"
#include "modE2p.h"
#include "modFIle.h"
#include "modFlash.h"
#include "modRelayBoard.h"
#include "modTime.h"
#include "modDb.h"

#define MB_DEBUG 0
#define MB_OUT uart2_Send

static void send_ack(u8 cmd)
{
    u8 buf[4];
    buf[0] = cmd;
    buf[1] = 0xaa;
    buf[2] = 0x55;
    buf[3] = 0x0d;
    MB_OUT(buf, 4);
}
static void send_err(u8 cmd)
{
    u8 buf[5];
    buf[0] = MB_ERROR;
    buf[1] = cmd;
    buf[2] = 0xaa;
    buf[3] = 0x55;
    buf[4] = 0x0c;
    MB_OUT(buf, 5);
}
static int getDataLength(u8 *p, int len0)
{
    int len = 0;
    switch (p[0])
    {
    case MB_AT:
    {
        if (p[1] != 'T')
            len = 1;
        else
            len = len0;
        break;
    }
    case MB_WRITE:
    {
        len = p[3];
        if (len > 192)
            len = 192;
        len += 4;
        break;
    }
    case MB_RELAYSTATUS:     // 读取系统输入检测的信号
    case MB_LAYERCOUNT:      // 读出层数
    case MB_APIINTERVAL_SET: // 设置 Api Url
    case MB_SWITCHDEBUG:     // 调试日志开关
    case MB_SWITCHAPP:       // 切换程序空间
    case MB_BLEFUNC_SET:     // 蓝牙功能开关
    case MB_APMID_GET:       // 获取APMID
    case MB_RESET:
    case MB_SYSTEM_INIT: // 初始化
        len = 2;
        break;
    case MB_LAYERCOUNT_SET: // 写入层数
    case MB_CLEAR_DB:       // 删除数据
        len = 3;
        break;
    case MB_READ:
    case MB_INPUTTOKEN: // TOKEN 回复
        len = 4;
        break;
    case MB_UNLOCK:
        len = 5;
        break;
    case MB_TIME_SET: // 设定时间
        len = 7;
        break;
    case MB_APMID_SET: // 设置 ApmID
    case MB_BLEID_SET: // 设置 BleID
        len = 11;
        break;
    case MB_AESKEY_SET: // 设置 AES 密码
        len = 17;
        break;
    case MB_APIURL_SET: // 设置 Api Url
        len = 101;
        break;
    case MB_NETPARAMS_SET: /* 设置网络参数 */
        len = 13;
        break;
    default:
        len = 1;
        break;
    }
    return len;
}
void mainBoard_SendBarcode(char *datas, int len)
{ /* 转发条码数据 */
    u8 sendbuf[len + 2];
    sendbuf[0] = MB_BARCODE;
    memcpy(&sendbuf[1], datas, len);
    sendbuf[len + 1] = 0;
    MB_OUT(sendbuf, len + 2);
}
void mainBoard_KeyInput(char key)
{
    u8 sendbuf[2];
    sendbuf[0] = MB_KEYINPUT;
    sendbuf[1] = key;
    MB_OUT(sendbuf, 2);
}
void mainBoard_Handle(UART_T *port)
{
    KuBuffer *buf = port->bufRx;
    int len0 = buffer_dataCount(buf);
    if (len0 <= 0)
        return;
    tickCheck = 0;
    u8 *p = buffer_get(buf, 0);
#if MB_DEBUG
    printf("mb recv: ");
    for (int i = 0; i < len0; i++)
        printf("%02X ", *(p + i));
    printf("\r\n");
#endif
    char flagCmd[2] = {0};
    while (len0 > 0)
    {
        int len = 0;
        len = getDataLength(p, len0);
        if (len0 < len)
            break;
        p = buffer_get(buf, 0);
        switch (p[0])
        {
        case MB_AT:
        {
            if (p[1] == 'T')
            {
				int n = 0;
				for (n = 1; n < len; n++)
				{
					if (p[n] == '\n')
					{
						u8 sendbuf[n + 1];
						memcpy(sendbuf, p, n + 1);
						ble_Send(sendbuf, n + 1);
						sendbuf[n - 1] = 0;			// 移除 \r\n
						e2p_SetBleName(&sendbuf[8]);
					}
				}
            }
            break;
        }
        case MB_WRITE:
        {
            u16 addr = p[1] * 256 + p[2];
            e2p_WriteBytes(addr, &p[4], len - 4);
            send_ack(p[0]);
            break;
        }
        case MB_READ:
        {
            u8 sendbuf[256];
            u16 addr = p[1] * 256 + p[2];
            u16 len2 = p[3];
            if (len2 > 192)
                len2 = 192;
            sendbuf[0] = p[0];
            sendbuf[1] = p[1];
            sendbuf[2] = p[2];
            sendbuf[3] = p[3];
            e2p_ReadBytes(addr, &sendbuf[4], len2);
            MB_OUT(sendbuf, len2 + 4);
            break;
        }
        case MB_UNLOCK:
        {
            u8 sum = p[0] ^ p[4];
            if (sum == 0xff)
            {
                u16 addr = p[2] * 100 + (p[3] >> 4) * 10 + (p[3] & 0x0f);
                if ((p[1] == 0xAA) || (p[1] == 0x55))
                {
                    relayBoard_Unlock((p[1] == 0xAA) ? 0 : 1, addr - 1); // 开out1
                    send_ack(p[0]);
                }
                else
                    send_err(p[0]);
            }
            else
                send_err(p[0]);
            break;
        }
        case MB_RELAYSTATUS: // 读取系统输入检测的信号
        {
            u8 sendbuf[41];
            if (p[1] == 0x55)
            {
                if (flagCmd[0] == 1)
                    break;
                else
                    flagCmd[0] = 1;
            }
            else if (p[1] == 0xAA)
            {
                if (flagCmd[1] == 1)
                    break;
                else
                    flagCmd[1] = 1;
            }
            else
            {
                send_err(p[0]);
                break;
            }
            sendbuf[0] = MB_RELAYSTATUS;
            sendbuf[1] = p[1];
            memcpy(&sendbuf[2], (p[1] == 0xAA) ? relayBoard[0].relayStatus : relayBoard[1].relayStatus, 38);
            sendbuf[40] = ~sendbuf[0];
            MB_OUT(sendbuf, 41);
            break;
        }
        case MB_VERSION:
        {
            MB_OUT((u8 *)VERSION, 6);
            break;
        }
        case MB_5V_ON:
        {
            powerOn_Scanner;
            send_ack(p[0]);
            break;
        }
        case MB_5V_OFF:
        {
            powerOff_Scanner;
            send_ack(p[0]);
            break;
        }
        case MB_POWERSTATUS: // 反馈 是否 掉电
        {
            u8 sendbuf[2];
            sendbuf[0] = MB_POWERSTATUS; // 反馈 PC 端
            sendbuf[1] = 0xAA;
            MB_OUT(sendbuf, 2);
            break;
        }
        case MB_RESET: // 单片机复位
        {
            if (p[1] == 0x55)
            {
                send_ack(p[0]);
                os_dly_wait(10);
                bsp_Restart();
            }
            else
            {
                send_err(p[0]);
            }
            break;
        }
        case MB_LAYERCOUNT_SET: // 写入层数
        {
            if ((p[1] != 'W') || ((p[2] != 8) && (p[2] != 9) && (p[2] != 10) && (p[2] != 12)))
            {
                send_err(p[0]);
            }
            else
            {
                e2p_SetFloorCount(p[2]);
                boardParams.floorCount = p[2];
                u8 sendbuf[4];
                sendbuf[0] = p[0]; // 反馈 PC 端
                sendbuf[1] = p[1]; //
                sendbuf[2] = p[2]; //
                sendbuf[3] = 0x0d; //
                MB_OUT(sendbuf, 4);
            }
            break;
        }
        case MB_LAYERCOUNT: // 读出层数
        {
            if (p[1] != 'R')
                send_err(p[0]);
            else
            {
                u8 sendbuf[4];
                sendbuf[0] = p[0];
                sendbuf[1] = p[1];
                sendbuf[2] = boardParams.floorCount;
                sendbuf[3] = 0x0d; //
                MB_OUT(sendbuf, 4);
            }
            break;
        }
        case MB_APMID_GET:
        {
            p[1] = (~p[1]);
            if (p[1] != p[0])
                send_err(p[0]);
            else
            {
                u8 sendbuf[11] = {0};
                sendbuf[0] = p[0];
                memcpy(&sendbuf[1], boardParams.apmID, 10);
                MB_OUT(sendbuf, 11);
            }
            break;
        }
        case MB_APMID_SET: // 设置 ApmID
        {
            e2p_SetApmID(&p[1]);
            memcpy(&boardParams.apmID[0], (unsigned char *)&p[1], 10);
            send_ack(p[0]);
            os_dly_wait(10);
            bsp_Restart();
            break;
        }
        case MB_APIURL_SET: // 设置 Api Url
        {
            e2p_SetApiUrl(&p[1]);
            memcpy(&boardParams.apiurl[0], (unsigned char *)&p[1], 100);
            send_ack(p[0]);
            break;
        }
        case MB_APIINTERVAL_SET: // 设置 Api Url
        {
            send_ack(p[0]);
            break;
        }
        case MB_BLEID_SET: // 设置 BleID
        {
            e2p_SetBleID(&p[1]);
            // memcpy(&boardParams.bleID[0], (unsigned char *)&p[1], 10);
            send_ack(p[0]);
            ble_Init();
            break;
        }
        case MB_BLEFUNC_SET: // 开关蓝牙功能
        {
            e2p_SetBleFunc(p[1]);
            if (p[1] == 0)
                powerOff_BLE;
            else
                powerOn_BLE;
            send_ack(p[0]);
            break;
        }
        case MB_AESKEY_SET: // 设置 AES 密码
        {
            memcpy(boardParams.aesKey, (unsigned char *)&p[1], 16);
            e2p_SetAesKey(boardParams.aesKey);
            send_ack(p[0]);
            os_dly_wait(10);
            // bsp_Restart();
            break;
        }
        case MB_INPUTTOKEN: // TOKEN 回复
        {
            // BLE_Status(p, p[2], 0, 0);
            break;
        }
        case MB_NETPARAMS_SET:
        { /* 设置网络参数 */
            e2p_SetNetParams(&p[1]);
            send_ack(p[0]);
            os_dly_wait(10);
            bsp_Restart(); // 复位 重新加载网络模块
            break;
        }
        case MB_NETPARAMS:
        { /* 获取网络参数 */
            u8 sendbuf[13];
            sendbuf[0] = MB_NETPARAMS;
            e2p_GetNetParams(&sendbuf[1]);
            MB_OUT(sendbuf, 13);
            break;
        }
        case MB_ADC_GET:
        { /* 获取当前电压 */
            u8 sendbuf[4];
            u16 v = bsp_Voltage();
            sendbuf[0] = MB_ADC_GET;
            sendbuf[1] = v / 100; // 电压整数部分
            sendbuf[2] = v % 100; // 电压小数部分
            sendbuf[3] = 0;       // 电压小数部分
            MB_OUT(sendbuf, 3);
            break;
        }
        case MB_SYSTEM_INIT:
        { /* 恢复出厂设置 */
            u8 v = ~p[0];
            if (p[1] != v)
                break;
            e2p_SetFlag_Init(0x66);
            send_ack(p[0]);
            os_dly_wait(10);
            bsp_Restart();
            os_dly_wait(10);
            break;
        }
        case MB_FILESYSTEM_INIT: // 初始化文件系统
        {
            curFlow = ClearDataFlow;
            send_ack(p[0]);
            break;
        }
        case MB_TIME: // 读取时间
        {
            u8 sendbuf[7];
            Time t;
            time_now(&t);
            sendbuf[0] = p[0];
            sendbuf[1] = decToBcd(t.year);
            sendbuf[2] = decToBcd(t.month);
            sendbuf[3] = decToBcd(t.day);
            sendbuf[4] = decToBcd(t.hour);
            sendbuf[5] = decToBcd(t.minute);
            sendbuf[6] = decToBcd(t.second);
            MB_OUT(sendbuf, 7);
            break;
        }
        case MB_TIME_INIT: // 初始化时间
        {
            time_init();
            send_ack(p[0]);
            break;
        }
        case MB_TIME_SET: // 设定时间
        {
            send_ack(p[0]);
            os_dly_wait(10);
            Time t;
            t.year = bcdToDec(p[1]);
            t.month = bcdToDec(p[2]);
            t.day = bcdToDec(p[3]);
            t.hour = bcdToDec(p[4]);
            t.minute = bcdToDec(p[5]);
            t.second = bcdToDec(p[6]);
            time_set(&t);
            break;
        }
        case MB_CLEAR_DB: // 删除数据
        {
            if (p[2] != (u8)(-MB_CLEAR_DB))
                break;
            flagCleadDB = 'P';
            send_ack(p[0]);
            break;
        }
        case MB_NOPC: // 切换蓝牙脱机和联机模式
        {
            // BLE_NoPC();      // 屏蔽
            send_ack(p[0]);
            break;
        }
        case MB_STOP_POWER: // 进入低功耗
        {
            send_ack(p[0]);
            curFlow = PowerSaveFlow;
            break;
        }
        case MB_SWITCHDEBUG:
        {
            send_ack(p[0]);
            boardParams.debug = p[1];
            break;
        }
        case MB_SWITCHAPP:
        {
            send_ack(p[0]);
            if (p[1] != flash_getAppFlag())
                curFlow = AppSwitchFlow;
            break;
        }
        case MB_PRINTDB:
        {
            send_ack(p[0]);
            if (curFlow == IdleFlow)
                curFlow = PrintShipmentFlow;
            break;
        }
        case MB_HOLD:
        {
            send_ack(p[0]);
            while (1)
                ;
            break;
        }
        }
        buffer_remove(buf, len);
        len0 -= len;
        os_dly_wait(1);
    }
}
