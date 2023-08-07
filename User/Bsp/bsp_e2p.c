#include "bsp_e2p.h"
#include "W5500/utility.h"

extern void bsp_delay(int n);

#define AT24CXX_Delay() bsp_delay(10)	// __NOP // delay_us(1)
#define AT24CXX_Delay2() os_dly_wait(5)

#define WRITE_AT24CXX_CMD ((unsigned char)0XA0)
#define READ_AT24CXX_CMD ((unsigned char)0xA1)
#define WRITE_AT24CXX_CMD1 ((unsigned char)0XAE)
#define READ_AT24CXX_CMD1 ((unsigned char)0xAF)

void e2p_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = AT24CXX_SCL_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(AT24CXX_PORT, &GPIO_InitStructure);
    //配置为开漏模式，此模式下可以实现真下的双向IO
    GPIO_InitStructure.GPIO_Pin = AT24CXX_SDA_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(AT24CXX_PORT, &GPIO_InitStructure);
}
/**
 * @brief 开始信号：SCL 为高电平时，SDA 由高电平向低电平跳变，开始传送数据。
 */
void AT24CXX_Start()
{
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
    AT24CXX_Delay();
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
}
void AT24CXX_Stop()
{
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
}
void AT24CXX_ACK(void)
{
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
    AT24CXX_Delay();
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
}
void AT24CXX_NoACK(void)
{
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
    AT24CXX_Delay();
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
}

bool AT24CXX_WaitACK(void)
{
    BOOL bitStatus;
    AT24CXX_Delay();
    GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    bitStatus = true;
    if (GPIO_ReadInputDataBit(AT24CXX_PORT, AT24CXX_SDA_IO) != 0)
    {
        bitStatus = false; //__FALSE;
    }
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    AT24CXX_Delay();
    return bitStatus;
}

bool AT24CXX_WriteData(unsigned char data)
{
    bool ret;
    unsigned char count;
    GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
    for (count = 8; count > 0; count--)
    {
        if (data & 0x80)
        {
            GPIO_SetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
        }
        else
        {
            GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SDA_IO);
        }
        AT24CXX_Delay();
        GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
        AT24CXX_Delay();
        GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
        data <<= 1; // d左移一位
    }
    ret = AT24CXX_WaitACK();
    return ret;
}

unsigned char AT24CXX_ReadData(void)
{
    unsigned char count, rbyte = 0;
    for (count = 8; count > 0; count--)
    {
        GPIO_SetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
        rbyte <<= 1;
        AT24CXX_Delay();
        if (GPIO_ReadInputDataBit(AT24CXX_PORT, AT24CXX_SDA_IO) != 0)
        {
            rbyte |= 0x01; // SDA高电平
        }
        GPIO_ResetBits(AT24CXX_PORT, AT24CXX_SCL_IO);
        AT24CXX_Delay();
    }
    return rbyte;
}

/*************************************************************
 *函数名 Function Name: void e2p_WriteByte(unsigned int address,unsigned char data)
 *函数功能 Description: 向AT24CXX内的某个地址，写一个字节
 *输入参数 Parameters : None
 *输出参数Return Value: None
 *注意事项  Attention ：
 **************************************************************/
void e2p_WriteByte(U16 address, U8 data)
{
    OS_RESULT oret = os_mut_wait(&mutex_e2p, OS_FOREVER);
    if ((oret != OS_R_OK) && (oret != OS_R_MUT))
        return;

    AT24CXX_Start();
    AT24CXX_WriteData(WRITE_AT24CXX_CMD);
    AT24CXX_WriteData(address >> 8);
    AT24CXX_WriteData(address & 0x00ff);
    AT24CXX_WriteData(data);
    AT24CXX_Stop();
    AT24CXX_Delay2();
    os_mut_release(&mutex_e2p);
}

/*************************************************************
 *函数名 Function Name: unsigned char e2p_ReadByte(unsigned int address)
 *函数功能 Description: 从AT24CXX内的某个地址，读一个字节
 *输入参数 Parameters : None
 *输出参数Return Value: None
 *注意事项  Attention ：
 **************************************************************/
U8 e2p_ReadByte(U16 address)
{
    OS_RESULT oret = os_mut_wait(&mutex_e2p, OS_FOREVER);
    if ((oret != OS_R_OK) && (oret != OS_R_MUT))
        return NULL;

    U8 retBuf;
    AT24CXX_Start();
    AT24CXX_WriteData(WRITE_AT24CXX_CMD);
    AT24CXX_WriteData(address >> 8);
    AT24CXX_WriteData(address & 0x00ff);
    AT24CXX_Start();
    AT24CXX_WriteData(READ_AT24CXX_CMD);
    retBuf = AT24CXX_ReadData();
    AT24CXX_NoACK();
    AT24CXX_Stop();
    os_mut_release(&mutex_e2p);
    return retBuf;
}

/*************************************************************
*函数名 Function Name: void e2p_WriteBytes(unsigned int address,unsigned char *pData,unsigned char byteCount)
*函数功能 Description: 写X个字节，从AT24CXX内的某个地址开始
*输入参数 Parameters :
*输出参数Return Value: None
*注意事项  Attention ：
       对于2401的n<=8,对于2402,2404,2408,2416的n<=16.  以此类推24256<=64
       调用时，数组前还要加"&",即	e2p_WriteBytes(0, (unsigned char *)&GetBuf[10],9);
**************************************************************/
void e2p_WriteBytes(U16 address, U8 *pData, U16 byteCount)
{
    OS_RESULT oret = os_mut_wait(&mutex_e2p, OS_FOREVER);
    if ((oret != OS_R_OK) && (oret != OS_R_MUT))
        return;

    bool ret;
    AT24CXX_Start();                      //启动
    AT24CXX_WriteData(WRITE_AT24CXX_CMD); //写入器件的寻址地址
    AT24CXX_WriteData(address >> 8);      //写入器件的字节地址
    AT24CXX_WriteData(address & 0x00ff);  //写入器件的字节地址
    for (; byteCount > 0; byteCount--)
    {
        AT24CXX_WriteData(*pData); //写入器件的字节首地址
        pData++;
        address++;
        if (((address + 64) % 64) == 0)
        {
            AT24CXX_Stop(); //停止
            AT24CXX_Delay2();
            AT24CXX_Start();                      //启动
            AT24CXX_WriteData(WRITE_AT24CXX_CMD); //写入器件的寻址地址
            AT24CXX_WriteData(address >> 8);      //写入器件的字节地址
            AT24CXX_WriteData(address & 0x00ff);  //写入器件的字节地址
        }
    }
    AT24CXX_Stop();
    AT24CXX_Delay2();
    AT24CXX_WaitACK();
    os_mut_release(&mutex_e2p);
}
/*************************************************************
*函数名 Function Name: void e2p_ReadBytes(unsigned int address,unsigned char *pData,unsigned char byteCount)
*函数功能 Description: 读X个字节，从AT24CXX内的某个地址开始
*输入参数 Parameters :
*输出参数Return Value: None
*注意事项  Attention ：
       调用时，数组前还要加"&",即	e2p_ReadBytes(0, (unsigned char *)&RecBuf[10],9);
**************************************************************/
void e2p_ReadBytes(U16 address, U8 *pData, U16 byteCount)
{
    OS_RESULT oret = os_mut_wait(&mutex_e2p, OS_FOREVER);
    if ((oret != OS_R_OK) && (oret != OS_R_MUT))
        return;

    AT24CXX_Start();
    AT24CXX_WriteData(WRITE_AT24CXX_CMD);
    AT24CXX_WriteData(address >> 8);
    AT24CXX_WriteData(address & 0x00ff);
    AT24CXX_Start();
    AT24CXX_WriteData(READ_AT24CXX_CMD);
    for (; byteCount != 1; byteCount--)
    {
        *pData = AT24CXX_ReadData();
        AT24CXX_ACK();
        pData++;
    }
    *pData = AT24CXX_ReadData();
    AT24CXX_NoACK();
    AT24CXX_Stop();
    AT24CXX_Delay2();
    AT24CXX_WaitACK();
    os_mut_release(&mutex_e2p);
}

void WriteAT24StrZero(U16 address, U16 n)
{
    AT24CXX_Start();                      //启动
    AT24CXX_WriteData(WRITE_AT24CXX_CMD); //写入器件的寻址地址
    AT24CXX_WriteData(address >> 8);      //写入器件的字节地址
    AT24CXX_WriteData(address & 0x00ff);  //写入器件的字节地址
    for (; n > 0; n--)
    {
        AT24CXX_WriteData(0); //写入器件的字节首地址
    }
    AT24CXX_Stop(); //停止
}

void AT24CXX_WriteOneByte1(unsigned int address, unsigned char data)
{
    AT24CXX_Start();
    AT24CXX_WriteData(WRITE_AT24CXX_CMD1);
    AT24CXX_WriteData(address >> 8);
    AT24CXX_WriteData(address & 0x00ff);
    AT24CXX_WriteData(data);
    AT24CXX_Stop();
}

/*************************************************************
 *函数名 Function Name: unsigned char e2p_ReadByte(unsigned int address)
 *函数功能 Description: 从AT24CXX内的某个地址，读一个字节
 *输入参数 Parameters : None
 *输出参数Return Value: None
 *注意事项  Attention ：
 **************************************************************/
unsigned char AT24CXX_ReadOneByte1(unsigned int address)
{
    unsigned char retBuf;
    AT24CXX_Start();
    AT24CXX_WriteData(WRITE_AT24CXX_CMD1);
    AT24CXX_WriteData(address >> 8);
    AT24CXX_WriteData(address & 0x00ff);

    AT24CXX_Start();
    AT24CXX_WriteData(READ_AT24CXX_CMD1);
    retBuf = AT24CXX_ReadData();
    AT24CXX_NoACK();
    AT24CXX_Stop();
    return retBuf;
}

/*************************************************************
*函数名 Function Name: void e2p_WriteBytes(unsigned int address,unsigned char *pData,unsigned char byteCount)
*函数功能 Description: 写X个字节，从AT24CXX内的某个地址开始
*输入参数 Parameters :
*输出参数Return Value: None
*注意事项  Attention ：
       对于2401的n<=8,对于2402,2404,2408,2416的n<=16.  以此类推24256<=64
       调用时，数组前还要加"&",即	e2p_WriteBytes(0, (unsigned char *)&GetBuf[10],9);
**************************************************************/
void AT24CXX_WriteSomeByte1(unsigned int address, unsigned char *pData, unsigned char byteCount)
{
    AT24CXX_Start();                       //启动
    AT24CXX_WriteData(WRITE_AT24CXX_CMD1); //写入器件的寻址地址
    AT24CXX_WriteData(address >> 8);       //写入器件的字节地址
    AT24CXX_WriteData(address & 0x00ff);   //写入器件的字节地址
    for (; byteCount > 0; byteCount--)
    {
        AT24CXX_WriteData(*pData); //写入器件的字节首地址
        pData++;
        address++;
        if (((address + 64) % 64) == 0)
        {
            AT24CXX_Stop(); //停止
            // SoftWareDelayMs(4);
            AT24CXX_Delay();
            AT24CXX_Start();                       //启动
            AT24CXX_WriteData(WRITE_AT24CXX_CMD1); //写入器件的寻址地址
            AT24CXX_WriteData(address >> 8);       //写入器件的字节地址
            AT24CXX_WriteData(address & 0x00ff);   //写入器件的字节地址
        }
    }
    AT24CXX_Stop();
}
/*************************************************************
*函数名 Function Name: void e2p_ReadBytes(unsigned int address,unsigned char *pData,unsigned char byteCount)
*函数功能 Description: 读X个字节，从AT24CXX内的某个地址开始
*输入参数 Parameters :
*输出参数Return Value: None
*注意事项  Attention ：
       调用时，数组前还要加"&",即	e2p_ReadBytes(0, (unsigned char *)&RecBuf[10],9);
**************************************************************/
void AT24CXX_ReadSomeByte1(unsigned int address, unsigned char *pData, unsigned char byteCount)
{
    AT24CXX_Start();
    AT24CXX_WriteData(WRITE_AT24CXX_CMD1);
    AT24CXX_WriteData(address >> 8);
    AT24CXX_WriteData(address & 0x00ff);
    AT24CXX_Start();
    AT24CXX_WriteData(READ_AT24CXX_CMD1);
    for (; byteCount != 1; byteCount--)
    {
        *pData = AT24CXX_ReadData();
        AT24CXX_ACK();
        pData++;
    }
    *pData = AT24CXX_ReadData();
    AT24CXX_NoACK();
    AT24CXX_Stop();
}

void WriteAT24StrZero1(U16 address, U16 n)
{
    AT24CXX_Start();                       //启动
    AT24CXX_WriteData(WRITE_AT24CXX_CMD1); //写入器件的寻址地址
    AT24CXX_WriteData(address >> 8);       //写入器件的字节地址
    AT24CXX_WriteData(address & 0x00ff);   //写入器件的字节地址
    for (; n > 0; n--)
    {
        AT24CXX_WriteData(0); //写入器件的字节首地址
    }
    AT24CXX_Stop(); //停止
}
