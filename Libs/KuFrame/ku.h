/**
 * @brief 环境适配选项在这里设置
 * @author kuais (dlm321@126.com)
 * @version 1.0
 * @date 2021-06-07 10:13:01
 */
#ifndef __KU__
#define __KU__

#include <stdio.h>

//#define KUDEBUG

#ifdef KUDEBUG
#define kuprintf printf
#else
#define kuprintf(...)
#endif

#define kumalloc mymalloc
#define kucalloc mycalloc
#define kurealloc myrealloc
#define kufree myfree

#endif
