#include "fs.h"
#include "ff.h"

#ifdef FS_DEBUG
#define fs_printf printf
#else
#define fs_printf
#endif

/* FatFs API的返回值 */
// const char *FR_Table[] = {
//     "FR_OK: OK", /* (0) Succeeded */
// "FR_DISK_ERR: 底层硬件错误",                             /* (1) A hard error occurred in the low level disk I/O layer */
// "FR_INT_ERR: 断言失败",                                  /* (2) Assertion failed */
// "FR_NOT_READY: 物理驱动没有工作",                        /* (3) The physical drive cannot work */
// "FR_NO_FILE: 文件不存在",                                /* (4) Could not find the file */
// "FR_NO_PATH: 路径不存在",                                /* (5) Could not find the path */
// "FR_INVALID_NAME: 无效文件名",                           /* (6) The path name format is invalid */
// "FR_DENIED: 由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
// "FR_EXIST: 文件已经存在",                                /* (8) Access denied due to prohibited access */
// "FR_INVALID_OBJECT: 文件或者目录对象无效",               /* (9) The file/directory object is invalid */
// "FR_WRITE_PROTECTED: 物理驱动被写保护",                  /* (10) The physical drive is write protected */
// "FR_INVALID_DRIVE: 逻辑驱动号无效",                      /* (11) The logical drive number is invalid */
// "FR_NOT_ENABLED: 卷中无工作区",                          /* (12) The volume has no work area */
// "FR_NO_FILESYSTEM: 没有有效的FAT卷",                     /* (13) There is no valid FAT volume */
// "FR_MKFS_ABORTED: 由于参数错误f_mkfs()被终止",           /* (14) The f_mkfs() aborted due to any parameter error */
// "FR_TIMEOUT: 在规定的时间内无法获得访问卷的许可",        /* (15) Could not get a grant to access the volume within defined period */
// "FR_LOCKED: 由于文件共享策略操作被拒绝",                 /* (16) The operation is rejected according to the file sharing policy */
// "FR_NOT_ENOUGH_CORE: 无法分配长文件名工作区",            /* (17) LFN working buffer could not be allocated */
// "FR_TOO_MANY_OPEN_FILES: 当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
// "FR_INVALID_PARAMETER: 参数无效"                         /* (19) Given parameter is invalid */
// };

bool file_rename(const char *fn, const char *fnNew)
{
    FRESULT iret = f_rename(fn, fnNew);
    if (iret != FR_OK)
        fs_printf("file_rename %s failed: %d \r\n", fn, iret);
    return iret == FR_OK;
}
bool file_mount(FATFS *fs, const char *path)
{
    FRESULT iret = f_mount(fs, path, 0);
    if (iret != FR_OK)
        fs_printf("file_mount [%s] failed: %d \r\n", path, iret);
    return iret == FR_OK;
}
bool file_unmount(const char *path)
{
    FRESULT iret = f_mount(NULL, path, 0);
    if (iret != FR_OK)
        fs_printf("file_unmount [%s] failed: %d \r\n", path, iret);
    return iret == FR_OK;
}
bool file_mkfs(const char *path)
{
    FRESULT iret = f_mkfs(path, 0, 0);
    if (iret != FR_OK)
        fs_printf("file_mkfs [%s] faild: %d \r\n", path, iret);
    return iret == FR_OK;
}
bool file_opendir(DIR *dir, const char *path)
{
    FRESULT iret = f_opendir(dir, path); // 打开文件夹, 如果不带参数，则从当前目录开始
    if (iret != FR_OK)
        fs_printf("file_opendir %s failed: %d \r\n", path, iret);
    return iret == FR_OK;
}
bool file_open(FIL *fp, const char *fn, BYTE mode)
{
    FRESULT iret = f_open(fp, fn, mode); // 打开文件
    if (iret != FR_OK && iret != FR_NO_FILE)
        fs_printf("file_open %s failed: %d \r\n", fn, iret);
    return iret == FR_OK;
}
bool file_close(FIL *fp)
{
    FRESULT iret = f_close(fp); // 打开文件
    if (iret != FR_OK && iret != FR_NO_FILE)
        fs_printf("file_close %s failed: %d \r\n", fp->dir_ptr, iret);
    return iret == FR_OK;
}
/**
 * @brief			打开文件用于读取
 * @param fn		目标文件名
 * @return 			true | false
 */
bool file_openRead(FIL *fp, const char *fn) { return file_open(fp, fn, FA_OPEN_EXISTING | FA_READ); }
/**
 * @brief			打开文件用于写入
 * @param fn		目标文件名
 * @return 			true | false
 */
bool file_openWrite(FIL *fp, const char *fn) { return file_open(fp, fn, FA_OPEN_EXISTING | FA_WRITE); }
bool file_openReadWrite(FIL *fp, const char *fn) { return file_open(fp, fn, FA_OPEN_EXISTING | FA_WRITE | FA_READ); }
bool file_openAppend(FIL *fp, const char *fn)
{
    bool ret = file_openWrite(fp, fn);
    if (ret)
        ret = file_move(fp, file_length(fp));
    return ret;
}
bool file_openNew(FIL *fp, const char *fn)
{ // 打开文件以写入，文件不存在则创建，文件存在则删除其中内容
    return file_open(fp, fn, FA_CREATE_ALWAYS | FA_WRITE);
}
bool file_new(const char *fn)
{
    FIL file;
    bool ret;
    ret = file_openNew(&file, fn);
    if (ret)
        ret = file_close(&file);
    return ret;
}
bool file_creat(const char *fn)
{
    FIL file;
    bool ret = file_openRead(&file, fn);
    if (!ret)
    {
        return file_new(fn);
    }
    file_close(&file);
    return true;
}
bool file_move(FIL *fp, DWORD pos) { return f_lseek(fp, pos) == FR_OK; }
bool file_delete(const char *fn)
{
    FRESULT iret = f_unlink(fn);
    if (iret != FR_OK)
        fs_printf("file_delete %s failed: %s \r\n", fn, iret);
    return iret == FR_OK;
} // >0 删除成功， =0 删除失败

int file_read(FIL *fp, void *buf, size_t size)
{
    int ret;
    FRESULT iret = f_read(fp, buf, size, &ret);
    if (iret != FR_OK)
    {
        fs_printf("file_read %s failed: %d \r\n", fp->dir_ptr, iret);
        ret = 0;
    }
    return ret; // >0 成功读取， =0 失败
}
int file_write(FIL *fp, const void *buf, size_t size)
{
    int ret;
    FRESULT iret = f_write(fp, buf, size, &ret);
    if (iret != FR_OK)
    {
        fs_printf("file_write %s failed: %d \r\n", fp->dir_ptr, iret);
        ret = 0;
    }
    else
    {
        iret = f_sync(fp);
        if (iret != FR_OK)
        {
            fs_printf("file_sync %s failed: %d \r\n", fp->dir_ptr, iret);
            ret = 0;
        }
    }
    return ret; // >0 成功写入， =0 失败
}

DWORD file_length(FIL *fp) { return f_size(fp); }

/**
 * @brief 查找指定字符串在文件里的位置
 * @param fn    要查找的文件名
 * @param buf   要查找的字符串
 * @param len   字符串长度
 * @return long  -1,未找到，其他，所在文件位置
 */
int file_find(const char *fn, const char *buf, size_t len)
{
    int fd;
    int ret = -1;
    FIL file;
    fd = file_openRead(&file, fn);
    if (!fd)
        return ret;
    char datas[len];
    DWORD pos = 0;
    while (1)
    {
        if (file_read(&file, &datas, len) <= 0)
            break;
        if (memcmp(datas, buf, len) == 0)
        {
            ret = pos;
            break;
        }
        pos += len;
    }
    file_close(&file);
    return ret;
}

/**
 * @brief   删除文件pos后面的内容
 * @param fn        要删除的文件
 * @param pos       删除的位置
 * @return DWORD    删除后文件大小，一般=pos
 */
DWORD file_truncate(const char *fn, DWORD pos)
{
    FRESULT ret;
    FIL fp;
    if (!file_openWrite(&fp, fn))
        return 0;
    DWORD len1 = file_length(&fp);
    if (len1 <= pos)
        return len1; // 删除的位置超出范围
    file_move(&fp, pos);
    ret = f_truncate(&fp);
    if (ret == FR_OK)
        pos = file_length(&fp);
    file_close(&fp);
    return pos;
}
int file_append(const char *fn, char *buf, size_t len)
{
    FIL fp;
    int ret = 0;
    int fd;
    fd = file_openAppend(&fp, fn);
    if (!fd)
        fd = file_openNew(&fp, fn);
    if (fd)
    {
        // file_move(&fp, pos);
        ret = file_write(&fp, buf, len);
        file_close(&fp);
    }
    return ret;
}
/**
 * @brief       删除文件中从指定位置开始的一段数据，后面的数据前移
 * @param fn    文件名
 * @param pos   起始位置
 * @param len   删除的字节数
 * @return int  删除后的文件长度
 */
DWORD file_remove(const char *fn, DWORD pos, size_t len)
{
    DWORD ret = 0;
    FIL fp;
    if (!file_openRead(&fp, fn))
        goto exit;
    DWORD len1 = file_length(&fp);
    if (len1 <= 0) // 文件内容为空
        goto exit2;
    if ((pos == 0) && (len >= len1))
    {
        file_close(&fp);
        file_new(fn);
        goto exit;
    }
    FIL fp2;
    int lenRead;
    char buf[64];
    if (!file_openWrite(&fp2, fn))
        goto exit2;
	file_move(&fp2, pos);
    if ((len + pos) > len1)
        len = len1 - pos; //
	pos += len;
    file_move(&fp, pos);
    while (pos < len1)
    {
        lenRead = file_read(&fp, buf, 64);
        if (lenRead == 0)
            break;
        file_write(&fp2, buf, lenRead);
		pos += lenRead;
    }
    f_truncate(&fp2);
    ret = file_length(&fp2);
    file_close(&fp2);
exit2:
    file_close(&fp);
exit:
    return ret;
}

// static FRESULT list(void)
// {
//     FRESULT iret;
//     { /* 读取当前文件夹下的文件和目录 */
//         char lfname[256];
//         uint32_t cnt = 0;
//         FILINFO fileInfo;
//         fileInfo.lfname = lfname;
//         fileInfo.lfsize = 256;
//         fs_printf("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
//         for (cnt = 0;; cnt++)
//         {
//             iret = f_readdir(&dirInfo, &fileInfo); /* 读取目录项，索引会自动下移 */
//             if (iret != FR_OK || fileInfo.fname[0] == 0)
//                 break;
//             if (fileInfo.fname[0] == '.')
//                 continue;
//             /* 判断是文件还是子目录 */
//             if (fileInfo.fattrib & AM_DIR)
//                 fs_printf("(0x%02d)目录  ", fileInfo.fattrib);
//             else
//                 fs_printf("(0x%02d)文件  ", fileInfo.fattrib);
//             /* 打印文件大小, 最大4G */
//             fs_printf(" %10ld", fileInfo.fsize);
//             fs_printf("  %s |", fileInfo.fname);            /* 短文件名 */
//             fs_printf("  %s\r\n", (char *)fileInfo.lfname); /* 长文件名 */
//         }
//     }
//     return iret;
// }
// /**
//  * @brief 			列出当前目录的所有文件
//  * @return 			true | false
//  */
// bool file_list(void)
// {
//     FRESULT ret_fs;
//     ret_fs = list();
//     if (ret_fs != FR_OK)
//         fs_printf("显示文件列表失败: %s \r\n", FR_Table[ret_fs]);
//     return ret_fs == FR_OK;
// }
