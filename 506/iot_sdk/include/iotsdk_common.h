#ifndef __IOTSDK_COMMON_HEADER__
#define __IOTSDK_COMMON_HEADER__

#include "datatype.h"

#define WIFI_SSID_LENGTH    (36)                  
#define WIFI_PSWD_LENGTH    (68)
//wifi info
typedef struct 
{   
    char ssid[33];
    char bssid[16];
    char pwd[65];
    char utc[16];
    char localserver[8];
    char serverlevel[8];
    char pre_code[128];
    char time_zone[32];
    int configed;/*set 1 when network configed */
    char code_verifier[16];
    char reserved[16];
}ApInfo;

//ota info
typedef struct {
    int  count;    //ota upgrade count
    char host[64]; //ota upgrade firmware host
    char url[64];  //ota upgtade firmware url
    char md5[64];  //ota upgrade firmware md5
    UINT32 uiVersion;
}OtaInfo;

#endif  //!__IOTSDK_COMMON_HEADER__
