
#ifndef _IOTPLATFORM_RTOS_
#define _IOTPLATFORM_RTOS_

const char *IOTSysP_BSSID(char *bssid);
int IOTSysP_ParamSave(void* data, int size);
int IOTSysP_ParamLoad(int offset, void* data, int size);
int IOTSysP_LoadRof(int offset,void *buf,int size);
int IOTSysP_GetMemParam(void *mem, int size);
int IOTSysP_SetMemParam(void *mem, int size);
UINT32 IOTSysP_Random(int max);
UINT32 IOTSysP_GetFreeHeapSize(void);
void IOTSysP_TimerStart(void *timer,int msec,void (*timer_fun)(void *),void *arg,int repeat_flag);
void IOTSysP_TimerStop(void *timer);
void IOTSysP_SetTime(UINT32 utc);
UINT32 IOTSysP_GetRunningTime(void);
int IOTSysP_SntpSync(void);
#endif  //!_IOTPLATFORM_RTOS_
