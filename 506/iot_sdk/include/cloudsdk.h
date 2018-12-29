
#ifndef __CLOUDSDK_HEADER__
#define __CLOUDSDK_HEADER__

#include "cJSON.h"
#include "datatype.h"

typedef int (*ReceiveACK_CB)(int isok, char *msg_type, cJSON *itemdata, void *param);

#if 1//GADGET_FUNCTION_ENABLE
typedef int (*Function_CB)(int cmdret, UINT32 function_key, cJSON *data, void *param);

int IOTCloud_IsConnected(void);
int IOTCloud_GetGadgetFunction(UINT32 function_key,
                        cJSON *data, ReceiveACK_CB cb, void *param);
int IOTCloud_SetGadgetFunction(UINT32 function_key,
                        cJSON *data, ReceiveACK_CB cb, void *param);
int IOTCloud_DelGadgetFunction(UINT32 function_key,
                        cJSON *data, ReceiveACK_CB cb, void *param);

int IOTCloud_RegisterFunctionCBRange(
                        UINT32 function_key_min, UINT32 function_key_max,
                        Function_CB function_cb, void *function_param);
int IOTCloud_RegisterFunctionCB(UINT32 function_key,
                        Function_CB function_cb, void *function_param);
int IOTCloud_UnRegisterFunctionCB(Function_CB function_cb);
#endif  //GADGET_FUNCTION_ENABLE
#endif  //!__CLOUDSDK_HEADER__

