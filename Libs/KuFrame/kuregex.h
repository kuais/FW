#ifndef __KU_REGEX__
#define __KU_REGEX__

#include <stdlib.h>
#include <string.h>
#include "ku.h"

extern int regex_match(const char *pattern, const char *text);
extern char *regex_catch(const char *pattern, const char *text);

#endif
