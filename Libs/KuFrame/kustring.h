/**
 * @brief 字符串功能扩展
 * @author kuais (dlm321@126.com)
 * @version 1.0
 * @date 2022-07-25 11:03:38
 */
#ifndef __KU_STRING__
#define __KU_STRING__

#include <string.h>

/*  string.h 扩展  */
extern char *strupr(char *s);
extern char *strdup(char *s);
extern char *strrev(char *s);
extern char **split(char *in, const char *sep, int *count);

extern void timeToString(char *result, char year, char month, char date, char hour, char minute, char second);

extern char UTF8toUNC(char *src, char *dst);
extern char UNCtoUTF8(char *src, char *dst);
extern int string_UTF8toUNC(char *src, char *dst);
extern int string_UNCtoUTF8(char *src, char *dst);
#endif
