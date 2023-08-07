#include "kuconvert.h"

char fromHex(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return c;
}
char toHex(char c)
{
    if (c >= 0 && c <= 9)
        return c + '0';
    if (c >= 10 && c <= 15)
        return c - 10 + 'A';
    return c;
}

int fromHexes(char *c, int count)
{
    int result = 0;
    while (count--)
    {
        result *= 10;
        result += fromHex(*(c++));
    }
    return result;
}

char bcdToDec(char p) { return ((p >> 4) * 10) + (p & 0xF); }
char decToBcd(char p) { return ((p / 10) << 4) + (p % 10); }
