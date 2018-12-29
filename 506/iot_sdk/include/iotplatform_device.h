
#ifndef __IOTPLATFROM_DEVICE_HEADER__
#define __IOTPLATFROM_DEVICE_HEADER__ 

#include "datatype.h"
#include "iotdevice.h"

int IOTDev_Init(void);
void IOTDev_Exit(void);
void IOTDev_Event(DMEvent *event);
DeviceInformation *IOTDev_DeviceInformation(void);
int IOTDev_GetAttribute(UINT32 attribute_id, OCTData *attr);
int IOTDev_ExecuteAction(UINT32 action_id, OCTData *param);
const char * IOTDev_AdapterVersion(char *version);
int IOTDev_FastConnEnable(void);
void IOTDev_LoopCall(void);

#endif  //!__IOTPLATFROM_DEVICE_HEADER__

