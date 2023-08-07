/**
 * @file test.c
 * @author LeoDu (dlm321@126.com)
 * @brief 测试
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "main.h"
#include "KuFrame/kustring.h"
#include "kuFrame/kulist.h"

/**
 * @brief 测试正则
 *
 */
void testRegex(void)
{
    //    char *pattern = "^(6A|6C|6G|6P|6Q|6V|6X|7D|7E|7F|7G|7H|7I|7J|7K|7P|7T|7V|7W|7X|7Z|8A|8G|8H|8K|8L|8P|8V|8W|9A|6W|9B|9C|9D|6M|9L|9P|9V|KPS)[0-9]{11}$";
    //    char *text = "6C00000000000";
    //    int length = strlen(text);

    //    int m = re_match(pattern, text, &length);

    printf("%s\n", "");
}
void testSplit()
{
    char *text = ",1,0,0,1,,1,0,0,2,";
    int count = 0;
    char *in = strdup(text);
    char **out = split(in, ",", &count);
    for (int i = 0; i < count; i++)
    {
        printf("%s\r\n", out[i]);
    }
    myfree(in);
    myfree(out);
}
void test_RandomDigit()
{
    size_t len = 0;
    unsigned char s[5] = {0};
    unsigned char s2[5] = {0};
    int count = 150;
    while (count--)
    {
        os_dly_wait(1000);
        randomDigits(s, 4, &len);
        os_dly_wait(1);
        randomDigits(s2, 4, &len);
        printf("Random digit:%s%s\r\n", s, s2);
    }
}
//void testList()
//{
//    char *arr[] = {"a11", "a12", "a13", "a14"};
//    KuList *listDev = list_new();
//    list_add(listDev, arr[0]);
//    list_add(listDev, arr[1]);
//    list_add(listDev, arr[2]);
//    list_add(listDev, arr[3]);
//    printf("List count: %d", list_Count(listDev));
//    listDev = list_removeAt(listDev, 2);
//    printf("List count: %d, Current: %s", list_Count(listDev), listDev->data);
//    listDev = list_remove(listDev);
//    printf("List count: %d, Current: %s", list_Count(listDev), listDev->data);
//}
void test(void)
{ /* 测试代码放这里 */
    // test_RandomDigit();
    // testSplit();
    // testRegex();
    /* test workflow */
    //    workFlow("5947357908");

    // testList();
}
