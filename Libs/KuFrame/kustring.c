#include "kustring.h"
#include "ku.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 字符串小写转大写
 *
 * @param s
 * @return char*
 */
char *strupr(char *s)
{
    char *p = s;
    while (1)
    {
        char c = *p;
        if (!c)
            break;
        if (c >= 'a' && c <= 'z')
            *p = c - 0x20;
        p++;
    }
    return s;
}
/**
 * @brief 字符串拷贝，需调用free释放内存
 * @param s         源字符串
 * @return char*    新字符串
 */
char *strdup(char *s)
{
    char *result = (char *)kumalloc(strlen(s) + 1);
    if (result == 0)
        return (char *)0;
    strcpy(result, s);
    return result;
}
/**
 * @brief 字符串翻转
 * @param s         源字符串
 * @return char*    新字符串
 */
char *strrev(char *s)
{
    int len = strlen(s);
    char *res = (char *)kumalloc(len);
    for (int i = 0; i < len; i++)
        res[len - 1 - i] = s[i];
    return res;
}
/**
 * @brief 字符串分割
 * 用sep中的每一个分隔符分割in字符串,然后返回一个字符串数组.
 * 这个函数有以下特性:
 * @param in    输入的字符串,
 * @param sep   分隔符
 * @param count 分割后的字符串个数.
 * @return char **  分割后的字符串数组
 */
char **split(char *input, const char *sep, int *count)
{
    char *in = input;
    int lenSep = strlen(sep);
    int len = strlen(in);
    int n = 0;

    char **out = (char **)kumalloc((len / 2) * sizeof(char **)); // 最多是原长度的一半
    *out = in;
    while (1)
    {
        in = strstr(in, sep);
        if (!in)
            break; // 最后1个字符串
        *in = '\0';
        in += lenSep;
        n++; // 分隔符个数
        *(out + n) = in;
    }
    *count = n + 1; // 总的字符串数为分隔符个数加1
    return out;
}

/**
 * @brief 将年月日时分秒输出成yyyy-MM-dd hh:mm:ss格式的字符串
 * @param result 格式化后的结果，格式为"2000-01-01 01:01:01"格式的字符串
 * @param year   年
 * @param month  月
 * @param date   日
 * @param hour   时
 * @param minute 分
 * @param second 秒
 */
void timeToString(char *result, char year, char month, char date, char hour, char minute, char second)
{
    result[2] = (year >> 4) + 0x30;
    result[3] = (year & 0xF) + 0x30;
    result[5] = (month >> 4) + 0x30;
    result[6] = (month & 0xF) + 0x30;
    result[8] = (date >> 4) + 0x30;
    result[9] = (date & 0xF) + 0x30;
    result[11] = (hour >> 4) + 0x30;
    result[12] = (hour & 0xF) + 0x30;
    result[14] = (minute >> 4) + 0x30;
    result[15] = (minute & 0xF) + 0x30;
    result[17] = (second >> 4) + 0x30;
    result[18] = (second & 0xF) + 0x30;
}

/**
 * @brief UTF8字符 转UNICODE 字符
 * @param src   输入UTF8字符数据
 * @param dst  输出UNICODE字符数据 2字节
 * @return char UTF8字符长度
 */
char UTF8toUNC(char *src, char *dst)
{
    uint8_t codeLen = 0;
    uint16_t unicode = 0;
    int i;
    if (0 == (src[0] & 0x80))
    { // 0 - 7F : 0xxxxxxx 7
        codeLen = 1;
        unicode = src[0];
    }
    else if (0 == (src[0] & 0x20))
    { // 80 - 7FF : 110xxxxx 10xxxxxx 11
        codeLen = 2;
        unicode = (src[0] & 0x1F);
    }
    else if (0 == (src[0] & 0x10))
    { // 800 - FFFF : 1110xxxx 10xxxxxx 10xxxxxx 16
        codeLen = 3;
        unicode = (src[0] & 0xF);
    }
    else if (0 == (src[0] & 0x8))
    { // 10000 - 1FFFFF : 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 21
        codeLen = 4;
        unicode = (src[0] & 0x7);
    }
    else if (0 == (src[0] & 0x4))
    { // 200000 - 3FFFFFF : 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 26
        codeLen = 5;
        unicode = (src[0] & 0x3);
    }
    else if (0 == (src[0] & 0x2))
    { // 4000000 - 7FFFFFFF : 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 31
        codeLen = 6;
        unicode = (src[0] & 0x1);
    }
    for (i = 1; i < codeLen; i++)
    {
        unicode <<= 6;
        unicode += (src[i] & 0x3F);
    }
    dst[0] = (char)(unicode >> 8);
    dst[1] = (char)unicode;
    return codeLen;
}

/**
 * @brief UNICODE 字符转 UTF8字符
 * @param src   输入UNICODE字符数据 2字节
 * @param dst  输出UTF8字符数据
 * @return char UTF8字符长度
 */
char UNCtoUTF8(char *src, char *dst)
{
    uint8_t codeLen = 0;
    uint16_t unicode;
    int i;
    unicode = (src[0] << 8) + src[1];
    dst[0] = 0;
    if (unicode < 0x80)
    { // 0 - 7F : 0xxxxxxx 7
        codeLen = 1;
        dst[0] = 0;
    }
    else if (unicode < 0x800)
    { // 80 - 7FF : 110xxxxx 10xxxxxx 11
        codeLen = 2;
        dst[0] = 0xC0;
    }
    else if (unicode < (uint16_t)0x10000)
    { // 800 - FFFF : 1110xxxx 10xxxxxx 10xxxxxx 16
        codeLen = 3;
        dst[0] = 0xE0;
    }
    else if (unicode < (uint16_t)0x20000)
    { // 10000 - 1FFFFF : 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 21
        codeLen = 4;
        dst[0] = 0xF0;
    }
    else if (unicode < (uint16_t)0x4000000)
    { // 200000 - 3FFFFFF : 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 26
        codeLen = 5;
        dst[0] = 0xF8;
    }
    else if (unicode < (uint16_t)0x80000000)
    { // 4000000 - 7FFFFFFF : 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 31
        codeLen = 6;
        dst[0] = 0xFC;
    }
    for (i = 1; i < codeLen; i++)
    {
        dst[codeLen - i] = 0x80 + (unicode & 0x3F);
        unicode >>= 6;
    }
    dst[0] += unicode;
    return codeLen;
}

/**
 * @brief 字符串数据从UTF8转成UNICODE
 *
 * @param src  源字符串
 * @param dst  目标字符串
 * @return char 转换后的数据长度
 */
int string_UTF8toUNC(char *src, char *dst)
{
    int count = 0;
    int n;
    while (src[0])
    {
        n = UTF8toUNC(src, dst);
        src += n;
        dst += 2;
        count += 2;
    }
    *dst = 0;
    return count;
}
int string_UNCtoUTF8(char *src, char *dst)
{
    int count = 0;
    int n;
    while (src[0])
    {
        n = UNCtoUTF8(src, dst);
        src += 2;
        dst += n;
        count += n;
    }
    *dst = 0;
    return count;
}
