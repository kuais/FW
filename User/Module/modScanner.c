#include "modScanner.h"
#include "modMainBoard.h"

#define SC_DEBUG 0
#define SC_OUT uart1_Send

static char *curCmd = NULL;
static bool isWaitRet = false;

static void waitRetEnd(void)
{
    if (curCmd != NULL)
    {
        myfree(curCmd);
        curCmd = NULL;
        isWaitRet = false;
    }
}
static BOOL waitRet()
{
    BOOL ret = false;
    int timeout = 2000;
    isWaitRet = true;
    while (timeout > 0)
    {
        if (!isWaitRet)
        {
            ret = true;
            break;
        }
        timeout -= 20;
        os_dly_wait(20);
    }
    waitRetEnd();
    return ret;
}
static U8 *pack(U8 *p)
{
    U8 len = strlen(p);
    U8 *ret = mymalloc(len + 3);
    ret[0] = 0x16;
    ret[1] = 0x4D; // 'M'
    ret[2] = 0x0D;
    memcpy(&ret[3], p, len);
    return ret;
}
static void scanner_cmdOfSetCodeEnable(char *cmd, char *codetype, char flag)
{
    sprintf(cmd, "%sENA%d.", codetype, flag ? 1 : 0);
}
static void scanner_cmdOfSetCodeMin(char *cmd, char *codetype, U16 length)
{
    if ((strcmp(codetype, "QRC") == 0) || (strcmp(codetype, "IDM") == 0))
        sprintf(cmd, "%sMIN%04d.", codetype, length);
    else if (strcmp(codetype, "MAX") == 0)
        sprintf(cmd, "%sMIN%03d.", codetype, length);
    else
        sprintf(cmd, "%sMIN%02d.", codetype, length);
}
static void scanner_cmdOfSetCodeMax(char *cmd, char *codetype, U16 length)
{
    if ((strcmp(codetype, "QRC") == 0) || (strcmp(codetype, "IDM") == 0))
        sprintf(cmd, "%sMAX%04d.", codetype, length);
    else if (strcmp(codetype, "MAX") == 0)
        sprintf(cmd, "%sMAX%03d.", codetype, length);
    else
        sprintf(cmd, "%sMAX%02d.", codetype, length);
}
/**
 * @brief 扫描仪闪灯
 *
 * @param count 闪灯次数
 */
void scanner_Blink(char count, u16 interval)
{
    while (count-- > 0)
    {
        os_dly_wait(interval);
        scanner_SwitchLed(1);
        os_dly_wait(interval);
        scanner_SwitchLed(0);
    }
}
void scanner_Activate(void)
{
    char datas[3] = {0};
    datas[0] = 0x16;
    datas[1] = 'T'; // 'T'
    datas[2] = 0x0D;
    SC_OUT(datas, 3);
}
void scanner_Deactivate(void)
{
    char datas[3] = {0};
    datas[0] = 0x16;
    datas[1] = 'U'; // 'T'
    datas[2] = 0x0D;
    SC_OUT(datas, 3);
}
BOOL scanner_SendMenu(char *cmd)
{
    U8 *datas = pack(cmd);
    SC_OUT(datas, strlen(cmd) + 3);
#if SC_DEBUG
    printf("%s\r\n", cmd);
#endif
    myfree(datas);
    if (curCmd == NULL)
        curCmd = mymalloc(6);
    memcpy(curCmd, cmd, 6);
    return waitRet();
}
BOOL scanner_SendMenuNoWait(char *cmd)
{
    U8 *datas = pack(cmd);
    SC_OUT(datas, strlen(cmd) + 3);
    myfree(datas);
    return true;
}
/**
 * @brief  开关LED
 *
 * @param flag 0:关灯   1:开灯
 */
BOOL scanner_SwitchLed(char flag)
{
    char arr[9] = {0};
    sprintf(arr, "SCNLED%d.", flag ? 1 : 0);
    return scanner_SendMenu(arr);
}
BOOL scanner_SwitchLight(char flag)
{
    char arr[11] = {0};
    if (flag == 0)
        strcpy(arr, "PWRLDC0.");
    else if (flag == 1)
        strcpy(arr, "PWRLDC100.");
    else
        strcpy(arr, "PWRLDC150.");
    // strcpy(arr, "PAPSPN.");
    return scanner_SendMenu(arr);
}
BOOL scanner_PhoneMode(char flag)
{
    char arr[9] = {0};
    if (flag)
        strcpy(arr, "PAPSPC.");
    else
        strcpy(arr, "PAPSPN.");
    return scanner_SendMenu(arr);
}
/**
 * @brief 设置扫描灯
 * @param flag      0:off;1:on,2:interlaced
 * @return BOOL
 */
BOOL scanner_AimMode(char flag)
{
    char arr[9] = {0};
    sprintf(arr, "%s%d.", "SCNAIM", flag);
    return scanner_SendMenu(arr);
}
/**
 * @brief  开关所有规则
 * @param flag 0:禁用所有规则， 1:启用所有规则
 */
BOOL scanner_AllRuleEnable(char flag)
{
    char arr[9] = {0};
    scanner_cmdOfSetCodeEnable(arr, "ALL", flag);
    return scanner_SendMenu(arr);
}
BOOL scanner_RuleEnable(LabelRule *rule)
{
    BOOL ret = false;
    char *codetype = rule->codetype;
    char arr[13] = {0};
    /* Set codetype enable */
    scanner_cmdOfSetCodeEnable(arr, codetype, 1);
    if (!scanner_SendMenu(arr))
        goto exit;
    if ((strcmp(codetype, "UPA") == 0) ||
        (strcmp(codetype, "UPE") == 0) ||
        (strcmp(codetype, "E13") == 0) ||
        (strcmp(codetype, "EA8") == 0))
    {
        goto exit;
    }
    /* Set codetype max length */
    scanner_cmdOfSetCodeMax(arr, codetype, rule->pos1);
    if (!scanner_SendMenu(arr))
        goto exit;
    /* Set codetype min length */
    scanner_cmdOfSetCodeMin(arr, codetype, rule->pos0);
    if (!scanner_SendMenu(arr))
        goto exit;
    ret = true;
exit:
    return ret;
}

static void getInput(u8 *p, int len)
{ // 过滤无效字符
    int i = 0;
    clearInput();
    for (; i < len; i++)
    {
        if (p[i] < 0x20 || p[i] > 0x7F || p[i] == ';')
            break;
        strInput[i] = p[i];
    }
    printf("Scanner input %s\r\n", strInput);
}

static int checkCmd(u8 *p, int len)
{
    int i = 0;
    for (; i < len; i++)
    {
        if (memcmp(curCmd, &p[i], 6) == 0)
            return i;
    }
    return -1;
}

void scanner_Handle(UART_T *port)
{
    KuBuffer *buf = port->bufRx;
    int len0 = buffer_dataCount(buf);
    if (len0 <= 0)
        return;
    u8 *p = buffer_get(buf, 0);
    buffer_remove(buf, len0);
#if SC_DEBUG
    printf("sc recv: ");
    for (int i = 0; i < len0; i++)
        printf("%c", *(p + i));
    printf("\r\n");
#endif
    if (curCmd)
    {
        int pos = checkCmd(p, len0);
        if (pos >= 0)
        { // 处理指令回复
            waitRetEnd();
        }
        else
            return;
    }
    else
    {
        mainBoard_SendBarcode(p, len0);
        if (flagInput < 2)
        { // 禁止输入
        }
        else
        {
            tickCheck = 0;
            if (curFlow == IdleFlow)
            {
                getInput(p, len0);
                curFlow = LoginFlow;
            }
            else if (curFlow == CourierDropoffFlow)
            {
                getInput(p, len0);
                curFlow = DropoffFlow;
            }
        }
    }
}
