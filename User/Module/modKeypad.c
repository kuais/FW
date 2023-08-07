#include "modKeypad.h"
#include "modMainBoard.h"
#include "flow.h"

static U8 keyLast = 0;

U8 keypad_scan()
{
    U8 keyBuf;
    U8 index = 0;
    powerOff_Key1;
    powerOn_Key2;
    powerOn_Key3;
    keyBuf = keypad_getInput(index); // 检测第3列
    if (keyBuf == 0)
    {
        index++;
        powerOn_Key1;
        powerOff_Key2;
        keyBuf = keypad_getInput(index); // 检测第2列
    }
    if (keyBuf == 0)
    {
        index++;
        powerOn_Key2;
        powerOff_Key3;
        keyBuf = keypad_getInput(index); // 检测第1列
    }
    return keyBuf;
}

void keypad_Handle(void)
{
    if (flagInput == 4)
        return;
    U8 keyBuf = keypad_scan();
    if (keyBuf == 0)
    {
        keyLast = 0;
        return; // 未检测到按键
    }
    if (keyLast == keyBuf)
        return; // 长按不重复提交
    /* 检测到按键 */
    tickCheck = 0;
    keyLast = keyBuf;
    if (keyBuf == 11)
        keyBuf = 0;   // 按键 0
    if (keyBuf == 10) /* Clear */
    {
        mainBoard_KeyInput(keyBuf);
        if (flagInput == 0)
        {
            if (curFlow == IdleFlow)
                flagInput = 1;
            else
                return;
        }
        else if (flagInput == 2)
            clearInput();
    }
    else if (keyBuf == 12) /* Enter */
    {
        if (flagInput < 2)
            return;
        mainBoard_KeyInput(keyBuf);
        if (strlen(strInput) > 0)
        {
            if (curFlow == IdleFlow)
                curFlow = LoginFlow;
            else if (curFlow == CourierDropoffFlow)
                curFlow = DropoffFlow;
        }
    }
    else
    {
        if (flagInput < 2)
            return;
        if (strlen(strInput) == 29)
            return; // 达到字数上限
        mainBoard_KeyInput(keyBuf);
        if (curFlow == IdleFlow)
        {
            sprintf(strInput, "%s%d", strInput, keyBuf);
            if (strlen(strInput) == 29)
                curFlow = LoginFlow;
        }
        else if (curFlow == CourierDropoffFlow)
        {
            sprintf(strInput, "%s%d", strInput, keyBuf);
            if (strlen(strInput) == 29)
                curFlow = DropoffFlow;
        }
        else if (curFlow == DropoffFlow || curFlow == CustomerDropoffFlow ||
                 curFlow == CourierPickupFLow || curFlow == BleOpenLockFlow)
        {
            if (keyBuf > (XL + 1))
                return; // 只接受0,1,2,3,4,5
            strInput[0] = 0x30 + keyBuf;
            flagInput = 0;
        }
    }
    bsp_Beep(100);
}
