
#ifndef _IOTPLATFORM_WIFI_HEADER_
#define _IOTPLATFORM_WIFI_HEADER_
#include "commcloud.h"
int IOTWifiP_NetWorkConnectStatus(void);
int IOTWifiP_NetworkIsOK(void);
int IOTWifiP_Reset(void);
int IOTWifiP_SoftapStart(const char *hotspot);
int IOTWifiP_SoftapStop(void);
int IOTWifiP_StationConfig(const char *ssid, const char *passwd);
int IOTWifiP_StationConnect(void);
int IOTWifiP_GetStationIpInfo(UINT32 *puiIp, UINT32 *puiMask, UINT32 *puiGateway);
int IOTWifiP_StationStaticIpRelase(int uiReleaseReason);
int IOTWifiP_StationDisconnect(void);
int IOTWifiP_GetWifiListInfo(bssid_info *bssid_array,int *bssid_current_number,int bssid_max_number);
int IOTWifiP_ConnectToAp(void);
#endif  //!_IOTPLATFORM_WIFI_HEADER_

