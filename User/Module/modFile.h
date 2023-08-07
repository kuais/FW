#ifndef __MODULE_FILE__
#define __MODULE_FILE__

#include "FatFS/fs.h"

extern bool file_start(void);
extern bool file_init(void);
extern DWORD file_Len(const char *fn);
extern void file_test(void);
extern void file_getFree(void);
#endif
