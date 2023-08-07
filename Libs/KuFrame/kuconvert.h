#ifndef __KU_CONVERT__
#define __KU_CONVERT__

extern char fromHex(char c);
extern int fromHexes(char *c, int count);
extern char toHex(char c);
extern char bcdToDec(char p);
extern char decToBcd(char p);

#endif
