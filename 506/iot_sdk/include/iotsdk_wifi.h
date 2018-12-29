
#ifndef __WIFI_HEADER__
#define __WIFI_HEADER__

int IOTWifi_Start(int force);
int IOTWifi_Reset(void);
int IOTWifi_NetworkReconnect(void);
int IOTWifi_NetworkIsOK(void);

#endif  //!__WIFI_HEADER__

