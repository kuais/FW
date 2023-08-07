#include "bsp.h"

#define sf_CsOff spi2_CsOff
#define sf_CsOn spi2_CsOn
#define sf_Write spi2_Write
#define sf_Read spi2_Read
#define sf_Delay __NOP()

#define CMD_AAI 0xAD  /* AAI 连续编程指令(FOR SST25VF016B) */
#define CMD_WRDI 0x04 /* 禁止写, 退出AAI状态 */
#define CMD_EWSR 0x50 /* 允许写状态寄存器的命令 */
#define CMD_WRSR 0x01 /* 写状态寄存器命令 */
#define CMD_WREN 0x06 /* 写使能命令 */
#define CMD_READ 0x03 /* 读数据区命令 */
#define CMD_RDSR 0x05 /* 读状态寄存器命令 */
#define CMD_RDID 0x9F /* 读器件ID命令 */
#define CMD_SE 0x20   /* 擦除扇区命令 */
#define CMD_SE2 0x52  /* 擦除32K块命令 */
#define CMD_SE3 0xD8  /* 擦除64K块命令 */
#define CMD_BE 0xC7   /* 批量擦除命令 */
#define WIP_FLAG 0x01 /* 状态寄存器中的正在编程标志（WIP) */

#define SF_MAX_PAGE_SIZE (4 * 1024)

/* 定义串行Flash ID */
enum
{
    SST25VF016B_ID = 0xBF2541,
    SST25VF064C_ID = 0xBF2643,
    MX25L1606E_ID = 0xC22015,
    W25Q64BV_ID = 0xEF4017,
    W25Q128JV_ID = 0xEF4018,
    PY25Q128HA_ID = 0x852018,

};

typedef struct
{
    u32 ChipID;        /* 芯片ID */
    char ChipName[16]; /* 芯片型号字符串，主要用于显示 */
    u32 TotalSize;     /* 总容量 */
    u16 SecSize;       /* 扇区大小 */
} SFLASH_T;

SFLASH_T g_tSF;

static void sf_WriteEnable(void)
{
    sf_CsOn();          /* 使能片选 */
    sf_Write(CMD_WREN); /* 发送命令 */
    sf_CsOff();         /* 禁能片选 */
}
static void sf_WriteDisable(void)
{
    sf_CsOn();          /* 使能片选 */
    sf_Write(CMD_WRDI); /* 发送命令 */
    sf_CsOff();         /* 禁能片选 */
}
static void sf_WaitForWriteEnd(void)
{
    sf_CsOn();          /* 使能片选 */
    sf_Write(CMD_RDSR); /* 发送命令， 读状态寄存器 */
    while ((sf_Read() & WIP_FLAG) == SET)
        sf_Delay; /* 判断状态寄存器的忙标志位 */
    sf_CsOff();   /* 禁能片选 */
}
static void sf_WriteStatus(u8 v)
{
    if (g_tSF.ChipID == SST25VF016B_ID || g_tSF.ChipID == SST25VF064C_ID)
    {
        /* 第1步：先使能写状态寄存器 */
        sf_CsOn();          /* 使能片选 */
        sf_Write(CMD_EWSR); /* 发送命令， 允许写状态寄存器 */
        sf_CsOff();         /* 禁能片选 */

        /* 第2步：再写状态寄存器 */
        sf_CsOn();          /* 使能片选 */
        sf_Write(CMD_WRSR); /* 发送命令， 写状态寄存器 */
        sf_Write(v);        /* 发送数据：状态寄存器的值 */
        sf_CsOff();         /* 禁能片选 */
    }
    else
    {
        sf_CsOn();          /* 使能片选 */
        sf_Write(CMD_WRSR); /* 发送命令， 写状态寄存器 */
        sf_Write(v);        /* 发送数据：状态寄存器的值 */
        sf_CsOff();         /* 禁能片选 */
    }
}
static u32 sf_ReadID(void)
{
    U32 temp = 0;
    sf_CsOn();
    // 发送4字节指令
    sf_Write(CMD_RDID);
    for (int i = 0; i < 3; i++)
    {
        temp <<= 8;
        temp += sf_Read();
    }
    sf_CsOff();
    return temp;
}
static void sf_info(void)
{
    char name[16];
    g_tSF.ChipID = sf_ReadID(); /* 芯片ID */
    switch (g_tSF.ChipID)
    {
    case SST25VF016B_ID:
        strcpy(name, "SST25VF016B");
        g_tSF.TotalSize = 2 * 1024 * 1024; /* 总容量 = 2M */
        g_tSF.SecSize = 4 * 1024;          /* 页面大小 = 4K */
        break;
    case SST25VF064C_ID:
        strcpy(name, "SST25VF064C_ID");
        g_tSF.TotalSize = 8 * 1024 * 1024; /* 总容量 = 8M */
        g_tSF.SecSize = 4 * 1024;          /* 页面大小 = 4K */
        break;
    case MX25L1606E_ID:
        strcpy(name, "MX25L1606E");
        g_tSF.TotalSize = 2 * 1024 * 1024; /* 总容量 = 2M */
        g_tSF.SecSize = 4 * 1024;          /* 页面大小 = 4K */
        break;
    case W25Q64BV_ID:
        strcpy(name, "W25Q64BV");
        g_tSF.TotalSize = 8 * 1024 * 1024; /* 总容量 = 8M */
        g_tSF.SecSize = 4 * 1024;          /* 页面大小 = 4K */
        break;
    case W25Q128JV_ID:
        strcpy(name, "W25Q128JV");
        g_tSF.SecSize = 4 * 1024;                   /* 1个扇区4K */
        g_tSF.TotalSize = 256 * 16 * g_tSF.SecSize; /* 总容量16M = 256块 * 16扇区 * 4k*/
        break;
    case PY25Q128HA_ID:
        strcpy(name, "PY25Q128HA");
        g_tSF.TotalSize = 16 * 1024 * 1024; /* 总容量 = 16M */
        g_tSF.SecSize = 4 * 1024;           /* 页面大小 = 4K */
        break;
    default:
        strcpy(name, "Unknow Flash");
        g_tSF.TotalSize = 2 * 1024 * 1024;
        g_tSF.SecSize = 4 * 1024;
        break;
    }
    // printf("============SPI FLASH INFO============\r\n");
    printf("FlashID: %2X\r\n", g_tSF.ChipID);
    // printf("ChipName: %s\r\n", name);
}
/**
 * @brief 判断写PAGE前是否需要先擦除。
 * @param bufOld    旧数据
 * @param buf       新数据
 * @param len       数据个数
 * @return u8
 * 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命
 * 算法第1步：old 求反, new 不变
          old    new
          1101   0101
    ~     1111
        = 0010   0101
    算法第2步: old 求反的结果与 new 位与
          0010   old
    &     0101   new
         =0000
    算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
 */
static u8 sf_NeedErase(u8 *bufOld, u8 *buf, u16 len)
{
    u16 i;
    u8 ucOld;
    for (i = 0; i < len; i++)
    {
        ucOld = *bufOld++;
        ucOld = ~ucOld;
        if ((ucOld & (*buf++)) != 0)
            return 1;
    }
    return 0;
}
static u16 sf_WritePage0(u32 addr, uint8_t *buf)
{
    u16 len = 0;
    sf_WriteEnable();                  /* 发送写使能命令 */
    sf_CsOn();                         /* 使能片选 */
    sf_Write(CMD_AAI);                 /* 发送AAI命令(地址自动增加编程) */
    sf_Write((addr & 0xFF0000) >> 16); /* 发送扇区地址的高8bit */
    sf_Write((addr & 0xFF00) >> 8);    /* 发送扇区地址中间8bit */
    sf_Write(addr & 0xFF);             /* 发送扇区地址低8bit */
    sf_Write(*buf++);                  /* 发送第1个数据 */
    sf_Write(*buf++);                  /* 发送第2个数据 */
    sf_CsOff();
    sf_WaitForWriteEnd(); /* 等待串行Flash内部写操作完成 */
    len += 2;

    while (len < g_tSF.SecSize)
    {
        sf_CsOn();         /* 使能片选 */
        sf_Write(CMD_AAI); /* 发送AAI命令(地址自动增加编程) */
        sf_Write(*buf++);
        sf_Write(*buf++); /* 发送第2个数据 */
        sf_CsOff();
        sf_WaitForWriteEnd(); /* 等待串行Flash内部写操作完成 */
        len += 2;
    }
    sf_WriteDisable();
    return len;
}
/**
 * @brief 初始化 SpiFlash
 */
void sf_Config(void)
{
    sf_WriteDisable();    /* 发送禁止写入的命令,即使能软件写保护 */
    sf_WaitForWriteEnd(); /* 等待串行Flash内部操作完成 */
    sf_info();            /* 打印芯片信息 */
    sf_WaitForWriteEnd(); /* 等待串行Flash内部操作完成 */
    sf_WriteStatus(0);    /* 解除所有BLOCK的写保护 */
    sf_WaitForWriteEnd(); /* 等待串行Flash内部操作完成 */
}

u16 sf_WritePage(u32 addr, u8 *buf, u16 len)
{
    u32 addr0;          /* 扇区首址 */
    u8 ucNeedErase = 1; /* 1表示需要擦除 */
    /* 长度为0时不继续操作,直接认为成功 */
    if ((len == 0) || (addr + len) > g_tSF.TotalSize)
        return 0;
    // /* 如果数据长度大于扇区容量，则退出，不会出现这种情况  */
    // if (len > g_tSF.SecSize)
    //     return 0;
    u8 *s_spiBuf = (U8 *)mymalloc(4 * 1024 * sizeof(u8));
    addr0 = addr & (~(g_tSF.SecSize - 1));
    /* 原扇区数据 */
    sf_ReadBytes(addr0, s_spiBuf, g_tSF.SecSize);
    /* 如果FLASH中的数据没有变化,则不写FLASH */
    if (memcmp(&s_spiBuf[addr - addr0], buf, len) == 0)
        goto exit;

    /* 判断是否需要先擦除扇区 */
    /* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
    ucNeedErase = sf_NeedErase(&s_spiBuf[addr - addr0], buf, len);
    memcpy(&s_spiBuf[addr - addr0], buf, len);
    if (ucNeedErase == 1)
        sf_EraseSector(addr0); /* 擦除扇区数据 */
    sf_WritePage0(addr0, s_spiBuf);
exit:
    myfree(s_spiBuf);
    return len;
}
void sf_EraseSector(u32 addr)
{
    addr = addr & (~(g_tSF.SecSize - 1));
    sf_WriteEnable(); /* 发送写使能命令 */
    /* 擦除扇区操作 */
    sf_CsOn();                         /* 使能片选 */
    sf_Write(CMD_SE);                  /* 发送擦除命令 */
    sf_Write((addr & 0xFF0000) >> 16); /* 发送扇区地址的高8bit */
    sf_Write((addr & 0xFF00) >> 8);    /* 发送扇区地址中间8bit */
    sf_Write(addr & 0xFF);             /* 发送扇区地址低8bit */
    sf_CsOff();                        /* 禁能片选 */
    sf_WaitForWriteEnd();              /* 等待串行Flash内部写操作完成 */
}

void sf_EraseChip(void)
{
    sf_WriteEnable(); /* 发送写使能命令 */
    /* 擦除扇区操作 */
    sf_CsOn();            /* 使能片选 */
    sf_Write(CMD_BE);     /* 发送整片擦除命令 */
    sf_CsOff();           /* 禁能片选 */
    sf_WaitForWriteEnd(); /* 等待串行Flash内部写操作完成 */
}

/**
 * @brief 向SPIFLASH写入数据
 * @param addr 地址
 * @param buf  数据缓存
 * @param len  数据长度
 * @return u8 写入的数据长度
 */
u16 sf_WriteBytes(u32 addr, u8 *buf, u16 len)
{
    // printf("Flash write: addr-%04x, length-%d", addr, len);
    /* 长度为0时不继续操作,直接认为成功 */
    if ((len == 0) || (addr + len) > g_tSF.TotalSize)
        return 0;
    u16 n = 0, count = 0;
    while (n < len)
    {
        count = g_tSF.SecSize - addr % g_tSF.SecSize;
        if (sf_WritePage(addr, buf, count) == 0)
            break;
        n += count;
        addr += count;
    }
    return n; /* 成功 */
}

/**
 * @brief 从SPIFLASH读出len长度数据
 * @param addr 起始地址
 * @param buf  数据缓存
 * @param len  数据长度
 * @return u16 实际读出的长度
 */
u16 sf_ReadBytes(u32 addr, u8 *buf, u16 len)
{
    /* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
    if ((len == 0) || (addr + len) > g_tSF.TotalSize)
        return 0;

    sf_CsOn();                  /* 使能片选 */
    sf_Write(CMD_READ);         /* 发送读命令 */
    sf_Write((u8)(addr >> 16)); /* 发送扇区地址的高8bit */
    sf_Write((u8)(addr >> 8));  /* 发送扇区地址中间8bit */
    sf_Write((u8)(addr));       /* 发送扇区地址低8bit */
    u16 i = len;
    while (i--)
    {
        *buf++ = sf_Read(); /* 读一个字节并存储到pBuf，读完后指针自加1 */
    }
    sf_CsOff(); /* 禁能片选 */
    return len;
}
