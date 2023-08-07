#include "kern_json.h"

char *Kern_GetToken(cJSON *jobj) { return cJSON_GetObjectItem(jobj, "token")->valuestring; }

bool Kern_ParseResult(cJSON *jobj, Kern_Result *result)
{
    result->data = cJSON_GetObjectItem(jobj, "data")->valuestring;
    result->error = cJSON_GetObjectItem(jobj, "error")->valuestring;
    result->pagesize = cJSON_GetObjectItem(jobj, "pagesize")->valueint;
    return strcmp(result->error, "") == 0;
}
void Kern_ParseSetting(cJSON *jobj, Kern_Setting *setting)
{
    setting->set_time = cJSON_GetObjectItem(jobj, "set_time")->valuestring;
    setting->seconds_time_limit_close_door = cJSON_GetObjectItem(jobj, "seconds_time_limit_close_door")->valueint;
}

char *trimEnter(char *c)
{
    int i = 0;
    while (i < strlen(c))
    {
        if (*(c + i) == 0x10)
        {
            *(c + i) = 0x0;
            return c;
        }
        i++;
    }
    return c;
}