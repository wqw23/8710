
#ifndef __IOTSDK_HEADER__
#define __IOTSDK_HEADER__

#include "iotdevice.h"

//reset reason
#define RST_REASON_OFFLINE_TIMEOUT 0 

typedef enum {
    STATE_OTA,
    STATE_WORK,
    STATE_SOFTAP
}eSdkState;

int IOTSDK_SoftStart(void);
int IOTSDK_Init(void);
void IOTSDK_Exit(void);
eSdkState IOTSDK_State(void);

int  IOTDM_Loop(void);
void IOTDM_Exit(void);
int IOTDM_AttributesChanged(UINT32 *attrIDList, UINT32 number);
void IOTDM_EventCallback(DMEvent *event);
int IOTCloud_IsConnected(void);
void IOTDM_LogLevel(int level);

int IOTConf_Save(void *data, int size);
int IOTConf_Load(void *data, int size);
int IOTConf_QuickRestart(void);
int IOTConf_SetStartReason(UINT32 reason);
void *IOTM_Malloc(UINT32 uByteNumber);
void IOTM_Free(void *buffer);

typedef struct {
    char sdk_version[32];
    const char *pn;
    const char *module_type;
    const char *cloudstr;
    UINT32 gadget_type_id;
    UINT32 hub_type;
}SDKInformation;
int IOTSDK_GetInformation(SDKInformation *info);

//===========================================
// Following functions is implemented by PLATFORM
const char *IOTSys_Mac(void);
void IOTSys_Reboot(void);
const char *IOTWifi_LocalIP(void);

#endif  //!__IOTSDK_HEADER__

