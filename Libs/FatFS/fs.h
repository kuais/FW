#ifndef __FS_
#define __FS_

#include <stdio.h>
#include <stdbool.h>
#include <RTL.h>
#include "ff.h"

#define FS_DEBUG

extern bool file_mount(FATFS *fs, const char *path);
extern bool file_unmount(const char *path);
extern bool file_mkfs(const char *path);
extern bool file_opendir(DIR *dir, const char *path);
extern bool file_open(FIL *fp, const char *fn, BYTE mode);
extern bool file_close(FIL *fp);
extern bool file_openRead(FIL *fp, const char *fn);
extern bool file_openWrite(FIL *fp, const char *fn);
extern bool file_openReadWrite(FIL *fp, const char *fn);
extern bool file_openAppend(FIL *fp, const char *fn);
extern bool file_openNew(FIL *fp, const char *fn);
extern bool file_move(FIL *fp, DWORD pos);
extern int file_read(FIL *fp, void *buf, size_t size);
extern int file_write(FIL *fp, const void *buf, size_t len);
extern DWORD file_length(FIL *fp);
extern bool file_new(const char *fn);
extern bool file_creat(const char *fn);
extern bool file_delete(const char *fn);
extern bool file_rename(const char *fn, const char *fnNew);
extern int file_find(const char *fn, const char *buf, size_t len);
extern DWORD file_truncate(const char *fn, DWORD pos);
extern DWORD file_remove(const char *fn, DWORD pos, size_t len);
extern int file_append(const char *fn, char *buf, size_t len);

#endif
