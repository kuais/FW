#include "kuutil.h"

/**
 * @brief 计算累加和
 * @param input 输入数据，
 * @param offset 偏移位置
 * @param count 要计算的数字数
 * @return int 累加和
 */
int getSum(char *input, int offset, int count)
{
	int sum = 0;
	while (count-- > 0)
		sum += input[offset++];
	return sum;
}

/**
 * @brief 获取指定bit的值
 * @param input 数据
 * @param pos	bit的位置
 * @param count bit数
 * @return int 0|1
 */
int getBit(int input, int pos, int count)
{
	unsigned char temp = 1;
	while (--count > 0)
	{
		temp <<= 1;
		temp += 1;
	}
	return (input >> pos) & temp;
}

/**
 * @brief 设置指定bit的值
 * @param input	数据
 * @param pos	bit的位置
 * @param value	bit的值
 * @param count	bit数
 * @return int 设置后数据的值
 */
int setBit(int input, int pos, int value, int count)
{
	unsigned char temp = 1;
	while (--count > 0)
	{
		temp <<= 1;
		temp += 1;
	}
	temp <<= pos;
	value <<= pos;
	return (input & ~temp) | value;
}

/**
 * @brief 翻转数组顺序
 * @param arr 	数组
 * @param len 	要翻转的数据量
 */
void revertArr(char *arr, int len)
{
	int i = 0;
	char *p;
	len--;
	while (1)
	{
		if (i >= len - i)
			break;
		p = arr[i];
		arr[i] = arr[len - i];
		arr[len - i] = p;
		i++;
	}
}

char isDigit(char c)
{
	return (c >= '0') && (c <= '9');
}
char isAtoZ(char c)
{
	return (c >= 'A') && (c <= 'Z');
}
char isatoz(char c)
{
	return (c >= 'a') && (c <= 'z');
}