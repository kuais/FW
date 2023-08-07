#include "modBle.h"
#include "KuFrame/kuconvert.h"
#include "KuFrame/kubuffer.h"
#include "Protocol/protocol.h"

#define BLE_DEBUG 0
#define BLE_OUT &g_tUart5

#if BLE_DEBUG
#define bleprintf printf
#else
#define bleprintf
#endif

static u16 errCount = 0;
static char arrMac[6]; // A6C080903AE9

volatile BleFlag bleFlag;

extern void e2p_GetBleID(U8 *p);

void ble_OnSent(char *p, int len)
{
#if BLE_DEBUG
    bleprintf("ble-send: ");
    for (int i = 0; i < strlen(p); i++)
        bleprintf("%c", *(p + i));
#endif
}
void ble_SendDatas(char *p, size_t len)
{
    ble_Send(p, len);
    ble_OnSent(p, len);
}
void ble_Test()
{
    char *p = "AT\r\n";
    size_t len = strlen(p);
    ble_SendDatas(p, len);
}
void ble_MAC()
{
    char *p = "AT+LBDADDR?\r\n";
    size_t len = strlen(p);
    ble_SendDatas(p, len);
}
void ble_ADV()
{
    unsigned char buf[100];
    int n = 15;
    memcpy(buf, "AT+ADVUSRDT=17,", n);
    buf[n++] = 0X30;
    buf[n++] = 0X38;
    buf[n++] = 0X30;
    buf[n++] = 0X38;
    for (int i = 0; i < 6; i++)
    {
        buf[n++] = toHex(arrMac[i] >> 4);
        buf[n++] = toHex(arrMac[i] & 0xF);
    }
    buf[n++] = 0X32;
    buf[n++] = 0X35;

    buf[n++] = 0X37;
    buf[n++] = 0X32;

    buf[n++] = 0X30;
    buf[n++] = 0X32;

    buf[n++] = 0X30;
    buf[n++] = 0X32;

    e2p_GetBleID(&buf[n]);
    n += 10;
    // for (int i = 0; i < 10; i++)
    //     buf[n++] = boardParams.bleID[i];

    buf[n++] = 0X0D;
    buf[n++] = 0X0A;

    ble_SendDatas(buf, n);
}

void ble_Init(void)
{
    bool bleFunc = true;
    // e2p_GetBleFunc(&bleFunc);
    if (bleFunc)
        bleStep = BLE_STEPINIT;
    else
        powerOff_BLE;
}

void ble_SendStatus(u8 flag, u8 *datas, int len)
{
    KuBuffer *bufSend = pBle_dataOfStatus(flag, datas, len);
    if (buffer_dataCount(bufSend) > 0)
        ble_Send(bufSend->datas, buffer_dataCount(bufSend));
    buffer_free(bufSend);
}
/**
 * @brief 接收处理
 * @param port 接收端口
 */
void ble_Handle(UART_T *port)
{
    KuBuffer *buf = port->bufRx;
    errCount = 0;
    while (buffer_dataCount(buf) > 0)
    {
        if ((*buffer_get(buf, 0)) == 0)
            buffer_remove(buf, 1);
        else
            break;
    }
    U16 len = buffer_dataCount(buf);
    if (len > 0)
    {
        // buffer_remove(buf, len);
#if BLE_DEBUG
        {
            U8 p[len + 1];
            memcpy(p, buffer_get(buf, 0), len);
            p[len] = 0;
            bleprintf("ble-recv(ASC): ");
            for (int i = 0; i < len; i++)
                bleprintf("%c", *(p + i));

            bleprintf("\r\n");
            bleprintf("ble-recv(HEX): ");
            for (int i = 0; i < len; i++)
                bleprintf("%02X ", *(p + i));
            bleprintf("\r\n");
        }
#endif
        int offset = 0;
        len = sizeof("+IM_READY") - 1;
        offset = buffer_finds(buf, 0, "+IM_READY", len);
        if (offset >= 0)
        {
            bleFlag.ready = 1;
            bleFlag.connect = 0;
            bleStep = BLE_STEPMAC;
            buffer_remove(buf, offset + len);
        }
        len = sizeof("+IM_CONNECT") - 1;
        offset = buffer_finds(buf, 0, "+IM_CONNECT", len);
        if (offset >= 0)
        {
            bleFlag.connect = 1;
            //            ble_Test();
            buffer_remove(buf, offset + len);
        }
        switch (bleStep)
        {
        case BLE_STEPTESTRET:
            if (buffer_finds(buf, 0, "OK", sizeof("OK") - 1) >= 0)
            {
                bleStep = BLE_STEPIDLE;
                buffer_clear(buf);
            }
            break;
        case BLE_STEPMACRET:
        {
            offset = buffer_finds(buf, 0, "+LBDADDR", sizeof("+LBDADDR") - 1);
            if (offset >= 0)
            {
                buffer_remove(buf, offset);
                int len = buffer_dataCount(buf);
                U8 p[len + 1];
                memcpy(p, buffer_get(buf, 0), len);
                p[len] = 0;
                printf("BLE Mac: ");
                for (int i = 0; i < 6; i++)
                {
                    arrMac[i] = (fromHex(p[10 + i * 3]) << 4) + fromHex(p[10 + 1 + i * 3]);
                    printf("%02x", arrMac[i]);
                }
                printf("\r\n");
                bleStep = BLE_STEPADV;
                buffer_clear(buf);
            }
            break;
        }
        case BLE_STEPADVRET:
            if (buffer_finds(buf, 0, "OK", sizeof("OK") - 1) >= 0)
            {
                bleStep = BLE_STEPIDLE;
                buffer_clear(buf);
            }
            break;
        case BLE_STEPIDLE:
            if (bleFlag.connect == 0)
                bleFlag.connect = 1;
            if (curFlow != IdleFlow && curFlow != PowerSaveFlow)
            { // 非待机模式不处理
                buffer_clear(buf);
            }
            else
            {
                offset = buffer_find(buf, 0, 0xA1);
                if (offset < 0)
                    offset = buffer_find(buf, 0, 0xA2);
                if (offset >= 0)
                {
                    buffer_remove(buf, offset);
                    int len = buffer_dataCount(buf);
                    if (len < 20)
                        return;
                    if (*buffer_get(buf, 1) == 0x55)
                    {
                        tickCheck = 0;
                        flagInput = 4;
                        U8 p[len + 1];
                        memcpy(p, buffer_get(buf, 0), len);
                        p[len] = 0;
                        KuBuffer *bufSend = pBle_Handle(p);
                        if (buffer_dataCount(bufSend) > 0)
                            ble_Send(bufSend->datas, buffer_dataCount(bufSend));
                        buffer_free(bufSend);
                    }
                }
                buffer_clear(buf);
            }
            break;
        default:
            break;
        }
    }
}
void ble_Start(void)
{
    errCount = 0;
    bleFlag = *(BleFlag *)0;
    bleStep = BLE_STEPIDLE;
    ble_Reset();
}
void ble_Task(void)
{
    switch (bleStep)
    {
    case BLE_STEPINIT:
        ble_Start();
        break;
    case BLE_STEPTEST:
        ble_Test();
        errCount = 0;
        bleStep = BLE_STEPTESTRET;
        break;
    case BLE_STEPTESTRET:
        if ((++errCount) >= 500)
            bleStep = BLE_STEPIDLE;
        break;
    case BLE_STEPMAC:
        ble_MAC();
        errCount = 0;
        bleStep = BLE_STEPMACRET;
        break;
    case BLE_STEPADV:
        ble_ADV();
        errCount = 0;
        bleStep = BLE_STEPADVRET;
        break;
    case BLE_STEPIDLE:
        //		if (bleFlag.connect)
        //		{
        //			if ((++errCount) >= 500)
        //				ble_Start();
        //		}
        break;
    case BLE_STEPSTOP:
        break;
    default:
        if ((++errCount) >= 500)
            bleStep--;
        break;
    }
}
