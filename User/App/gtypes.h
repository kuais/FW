#ifndef __G_TYPES__
#define __G_TYPES__

#include "RTL.h"
#include <stdbool.h>

#define LEN_INCIDENTINFO 200

enum _FlowType
{ // 0:启动初始化, 1:同步; 2:待机;3:Login Flow;...; 91:清空数据;92:切换程序空间;93.省电模式;94:闹钟唤醒;
    StartupFlow,
    SyncFlow,
    SyncEpraFlow,
    BleOpenLockFlow = 60,
    BleConfirmFlow = 61,
    PrintShipmentFlow = 81,
    AlarmWakeupFlow = 91,
    AppSwitchFlow,
    ClearDataFlow,
    PowerSaveFlow,
    IdleFlow,
    LoginFlow,
    CourierPickupFLow,
    CourierDropoffFlow,
    DropoffFlow,
    CustomerPickupFlow,
    CustomerDropoffFlow,
    TechnicianFlow,
    SuperTechnicianFlow,
};
enum _BoxSizes
{
    XS,
    S,
    M,
    L,
    XL
};
enum _RuleTypes
{
    RT_CourierDO = 1,
    RT_CustomerPU = 2,
    RT_CustomerDO = 3,
    RT_CourierPU = 4,
};
enum _BarcodeType
{
    BT_COUDO = 1,
    BT_CUSPU = 2,
    BT_CUSDO = 3,
    BT_LOGIN = 4,
};
enum _ShipmentStatus
{
    SS_Normal,
    SS_COUDO,
    SS_CUSPU,
    SS_CUSDO,
    SS_RETURN, // 等待快递员取回
    SS_EPRA,
};
enum _ReservationEventTypes
{
    RE_CourierDeposited = 4,
    RE_CustomerDeposited = 5,
    RE_CustomerPickup = 6,
    RE_CourierPickup = 17,
    RE_NotCollectedCourier = 38
};
enum _IncidentTypes
{
    IT_Invalid,
    IT_DoornotOpen,
    IT_DoornotClose,
    IT_Doorwithoutparcel = 6,
    IT_Others = 11,
    IT_BarCodeNotValid = 23,
    // IT_NotCollectedCourier = 38,
    IT_ShipmentOverdue = 55,
    IT_ShipmentRemoved = 57,
    IT_BlockedDoor = 59,
    IT_CourierLogin = 60,
    IT_CourierLoginFail = 61,
    IT_SyncFailed = 62,
    IT_SyncFunish = 63,
    IT_BarcodeRead = 64,
    IT_AbortCustomerDropoff = 65,
    IT_AbortCourierDropoff = 66,
    IT_Log = 67
};
enum _BLE_EventType
{
    Courier_DropOff = 0x0, // reservation
    Customer_DropOff,      // reservation
    Courier_PickUp,        // Return|Expired
    Customer_PickUp,       // Return|Expired
    Box_Change,
    Courier_DropOff_Hot,  // Hot Drop
    Customer_DropOff_Hot, // Hot Drop
    BleEv_Unknow = 0xFF
};
typedef __packed struct
{
    unsigned ready : 1;   // 0:初始化中，   1：初始化完成
    unsigned connect : 1; // 0:未连接，     1：已连接
} BleFlag;
typedef __packed struct
{
    unsigned b0 : 1;
    unsigned b1 : 1;
    unsigned b2 : 1;
    unsigned b3 : 1;
    unsigned b4 : 1;
    unsigned b5 : 1;
    unsigned b6 : 1;
    unsigned b7 : 1;
} Byte;
typedef __packed struct
{
    unsigned router : 1;     // 0:路由断电   1:路由通电
    unsigned relayboard : 1; // 0:锁板断电 1:锁板通电
    unsigned scanner : 1;    // 0:扫描仪断电 1:扫描仪通电
    unsigned reOpen : 1;     // 0:自动开启格口 2：不自动开启格口
    unsigned isTimeout : 1;  // 0:正常 2：即将超时
    unsigned tempFlag : 2;   // b6-b7 临时状态， 根据屏幕操作赋值
    unsigned network : 1;    // 0:无网络; 1:有网络
} AppSetting;                // 程序运行时状态
typedef __packed struct
{
    unsigned doubleclick : 1; // 0: 默认  1:等待双击
    unsigned solarpanel : 1;  // 0: 不使用太阳能板； 1：使用太阳能板
    unsigned firstsync : 1;   // 0: 不是第一次同步 1: 第一次同步
    unsigned synclater : 1;   // 1: sync after 10 min to check EPRA parcel
    unsigned synclog : 1;     // 0: 上传日志，不上传日志
    unsigned undefined : 3;   // 未分配
} AppFlag;                    // 程序运行时标志位
typedef __packed struct
{
    char apiurl[100]; // api地址
    char apmID[11];   // 设备ID
    char aesKey[16];  // 蓝牙密钥
    char rebootTime;  // 设备重启时间[时]
    char floorCount;  // 锁板层数
    bool debug;       // 调试开关， true: 开, falue:关
} BoardParams;
typedef __packed struct
{
    unsigned status : 2; // b0-b1 0:空闲， 1:使用中  2:故障  3:无效 (实际只使用0和1)
    unsigned lock : 1;   // b2    0:关   1:开
    unsigned sensor : 1; // b3    0:无物 1:有物
    unsigned size : 3;   // b4-b6 0:XS, 1:S， 2:M， 3:L, 4:XL,5,6,7 预留
    unsigned used : 1;   // b7    分配格口时比较2个格口，如果两个格口此标志相同则分配前面的格口，否则分配后面的格口
} BoxFlag;
typedef __packed struct
{
    u16 id; // 格口ID
} Box;
typedef __packed struct
{
    char plu[9];   // 快递公司名称
    char size;     // 预留格口尺寸
    char reserved; // 预留格口数
} BoxPlu;
typedef __packed struct
{
    char id[13]; // id     12个字符+'\0'
    char plu[9]; // 快递公司名称    8个字符+'\0'
} Courier;
typedef __packed struct
{
    char plu[9]; // 快递公司名称
    char flag;   // 0: 不允许热投递 1：允许热投递
} Plu;
typedef __packed struct // 条码规则
{
    char rulename[30]; // LabelRule名称
    char codetype[4];  // 符号集 3个字符+'\0'
    char regex[256];   // 正则表达式
    u16 length;        // 长度
    u16 pos0;          // 实际条码起始位置
    u16 pos1;          // 实际条码结束位置
} LabelRule;
typedef __packed struct
{
    char rulename[30]; // LabelRule名称 29个字符+'\0'
    char type;         // 1 Delivery Carrier, 2 Pickup Client, 3 Delivery Client
    char plu[9];       // 快递公司名称
} LabelPlu;            // 快递公司条码规则
typedef __packed struct
{
    char id[13]; // id     12个字符+'\0'
    char type;   // 类型    1:可开启所有门;  2:只能开启空闲的门
} Technician;
typedef __packed struct
{
    char plu[9];      // 快递公司名称
    char size;        // 格口尺寸
    char barcode[30]; // 条码
    char pin[7];      // 取件码 6个字符+'\0'
    u16 boxid;        // 格口ID
    char status;      // b0-2 1.快递员投递;2:已取走 3:用户投递 4.等待取回;5:快递员取回
    char onlyApp;     // 0:可以键盘取件； 1:只能app取件
} Shipment;
typedef __packed struct
{
    Shipment shipment; // 订单信息
    char eventType;    // 事件类型
    char time[18];     // 时间 22-07-12 12:31:22 17个字符+'\0'
    char user[13];     // 操作者 id  快递员 12个字符+'\0';管理员 12个字符+'\0'
} Event;
typedef __packed struct
{
    char type;                   // 事件类型 参考 IncidentType
    u16 boxid;                   // 格口ID
    char time[18];               // 时间 22-07-12 12:31:22 17个字符+'\0' 实际格式为2022-07-12 12:31:22.000
    char info[LEN_INCIDENTINFO]; // 附加信息
} Incident;
typedef __packed struct
{
    char type;     // 事件类型 1:Login;2:OpenDoor;3:CloseDoor;4:CloseIncident
    char time[18]; // 时间 22-07-12 12:31:22 17个字符+'\0' 实际格式为2022-07-12 12:31:22.000
    char user[13]; // 操作者 id  快递员 12个字符+'\0';管理员 12个字符+'\0'
} TechnicianAction;
typedef __packed struct
{
    char plu[9];      // 快递公司名称
    char size;        // 格口尺寸
    char barcode[30]; // 条码
    char pin[7];      // 取件码
    char type;        // 0:快递员投递;1:用户投递
    char onlyApp;     // 0:可以键盘取件； 1:只能app取件
} PreReservation;
typedef __packed struct
{
    u32 time;         // 失效时间 MMddHHmm
    char barcode[30]; // 条码
    char pin[7];      // 取件码
    char type;        // 0:存件 1:取件
    u16 boxIndex;     // 格口序号
} ResShipment;

#endif
