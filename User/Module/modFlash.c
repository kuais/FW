#include "modFlash.h"

#define Flash_PageSize 0x0800 // F103RE			???????2K???
#define Flash_AddrBegin 0x08000000
#define Flash_AddrEnd 0x0807FFFF        // FLASH???			0x08000000 - 0x0807FFFF
#define Flash_AddrBootLoader 0x08000000 // BootLoader???	0x08000000 - 0x08001FFF
#define Flash_AddrApp1 0x08004000       // App1???			0x08004000 - 0x0803FFFF
#define Flash_AddrApp2 0x08044000       // App2???			0x08044000 - 0x0807FFFF
#define Flash_AddrAppflag 0x08040000    //????App??? 		0: App1   1??App2

/**
 * 从Flash指定地址address开始读取n个字节数据到buffer
 * @param address 起始地址
 * @param buffer  读出的数据
 * @param n       读出的字节数
 */
void flash_read(uint32_t address, uint8_t *buffer, uint32_t count)
{
    uint32_t i = 0;
    while (i < count)
    {
        *(buffer++) = *(__IO uint8_t *)address++;
        i++;
    }
}

/**
 * 向Flash指定地址address开始写入n个字节数据
 * @param address 起始地址
 * @param buffer  写入的数据
 * @param n       写入的字节数
 */
void flash_write(uint32_t address, uint8_t *buffer, uint32_t count)
{
    uint32_t i;
    uint32_t *temp;
    uint32_t addr_Page = (address / Flash_PageSize) * Flash_PageSize;
    uint8_t pagedata[Flash_PageSize] = {0};
    uint32_t offset = address - addr_Page;

    // 读出原扇区数据
    flash_read(addr_Page, pagedata, Flash_PageSize);
    // 更新扇区数据
    i = 0;
    while (i < count)
    {
        pagedata[offset + i] = buffer[i];
        i++;
    }
    // 擦除原扇区数据
    FLASH_SetLatency(FLASH_Latency_1);
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(address);
    // 写入新扇区数据
    for (i = 0; i < Flash_PageSize; i += 4)
    {
        temp = (uint32_t *)(pagedata + i);
        FLASH_ProgramWord(addr_Page, *temp);
        while (FLASH_GetStatus() != FLASH_COMPLETE)
            __NOP();
        addr_Page += 4;
    }
    FLASH_Lock();
}

/**
 * 获取当前使用的App的标志
 * @return  0-App1, 1-App2
 */
uint8_t flash_getAppFlag(void)
{
    uint8_t result;
    flash_read(Flash_AddrAppflag, &result, 1);
    if ((result != (uint8_t)1) && (result != (uint8_t)0))
    {
        result = 0;
        flash_setAppFlag(0);
    }
    return result;
}
/**
 * 设置将使用的App的标志
 * @param value 0-App1, 1-App2
 */
void flash_setAppFlag(uint8_t value) { flash_write(Flash_AddrAppflag, &value, 1); }
/**
 * 获取当前App的偏移地址，用于设置中断向量表
 * @return  当前App的偏移地址
 */
uint32_t flash_getOffsetAddress(void)
{
    //	return 0;
    switch (flash_getAppFlag())
    {
    case 1:
        return Flash_AddrApp2 - Flash_AddrBegin;
    case 0:
    default:
        return Flash_AddrApp1 - Flash_AddrBegin;
    }
}
/**
 * 获取App的起始地址
 * @return  App的起始地址
 */
uint32_t flash_getAppAddress(void)
{
    switch (flash_getAppFlag())
    {
    case 0:
        return Flash_AddrApp1;
    case 1:
    default:
        return Flash_AddrApp2;
    }
}

/**
 * 获取App的更新起始地址
 * @return  App的更新起始地址
 */
uint32_t flash_getUpdateBaseAddress(void)
{
    switch (flash_getAppFlag())
    {
    case 0:
        return Flash_AddrApp2;
    case 1:
    default:
        return Flash_AddrApp1;
    }
}

void flash_writeApp(uint32_t offset, uint8_t *buffer, uint16_t count)
{
    uint32_t address, intTemp;

    address = flash_getUpdateBaseAddress() + offset;
    intTemp = Flash_PageSize - (offset % Flash_PageSize); // 当前扇区剩余空间
    if (intTemp < count)
    {                                          // 跨扇区了
        flash_write(address, buffer, intTemp); // 将数据写入Flash, 当前扇区
        address += intTemp;
        buffer += intTemp;
        intTemp = count - intTemp;             // 剩下的数据
        flash_write(address, buffer, intTemp); // 将数据写入Flash，下一个扇区
    }
    else
    {
        flash_write(address, buffer, count); // 将数据写入Flash
    }
}

/**
 * 将更新App程序数据写入Flash, 自定协议
 * @param buffer [description]
 */
void flash_updateApp(uint8_t *buffer)
{
    uint32_t offset;
    uint16_t count;

    offset = *((uint32_t *)(++buffer)); // 当前写入地址
    buffer += 4;
    count = *((uint16_t *)(buffer)); // 获取当前写入的数据长度
    buffer += 2;
    flash_writeApp(offset, buffer, count);
}
