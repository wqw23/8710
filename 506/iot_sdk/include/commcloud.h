
#ifndef _IOTCLOUD_HEADER_
#define _IOTCLOUD_HEADER_

#include "iotdevice.h"
#include "iotsdk_common.h"

typedef struct
{
    int rssi;
    char bssid[16];
}bssid_info;

int  IOTCloudI_Init(const char *hubmac);
void IOTCloudI_Exit(void);

int  IOTCloudI_DoConnect(void);
int  IOTCloudI_ActivateHub(void);
void IOTCloudI_ReportHeartBit(void);
int  IOTCloudI_Firmware(void);
int  IOTCloudI_ReportHubHalStatus(UINT32 HubIsOnline);
int  IOTCloudI_CreateNewGadget(void);
int  IOTCloudI_ReportGadgetConnect(void);
int  IOTCloudI_ReportGadgetStatus(UINT32 *attrIDList, UINT32 number, OCTData *attr_data);
void IOTCloudI_SetCloudDisconnect(void);
int  IOTCloudI_CheckUpdateVersion(int force_check);
int  IOTCloudI_AuthGetToken(ApInfo *apInfo);
int  IOTCloudI_AuthTokenRefresh(const char *refresh_token, char *spdid);
int  IOTCloud_IsActivated(void);

const char *IOTCloudI_GetUrl(void);
const char *IOTCloudI_GetOtaUrl(void);

#endif  //!_IOTCLOUD_HEADER_

