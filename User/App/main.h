#ifndef __MAIN__
#define __MAIN__

#include "bsp.h"
#include "gtypes.h"
#include "os.h"
#include "W5500/utility.h"
#include "module/modFile.h"
#include "module/modTime.h"

/* ===== 常量    ===== */
#define SOFTNAME "SPKERN_MB8"
#define VERSION "V0.4.9"
#define IVERSION 49

/* 短文件名 最多8个字符 */
#define FN_TEMP "TEMP"
#define FN_BOX "BOX"
#define FN_BOXFLAG "BOXFLAG"
#define FN_BOXPLU "BOXPLU"
#define FN_PLU "PLU"
#define FN_COURIER "COURIER"
#define FN_LABELRULE "LABELRUL"
#define FN_LABELPLU "LABELPLU"
#define FN_TECHNICIAN "TECHNICI" // Technician
#define FN_TECHACTION "TECHACT"  // TechnicianAction
#define FN_SHIPMENT "SHIPMENT"
#define FN_EVENT "EVENT"
#define FN_PRERESER "PRERESER"
#define FN_INCIDENT "INCIDENT"
#define FN_RESSHIPMENT "RESSPM"

#define CA_SWISS                                                       \
    "-----BEGIN CERTIFICATE-----\r\n"                                  \
    "MIIIujCCBqKgAwIBAgITMwAxS6DhmVCLBf6MWwAAADFLoDANBgkqhkiG9w0BAQwF" \
    "ADBZMQswCQYDVQQGEwJVUzEeMBwGA1UEChMVTWljcm9zb2Z0IENvcnBvcmF0aW9u" \
    "MSowKAYDVQQDEyFNaWNyb3NvZnQgQXp1cmUgVExTIElzc3VpbmcgQ0EgMDEwHhcN" \
    "MjIwMzE0MTgzOTU1WhcNMjMwMzA5MTgzOTU1WjBqMQswCQYDVQQGEwJVUzELMAkG" \
    "A1UECBMCV0ExEDAOBgNVBAcTB1JlZG1vbmQxHjAcBgNVBAoTFU1pY3Jvc29mdCBD" \
    "b3Jwb3JhdGlvbjEcMBoGA1UEAwwTKi5henVyZXdlYnNpdGVzLm5ldDCCASIwDQYJ" \
    "KoZIhvcNAQEBBQADggEPADCCAQoCggEBAM3heDMqn7v8cmh4A9vECuEfuiUnKBIw" \
    "7y0Sf499Z7WW92HDkIvV3eJ6jcyq41f2UJcG8ivCu30eMnYyyI+aRHIedkvOBA2i" \
    "PqG78e99qGTuKCj9lrJGVfeTBJ1VIlPvfuHFv/3JaKIBpRtuqxCdlgsGAJQmvHEn" \
    "vIHUV2jgj4iWNBDoC83ShtWg6qV2ol7yiaClB20Af5byo36jVdMN6vS+/othn3jG" \
    "pn+NP00DWYbP5y4qhs5XLH9wQZaTUPKIaUxmHewErcM0rMAaWl8wMqQTeNYf3l5D" \
    "ax50yuEg9VVjtbDdSmvOkslGpVqsOl1NrmyN7gCvcvcRUQcxIiXJQc0CAwEAAaOC" \
    "BGgwggRkMIIBfwYKKwYBBAHWeQIEAgSCAW8EggFrAWkAdgCt9776fP8QyIudPZwe" \
    "PhhqtGcpXc+xDCTKhYY069yCigAAAX+Jw/reAAAEAwBHMEUCIE8AAjvwO4AffPn7" \
    "un67WykJ2hGB4n8qJE7pk4QYjWW+AiEA/pio1E9ALt30Kh/Ga4gRefH1ILbQ8n4h" \
    "bHFatezIcvYAdwB6MoxU2LcttiDqOOBSHumEFnAyE4VNO9IrwTpXo1LrUgAAAX+J" \
    "w/qlAAAEAwBIMEYCIQCdbj6FOX6wK+dLoqjWKuCgkKSsZsJKpVik6HjlRgomzQIh" \
    "AM7mYp5dBFmNLas3fFcP0rMMK+17n8u0GhFH2KpkPr1SAHYA6D7Q2j71BjUy51co" \
    "vIlryQPTy9ERa+zraeF3fW0GvW4AAAF/icP6jgAABAMARzBFAiAhjTz3PBjqRrpY" \
    "eH7us44lESC7c0dzdTcehTeAwmEyrgIhAOCaqmqA+ercv+39jzFWkctG36bazRFX" \
    "4gGNiKU0bctcMCcGCSsGAQQBgjcVCgQaMBgwCgYIKwYBBQUHAwIwCgYIKwYBBQUH" \
    "AwEwPAYJKwYBBAGCNxUHBC8wLQYlKwYBBAGCNxUIh73XG4Hn60aCgZ0ujtAMh/Da" \
    "HV2ChOVpgvOnPgIBZAIBJTCBrgYIKwYBBQUHAQEEgaEwgZ4wbQYIKwYBBQUHMAKG" \
    "YWh0dHA6Ly93d3cubWljcm9zb2Z0LmNvbS9wa2lvcHMvY2VydHMvTWljcm9zb2Z0" \
    "JTIwQXp1cmUlMjBUTFMlMjBJc3N1aW5nJTIwQ0ElMjAwMSUyMC0lMjB4c2lnbi5j" \
    "cnQwLQYIKwYBBQUHMAGGIWh0dHA6Ly9vbmVvY3NwLm1pY3Jvc29mdC5jb20vb2Nz" \
    "cDAdBgNVHQ4EFgQUiiks5RXI6IIQccflfDtgAHndN7owDgYDVR0PAQH/BAQDAgSw" \
    "MHwGA1UdEQR1MHOCEyouYXp1cmV3ZWJzaXRlcy5uZXSCFyouc2NtLmF6dXJld2Vi" \
    "c2l0ZXMubmV0ghIqLmF6dXJlLW1vYmlsZS5uZXSCFiouc2NtLmF6dXJlLW1vYmls" \
    "ZS5uZXSCFyouc3NvLmF6dXJld2Vic2l0ZXMubmV0MAwGA1UdEwEB/wQCMAAwZAYD" \
    "VR0fBF0wWzBZoFegVYZTaHR0cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9j" \
    "cmwvTWljcm9zb2Z0JTIwQXp1cmUlMjBUTFMlMjBJc3N1aW5nJTIwQ0ElMjAwMS5j" \
    "cmwwZgYDVR0gBF8wXTBRBgwrBgEEAYI3TIN9AQEwQTA/BggrBgEFBQcCARYzaHR0" \
    "cDovL3d3dy5taWNyb3NvZnQuY29tL3BraW9wcy9Eb2NzL1JlcG9zaXRvcnkuaHRt" \
    "MAgGBmeBDAECAjAfBgNVHSMEGDAWgBQPIF3XoVeV25LPK9DHwncEznKAdjAdBgNV" \
    "HSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwDQYJKoZIhvcNAQEMBQADggIBAKtk" \
    "4nEDfqxbP80uaoBoPaeeX4G/tBNcfpR2sf6soW8atAqGOohdLPcE0n5/KJn+H4u7" \
    "CsZdTJyUVxBxAlpqAc9JABl4urWNbhv4pueGBZXOn5K5Lpup/gp1HhCx4XKFno/7" \
    "T22NVDol4LRLUTeTkrpNyYLU5QYBQpqlFMAcvem/2seiPPYghFtLr5VWVEikUvnf" \
    "wSlECNk84PT7mOdbrX7T3CbG9WEZVmSYxMCS4pwcW3caXoSzUzZ0H1sJndCJW8La" \
    "9tekRKkMVkN558S+FFwaY1yARNqCFeK+yiwvkkkojqHbgwFJgCFWYy37kFR9uPiv" \
    "3sTHvs8IZ5K8TY7rHk3pSMYqoBTODCs7wKGiByWSDMcfAgGBzjt95SKfq0p6sj0C" \
    "+HWFiyKR+PTi2esFP9Vr9sC9jfRM6zwa7KnONqLefHauJPdNMt5l1FQGWvyco4IN" \
    "lwK3Z9FfEOFZA4YcjsqnkNacKZqLjgis3FvD8VPXETgRuffVc75lJxH6WmkwqdXj" \
    "BlU8wOcJyXTmM1ehYpziCpWvGBSEIsFuK6BC/iBnQEuWKdctAdbHIDlLctGgDWjx" \
    "xYDPZ/TtORGL8YaDnj6QHeOURIAHCtt6NCWKV6OR2HtMx+tCEvfi5ION1dyJ9hAX" \
    "+4K9FXc71ab7tdV/GLPkWc8Q0x1nk7ogDYcqKbiF"                         \
    "-----END CERTIFICATE-----"
/* ==================== */

/* ===== 功能开关 ===== */
// #define FUNCTION_WATCHDOG

/* ==================== */

/* ===== 调试开关 ===== */
#define NETDEBUG true

/* ==================== */

/* ===== 宏定义  ===== */

#if NETDEBUG
#define myprintf printf
#else
#define myprintf
#endif

#define POWER_SAVE_TICKCOUNT 20  // 无操作20秒后进入省电模式
#define INPUT_EXIT_TICKCOUNT 20  // 无操作20秒后退出输入模式
#define DOOR_CLOSE_TICKCOUNT 30  // 关门检测最多30秒，超过后退出流程
#define DOOR_OPEN_TICKCOUNT 3    // 开门检测最多5秒，超过后退出流程
#define DOOR_CANCEL_TICKCOUNT 10 // 关门，无物时等待按键

/* ==================== */
extern volatile char curFlow;
extern volatile char curPage;

// extern volatile char preFlow;
extern volatile bool flagNewBarcode;
extern volatile bool flagPreReserved;
extern volatile char flagInput;
extern volatile char flagCleadDB;
extern volatile char strInput[30]; // 当前输入字符串
extern volatile char bleEventType;
extern volatile u8 tickCheck;
extern volatile BoardParams boardParams;
extern volatile BleFlag bleFlag;
// extern char boxSize;         // 格口尺寸 0:S;1:M;2:L;3：Else
extern u16 boxCount;         // 格口数
extern Box *boxList;         // box列表
extern BoxFlag *boxFlagList; // boxflag列表
extern Event event;
extern Shipment *shipment;
extern AppSetting appSet;
extern AppFlag appFlag;
extern char *apmid;

extern void dprintf(const char *fmt, ...); // 打印调试日志
extern void printLog(char *info);
extern void randomBytes(unsigned char *output, size_t len, size_t *olen);
extern void randomDigits(unsigned char *output, size_t len, size_t *olen);
extern void sleep(int sec);
extern void clearInput(void);
extern bool inputOn(void);
extern bool inputOff(void);

extern u16 getBoxID(u8 side, u16 relay);
extern u8 getSizeValue(int v);
extern void getSizeText(u8 size, char *dst);
extern u32 timeToU32(Time *t);
extern void getSyncId(char *text);
extern void getReservationText(Event *p, char *text);
extern void getIncidentText(Incident *p, char *text);
extern void getTechnicianAction(TechnicianAction *p, char *text);
extern void getLabelRuleText(LabelRule *p, char *text);

extern void *mymalloc(size_t s);
extern void *mycalloc(size_t n, size_t s);
extern void *myrealloc(void *p, size_t s);
extern void myfree(void *p);

#endif
