#include "bsp_ds1302.h"

/*---------------constant define-----------------
-----------------------------------------------*/
#define ht1380_DISWP 0X00           // 关写保护
#define ht1380_ENWP 0X80            // 开写保护
#define ht1380_BURST_WRITE_CMD 0XBE // 连续写命令
#define ht1380_BURST_READ_CMD 0XBF  // 连续读命令

#define DS1302_Delay() __NOP()

void ds1302_Config()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DS1302_SCLK | DS1302_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure);
    /* 配置为开漏模式，此模式下可以实现真的双向IO */
    GPIO_InitStructure.GPIO_Pin = DS1302_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(DS1302_PORT, &GPIO_InitStructure);
}

/**
 * @brief           写入1byte
 * @param data      数据
 */
void ds1302_Out(U8 data)
{
    U8 i;
    for (i = 8; i > 0; i--)
    {
        if (data & 0x01)
            GPIO_SetBits(DS1302_PORT, DS1302_IO);
        else
            GPIO_ResetBits(DS1302_PORT, DS1302_IO);
        data >>= 1;
        GPIO_SetBits(DS1302_PORT, DS1302_SCLK);
        DS1302_Delay();
        GPIO_ResetBits(DS1302_PORT, DS1302_SCLK);
    }
}

/**
 * @brief           读出1Byte
 * @return U8       数据
 */
U8 ds1302_In(void)
{
    U8 i;
    U8 retBuf = 0;
    for (i = 8; i > 0; i--)
    {
        retBuf >>= 1;
        if (GPIO_ReadInputDataBit(DS1302_PORT, DS1302_IO) != 0)
        {
            retBuf |= 0x80;
        }
        GPIO_SetBits(DS1302_PORT, DS1302_SCLK);
        DS1302_Delay();
        GPIO_ResetBits(DS1302_PORT, DS1302_SCLK);
    }
    //	ht1380_IO_CTRL=0;
    return retBuf;
}

/**
 * @brief               写入8字节数据
 * @param buf           要写入的数据
 */
void ds1302_WriteBytes(U8 *buf)
{
    U8 i;
    GPIO_ResetBits(DS1302_PORT, DS1302_RST);
    GPIO_SetBits(DS1302_PORT, DS1302_RST);
    ds1302_Out(0x8e);
    ds1302_Out(ht1380_DISWP);
    GPIO_ResetBits(DS1302_PORT, DS1302_RST);
    GPIO_SetBits(DS1302_PORT, DS1302_RST);
    ds1302_Out(ht1380_BURST_WRITE_CMD);
    for (i = 0; i < 7; i++)
    {
        ds1302_Out(buf[i]);
    }
    ds1302_Out(0x8e);
    ds1302_Out(ht1380_ENWP);
    GPIO_ResetBits(DS1302_PORT, DS1302_RST);
}

/**
 * @brief               读出一组数据
 * @param buf     读出的数据
 */
void ds1302_ReadBytes(U8 *buf)
{
    U8 i;
    GPIO_ResetBits(DS1302_PORT, DS1302_RST);
    GPIO_SetBits(DS1302_PORT, DS1302_RST);
    ds1302_Out(ht1380_BURST_READ_CMD);
    for (i = 0; i < 7; i++)
    {
        buf[i] = ds1302_In();
    }
    GPIO_ResetBits(DS1302_PORT, DS1302_RST);
}
