#include "bsp_keypad.h"
#include "bsp_uart.h"
#include "modBle.h"
#include "modMainBoard.h"
#include "modRelayBoard.h"
#include "modScanner.h"
#include "modKeypad.h"
#include "os.h"

void uart1_Recv(void)
{
    OS_RESULT oret = os_sem_wait(&semaphore_uart1, 10);
    if ((oret != OS_R_OK) && (oret != OS_R_SEM))
        return;
    scanner_Handle(&g_tUart1);
}
void uart2_Recv(void)
{
    OS_RESULT oret = os_sem_wait(&semaphore_uart2, 10);
    if ((oret != OS_R_OK) && (oret != OS_R_SEM))
        return;
    if (!appSet.relayboard)
    {
        appSet.relayboard = 1;
        powerOn_RelayBoard; // 串口调试时启动锁板
        os_dly_wait(1500);
    }
    mainBoard_Handle(&g_tUart2);
}
void uart3_Recv(void)
{
    OS_RESULT oret = os_sem_wait(&semaphore_uart3, 10);
    if ((oret != OS_R_OK) && (oret != OS_R_SEM))
        return;
    relayBoard_Handle(&g_tUart3);
}
void uart4_Recv(void)
{
    OS_RESULT oret = os_sem_wait(&semaphore_uart4, 10);
    if ((oret != OS_R_OK) && (oret != OS_R_SEM))
        return;
    relayBoard_Handle(&g_tUart4);
}
void uart5_Recv(void)
{
    OS_RESULT oret = os_sem_wait(&semaphore_uart5, 10);
    if ((oret != OS_R_OK) && (oret != OS_R_SEM))
        return;
    ble_Handle(&g_tUart5);
}
void uart_Handle(void)
{
    keypad_Handle(); /* keypad */
    os_dly_wait(10);
    relayBoard_Task(); /* uart3 & uart4 */
    os_dly_wait(10);
    ble_Task(); /* uart5  */
    os_dly_wait(10);
    uart1_Recv();
    os_dly_wait(10);
    uart2_Recv();
    os_dly_wait(10);
    uart3_Recv();
    os_dly_wait(10);
    uart4_Recv();
    os_dly_wait(10);
    uart5_Recv();
    os_dly_wait(10);
}
