#include "main.h"
#include "Kern/kern_api.h"
#include "Module/flow.h"
#include "Module/modBle.h"
#include "Module/modE2p.h"
#include "Module/modFile.h"
#include "Module/modFlash.h"
#include "Module/modKern.h"
#include "Module/modRelayBoard.h"
#include "Module/modTime.h"
#include "Module/modW5500.h"
#include "Module/modScanner.h"
#include "Module/modDb.h"
#include "Module/modCrypto.h"
#include "KuFrame/kustring.h"

static uint64_t stkInit[20];
static uint64_t stkUart[128];
static uint64_t stkApp[2000];

// static uint64_t stkInit[40];
// static uint64_t stkUart[160];
// static uint64_t stkApp[2000];

OS_MUT mutex_e2p;
OS_SEM semaphore_uart1, semaphore_uart2, semaphore_uart3, semaphore_uart4, semaphore_uart5;

u16 boxCount = 0;
AppSetting appSet; // 初始状态为路由通电
AppFlag appFlag;
Box *boxList = NULL;
BoxFlag *boxFlagList = NULL;
Event event;
Shipment *shipment = &event.shipment;

volatile BoardParams boardParams;      // 系统参数
volatile char strInput[30] = {0};      // 输入的字符串
volatile char flagInput = 0;           // 输入类型:    0:禁止输入;1:准备进入输入模式; 2:所有输入可用; 3:只键盘输入可用  4:蓝牙通讯中禁用键盘
volatile char flagCleadDB = 0;         // 清除数据类型 ‘P’: 清空PreReservation
volatile bool flagNewBarcode = true;   // false:非新订单 true:新订单
volatile bool flagPreReserved = false; // false:普通订单 true:预订订单
volatile char curFlow = StartupFlow;   // 当前流程:
volatile char curPage = 0;             // 当前数据所在页面
volatile u8 tickCheck = 0;             // 代表过了多少秒
volatile char bleEventType = BleEv_Unknow;

extern void test(void);

bool sys_init(bool flag)
{
    U8 ret = 0;
    // Byte *b = (Byte *)&ret;
    file_start();
    if (!flag)
    {
        e2p_GetFlag_Init(&ret);
        if (ret != 0x66)
            return true; // 不为0x66则不初始化
    }
    // 出厂设置
    if (!file_init())
        return false; // 初始化失败
    time_init();
    e2p_Init();
    return true;
}
// void checkTime(char d)
// {
//     if (time_day() != d)
//         if (time_hour() == boardParams.rebootTime)
//             bsp_Restart();
// }
void getAppParams(void)
{
    e2p_GetFloorCount(&boardParams.floorCount);
    e2p_GetApmID(boardParams.apmID);
    boardParams.apmID[10] = 0; // 字符串末尾字符
    boardParams.rebootTime = 1;
    e2p_GetAesKey(boardParams.aesKey);
    e2p_GetApiUrl(boardParams.apiurl);

    appFlag.firstsync = 1;
    appFlag.synclater = 0;
    appFlag.synclog = 0;
}
void init_Mutex(void) { os_mut_init(&mutex_e2p); }
void init_Semaphores(void)
{
    os_sem_init(&semaphore_uart1, 0);
    os_sem_init(&semaphore_uart2, 0);
    os_sem_init(&semaphore_uart3, 0);
    os_sem_init(&semaphore_uart4, 0);
    os_sem_init(&semaphore_uart5, 0);
}
void printAppStart(void)
{
    Time t;
    time_now(&t);
    printf("App-%d %s Start at 20%02d-%02d-%02d %02d:%02d:%02d.\n", flash_getAppFlag() + 1, VERSION, t.year, t.month, t.day, t.hour, t.minute, t.second);
    //	rtc_SetAlarmAfter(10);
}
__task void threadApp(void)
{
    while (1)
    {
        os_dly_wait(1);
        if (curFlow == StartupFlow)
        {
            printAppStart();
            sf_Config();
            sys_init(false);
            getAppParams();
            aes_Init();
            ble_Init();
            w5500_Init();
            db_init();
            test();
            curFlow = SyncFlow;
            os_dly_wait(500);
        }
        else if (curFlow == PrintShipmentFlow)
        {
            db_print();
            curFlow = IdleFlow;
        }
        else if (curFlow == AlarmWakeupFlow) // 闹钟唤醒处理
        {
            // curFlow = IdleFlow;
            // checkTime(curDay);
            flow_AlarmWakeup();
        }
        else if (curFlow == AppSwitchFlow) // 切换程序空间
            flow_AppSwitch();
        else if (curFlow == ClearDataFlow) // 清空数据
            flow_ClearData();
        else if (curFlow == PowerSaveFlow) // 进入省电模式
            flow_PowerSave();
        else if (curFlow == SyncFlow) // 数据同步
            flow_SyncAll();
        else if (curFlow == LoginFlow) // 登录判断
            flow_Login();
        else if (curFlow == CourierPickupFLow) // 快递员取件
            flow_CourierPickup();
        else if (curFlow == CourierDropoffFlow) // 快递员存件
            flow_CourierDropoff();
        else if (curFlow == DropoffFlow) // 实际存件流程
            flow_Dropoff();
        else if (curFlow == CustomerPickupFlow) // 用户取件
            flow_CustomerPickup();
        else if (curFlow == CustomerDropoffFlow) // 用户存件
            flow_CustomerDropoff();
        else if (curFlow == SuperTechnicianFlow) // 超级管理员
            flow_SuperTechnician();
        else if (curFlow == TechnicianFlow) // 普通管理员
            flow_TechnicianFlow();
        else if (curFlow == BleOpenLockFlow) // 蓝牙开门
            flow_BleOpenLock();
        else if (curFlow == BleConfirmFlow) // 蓝牙确认操作
            flow_BleConfirm();
        else if (curFlow == SyncEpraFlow) // 10分钟后同步EPRA信息
            flow_SyncEpra();
        else /* 待机 */
        {
            if (flagCleadDB == 'P')
            {
                flagCleadDB = 0;
                file_new(FN_PRERESER);
            }
            flow_Idle();
        }
    }
}
__task void threadUart(void)
{
    relayBoard_Init();
    while (1)
    { /* 串口通信 */
        uart_Handle();
    }
}
void init_Threads(void)
{
    os_tsk_create_user(threadApp,        /* 任务函数 */
                       1,                /* 任务优先级 */
                       &stkApp,          /* 任务栈 */
                       sizeof(stkApp));  /* 任务栈大小，单位字节数 */
    os_tsk_create_user(threadUart,       /* 任务函数 */
                       3,                /* 任务优先级 */
                       &stkUart,         /* 任务栈 */
                       sizeof(stkUart)); /* 任务栈大小，单位字节数 */
}
__task void threadInit(void)
{
    init_Mutex();
    init_Semaphores();
    init_Threads();
#ifdef FUNCTION_WATCHDOG
    iwdg_Start();
#endif // FUNCTION_WATCHDOG
    os_itv_set(500);
    while (1)
    { /* 逻辑处理 1秒处理1次*/
        os_itv_wait();
#ifdef FUNCTION_WATCHDOG
        iwdg_Feed();
#endif // FUNCTION_WATCHDOG
        powerOn_Led1;
        os_itv_wait();
        powerOff_Led1;
        tickCheck++;
        if (tickCheck == 200)
            bsp_Restart();
    }
}

/*----------------------------------------------------------------------------
 * Application main thread
 *---------------------------------------------------------------------------*/
int main(void)
{
    /* 设置中断向量表偏移 */
#ifdef VECT_TAB_RAM
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else /* VECT_TAB_FLASH  */
    // NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, flash_getOffsetAddress());
#endif
    /* 优先级分组设置为4 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    // System Initialization
    SystemCoreClockUpdate();
    /*  // Flash 设置
        FLASH_Unlock();
        FLASH_UserOptionByteConfig(OB_IWDG_SW, OB_STOP_NoRST, OB_ST DBY_NoRST);
        FLASH_Lock();
    */
    /* 硬件驱动配置 */
    bsp_Config();
#ifdef RTE_Compiler_EventRecorder
    // Initialize and start Event Recorder
    EventRecorderInitialize(EventRecordError, 1U);
#endif
    /* 创建启动任务 */
    os_sys_init_user(threadInit,       /* 任务函数 */
                     4,                /* 任务优先级 */
                     &stkInit,         /* 任务栈 */
                     sizeof(stkInit)); /* 任务栈大小，单位字节数 */
    while (1)
        ;
}
