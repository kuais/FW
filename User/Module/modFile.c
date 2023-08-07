#include "modFile.h"
#include "modTime.h"
#include "main.h"

#define FS_ROOT "0:"

static FATFS fs1;

bool file_start(void) { return file_mount(&fs1, FS_ROOT); }

bool file_init(void)
{
    if (!file_mkfs(FS_ROOT))
        return false;
    file_new(FN_BOXFLAG);
    file_new(FN_SHIPMENT);
    file_new(FN_EVENT);
    file_new(FN_PRERESER);
    file_new(FN_INCIDENT);
#ifdef FS_DEBUG
    file_test();
#endif
    return true;
}

DWORD file_Len(const char *fn)
{
    FIL fp;
    DWORD iret = 0;
    if (!file_openRead(&fp, fn))
        goto exit;
    iret = file_length(&fp);
    file_close(&fp);
exit:
    return iret;
}

void file_getFree(void)
{
    DWORD fre_clust, fre_size, tot_size;
    FATFS *pfs = &fs1;

    uint8_t iret = f_getfree(FS_ROOT, &fre_clust, &pfs);
    if (iret == FR_OK)
    {
        // 总容量计算方法
        // pfs->csize 该参数代表一个簇占用几个 SD卡物理扇区，每个扇区512字节
        // pfs->n_fatent 簇的数量+2
        // 总容量 = 总簇数*一个簇占用大小
        // 剩余容量 = 剩余簇数*一个簇占用大小
        tot_size = (fs1.n_fatent - 2) * fs1.csize; // 总扇区
        fre_size = fre_clust * fs1.csize;          // 空扇区
        printf("file_getFreeSpace: Free|Total - %lu|%lu sectors \r\n", fre_size, tot_size);
    }
    else
        printf("file_getFreeSpace failed: %d \r\n", iret);
}

void file_test(void)
{
    //    int fd, ret = -1;
    //    FIL file;
    //    fd = file_openAppend(&file, FN_SHIPMENT);
    //    if (!fd)
    //        fd = file_openNew(&file, FN_SHIPMENT); //文件不存在则创建
    //    file_close(&file);
}
