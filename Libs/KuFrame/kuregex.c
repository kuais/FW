/**
 * @file kuRegex.c
 * @author LeoDu (dlm321@126.com)
 * @brief 正则匹配
 * @version 0.1
 * @date 2023-01-04
 * 按字符比较，分别处理
 * @copyright Copyright (c) 2023
 *
 */

#include "kuregex.h"

typedef enum
{
    PT_UNUSED,
    PT_DOT,          // .
    PT_BEGIN,        // ^
    PT_END,          // $ or '\0'
    PT_QUESTIONMARK, // ?
    PT_STAR,         // *
    PT_PLUS,         // +
    PT_CHAR,
    PT_CHARCLASS,
    PT_INVCHARCLASS,
    PT_DIGIT,         // \d
    PT_NOTDIGIT,      // \D
    PT_ALPHA,         // \w
    PT_NOTALPHA,      // \W
    PT_WHITESPACE,    // \s
    PT_NOTWHITESPACE, // \S
    PT_BRANCH,        // |
    PT_RANGE,         // []
    PT_GROUP,         // ()
    PT_LOOP           // {}
} PatternType;

typedef struct
{
    char *p; // Pattern
    char *t; // Text3
} RegexObj;

static PatternType getPatternType(char *p)
{
    switch (*p)
    {
    case '^':
        return PT_BEGIN;
    case '$':
    case '\0':
        return PT_END;
    case '.':
        return PT_DOT;
    case '*':
        return PT_STAR;
    case '+':
        return PT_PLUS;
    case '?':
        return PT_QUESTIONMARK;
    case '|':
        return PT_BRANCH;
    case '[':
        return PT_RANGE;
    case '{':
        return PT_LOOP;
    case '(':
        return PT_GROUP;
    case '\\':
    {
        p++;
        switch (*p)
        {
        case 'd':
            return PT_DIGIT;
        case 'D':
            return PT_NOTDIGIT;
        case 'w':
            return PT_ALPHA;
        case 'W':
            return PT_NOTALPHA;
        case 's':
            return PT_WHITESPACE;
        case 'S':
            return PT_NOTWHITESPACE;
        default: /* Escaped character, e.g. '.' or '$' */
            return PT_CHAR;
        }
    }
    default:
        return PT_CHAR;
    }
}

static int getLoopTime(RegexObj *r)
{
    int ret = 1;
    char *pText = r->p + 1;
    int n = 0;
    PatternType type = getPatternType(pText);
    if (type == PT_LOOP)
    {
        pText++;
        while (1)
        {
            if (pText[n] == '}')
                break;
            if (pText[n] == 0)
                return 0; // 无效表达式
            n++;
        }
        if (n > 0)
        {
            char p2[n + 1];
            p2[n] = 0;
            memcpy(p2, pText, n); // 得到表达式
            ret = atoi(p2);
        }
        r->p += n + 2;
    }
    return ret;
}

static int matchChar(RegexObj *r)
{
    char c = r->p[0];
    int n = getLoopTime(r);
    if (n == 0)
        return 0; // 循环次数无效
    while (n-- > 0)
    {
        if (c != r->t[0])
            return 0;
        r->t++;
    }
    return 1;
}
static char *getRange(RegexObj *r)
{
    int n = 0;
    char *pText = r->p;
    pText++;
    while (1)
    {
        if (pText[n] == ']')
            break;
        if (pText[n] == '\0')
            return 0; // 无效表达式
        n++;
    }
    if (n == 0)
        return 0; // 空表达式
    char *text = kucalloc(n + 1, 1);
    memcpy(text, pText, n); // 得到表达式
    r->p += n + 1;
    return text;
}
static int matchRange(RegexObj *r)
{
    char *s = getRange(r);
    if (s == NULL)
        return 0; // 子表达式无效
    int n = getLoopTime(r);
    if (n == 0)
        goto error1; // 循环次数无效
    char c = 0;
    int i = 0;
    while (n-- > 0)
    {
        i = 0;
        while (1)
        {
            if (s[i] == 0)
                goto error1; // 匹配失败
            if (s[i] == '-')
            {
                i++;
                while (c <= s[i])
                {
                    c++;
                    if (c == r->t[0])
                        goto exit1; // 匹配成功
                }
            }
            else
            {
                c = s[i];
                if (c == r->t[0])
                    goto exit1; // 匹配成功
            }
            i++;
        }
    exit1:
        r->t++;
    }
    kufree(s);
    return 1;
error1:
    kufree(s);
    return 0;
}
/**
 * @brief 获取子表达式
 * @param r        原正则对象
 * @param pObj     子正则对象
 * @return int     子表达式长度
 */
static int getGroup(RegexObj *r, RegexObj *pObj)
{
    int n = 0;
    char *pText = r->p;
    /* 获取子表达式 */
    pText++;
    while (1)
    {
        if (pText[n] == ')')
            break;
        if (pText[n] == '\0')
            return 0; // 无效表达式
        n++;
    }
    if (n == 0)
        return 0; // 空表达式
    pObj->p = kucalloc(n + 1, 1);
    memcpy(pObj->p, pText, n); // 得到表达式
    r->p += n + 1;
    return n;
}
static int matchGroup(RegexObj *r)
{
    int ret = 0;
    RegexObj obj;
    char *pBak = NULL;
    int len = getGroup(r, &obj);
    if (len == 0)
        goto error1; // 子表达式无效
    int n = getLoopTime(r);
    if (n == 0)
        goto error1; // 循环次数无效
    obj.t = r->t;
    pBak = obj.p;
    while (n-- > 0)
    {
        if (!match(&obj))
            goto error1;
        obj.p = pBak;
    }
    r->t = obj.t;
    ret = 1;
error1:
    if (pBak != NULL)
        kufree(pBak);
    return ret;
}
/**
 * @brief 正则匹配
 *
 * @param r     要进行匹配的对象
 * @return int  0: 失败， 1:成功
 */
static int match(RegexObj *r)
{
    int ret = 1;
    char *tBak = r->t;
    while (1)
    {
        PatternType type = getPatternType(r->p);
        switch (type)
        {
        case PT_BEGIN:
            break;
        case PT_END:
            return ret; // 匹配结束
        case PT_CHAR:
        {
            ret = matchChar(r);
            break;
        }
        case PT_RANGE:
        {
            ret = matchRange(r);
            break;
        }
        case PT_GROUP:
        {
            ret = matchGroup(r);
            break;
        }
        default: /* 其他未实现 */
            break;
        }
        r->p++;
        if (getPatternType(r->p) == PT_BRANCH)
        {
            if (ret)        // 分支匹配成功
                return ret; // 匹配结束
            else            // 分支匹配失败，继续匹配下一分支
            {
                r->t = tBak;
                r->p++;
                ret = 1;
            }
        }
    }
exit:
    return ret;
}

/**
 * @brief 正则匹配
 * @param pattern   正则表达式
 * @param text      要匹配的字符串
 * @param length    要匹配的字符长度
 * @return int      0:匹配失败 1:匹配成功
 */
int regex_match(const char *pattern, const char *text)
{
    RegexObj obj;
    obj.p = pattern;
    obj.t = text;
    int ret = match(&obj);
    if (ret)
    {
        if (obj.t[0] != 0)
            ret = 0;
    }
    return ret;
}

/**
 * @brief 从字符串里找出符合规则的子字符串
 * @return char*     0:未找到, >0:子字符串的起始位置
 */
char *regex_catch(const char *pattern, const char *text)
{
    char *ret = 0;
    RegexObj obj;
    obj.p = pattern;
    obj.t = text;
    return ret;
}
