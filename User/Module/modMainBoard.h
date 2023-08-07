#ifndef __MODULE_MAINBOARD__
#define __MODULE_MAINBOARD__

#include "main.h"

enum _MB_CMD
{
    MB_RELAYSTATUS = 0xC,
    MB_UNLOCK,
    MB_VERSION,
    MB_5V_ON,
    MB_5V_OFF,
    MB_MIKESTATUS = 0x13,
    MB_POWERSTATUS,
    MB_RESET,
    MB_LAYERCOUNT_SET,
    MB_LAYERCOUNT,
    MB_APMID_GET,
    MB_UNLOCK_MINI = 0x1A,
    MB_5V_SWITCH = 0x20,
    MB_TEMPERATURE_PROBE,
    MB_BLEID_SET,
    MB_AESKEY_SET,
    MB_INPUTTOKEN,
    MB_APMID_SET,
    MB_APIURL_SET,
    MB_APIINTERVAL_SET,
    MB_BLEFUNC_SET,
    MB_AT = 'A',             // AT指令
    MB_READ = 'R',           // 0x52
    MB_WRITE = 'W',          // 0x57
    MB_NETPARAMS_SET = 0xC0, // C0
    MB_NETPARAMS = 0xC1,     // C1
    MB_ADC_GET,              // C2
    MB_CLEAR_DB = 0xCD,
    MB_SYSTEM_INIT = 0xCE,
    MB_FILESYSTEM_INIT = 0xCF,
    MB_TIME,               // D0
    MB_TIME_INIT,          // D1
    MB_TIME_SET,           // D2
    MB_NOPC,               // D3
    MB_LOCKER_12,          // D4
    MB_LOCKER_12_STA,      // D5
    MB_STOP_POWER,         // D6
    MB_SWITCHDEBUG = 0xE0, // E0, 调试开关
    MB_SWITCHAPP,          // E1，程序空间切换
    MB_PRINTDB,            // E2, 打印终端数据
    MB_HOLD = 0xEF,        // EF，使程序卡死
    MB_KEYINPUT = 0xFC,
    MB_BARCODE,
    MB_CARDID,
    MB_ERROR
};

extern void mainBoard_Handle(UART_T *port);
extern void mainBoard_SendBarcode(char *datas, int len);
extern void mainBoard_KeyInput(char key);

#endif
