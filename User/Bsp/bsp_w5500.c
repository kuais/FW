#include "bsp_w5500.h"
#include "W5500/utility.h"
#include "W5500/w5500_interface.h"
#include "bsp_spi.h"

#define w5500_CsOff spi1_CsOff
#define w5500_CsOn spi1_CsOn
#define w5500_Write spi1_Write
#define w5500_Read spi1_Read

void w5500_Reset(void)
{
    GPIO_ResetBits(W5500_PORT, W5500_PINRST);
    os_dly_wait(100);
    GPIO_SetBits(W5500_PORT, W5500_PINRST);
    os_dly_wait(2000);
}
/**
 * @brief w5500 GPIO Config
 */
void w5500_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* RESET */
    GPIO_InitStructure.GPIO_Pin = W5500_PINRST;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(W5500_PORT, &GPIO_InitStructure);
    /* INT */
    GPIO_InitStructure.GPIO_Pin = W5500_PININT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(W5500_PORT, &GPIO_InitStructure);
}

/**
 * @brief   向W5500写入1个数据
 * @param addr 地址
 * @param data 数据
 */
void w5500_WriteByte(u32 addr, u8 data)
{
    w5500_CsOn();
    w5500_Write((addr & 0x00FF0000) >> 16);
    w5500_Write((addr & 0x0000FF00) >> 8);
    w5500_Write((addr & 0x000000F8) + 4);
    w5500_Write(data);
    w5500_CsOff();
}

/**
 * @brief 从W5500读出一个8位数据
 * @param addr 地址
 * @return 读取到的8位数据
 */
u8 w5500_ReadByte(u32 addr)
{
    u8 data = 0;
    w5500_CsOn();
    w5500_Write((addr & 0x00FF0000) >> 16);
    w5500_Write((addr & 0x0000FF00) >> 8);
    w5500_Write((addr & 0x000000F8));
    data = w5500_Read();
    w5500_CsOff();
    return data;
}

/**
 * @brief 向W5500写入数据
 * @param addr 地址
 * @param buf  数据缓存
 * @param len  数据长度
 */
void w5500_WriteBytes(u32 addr, u8 *buf, u16 len)
{
    if (len == 0)
        printf("Unexpected length 0\r\n");
    else
    {
        w5500_CsOn();
        w5500_Write((addr & 0x00FF0000) >> 16);
        w5500_Write((addr & 0x0000FF00) >> 8);
        w5500_Write((addr & 0x000000F8) + 4);
        for (u16 idx = 0; idx < len; idx++)
            w5500_Write(buf[idx]);
        w5500_CsOff();
    }
}

/**
 * @brief 从W5500读出len长度数据
 * @param addr 起始地址
 * @param buf  数据缓存
 * @param len  数据长度
 * @return uint16_t 实际读出的长度
 */
u16 w5500_ReadBytes(u32 addr, u8 *buf, u16 len)
{
    if (len == 0)
        printf("Unexpected2 length 0\r\n");
    else
    {
        w5500_CsOn();
        w5500_Write((addr & 0x00FF0000) >> 16);
        w5500_Write((addr & 0x0000FF00) >> 8);
        w5500_Write((addr & 0x000000F8));
        for (u16 idx = 0; idx < len; idx++)
            buf[idx] = w5500_Read();
        w5500_CsOff();
    }
    return len;
}
