#ifndef _DEVICE_HEADER_
#define _DEVICE_HEADER_

#include "datatype.h"

/* -----------------------------------------------
 * Device Management Event
 *------------------------------------------------ */
typedef enum {
    IOTDM_EVENT_NONE,
    IOTDM_EVENT_NETWORK_CONFIG,
    IOTDM_EVENT_NETWORK_CONNECT,
    IOTDM_EVENT_CLOUD_CONNECT,
    IOTDM_EVENT_AUTH,
    IOTDM_EVENT_ACTIVATE,
    IOTDM_EVENT_CREATEGADGET,
    IOTDM_EVENT_FORCEBIND,
    IOTDM_EVENT_RESET,
    IOTDM_EVENT_OTA_STATUS,
    IOTDM_EVENT_ERROR
}DMEventType;
typedef struct {
    DMEventType event;
    union {
        struct {
            int state;
            //state==1, network NOT configed, start config
            //state==2, network ALREADY configed, skip config
            //state<0,  network config FINISH, failed
            //state==0, network config FINISH, success
        }nwconfig;
        struct {
            int connected;
            //connected<0, network unconfigured
            //connected=0, network configured and is currently disconnected
            //connected=1, network configured and is currently connected
        }network;
        struct {
            int connected;
            //connected=0, disconnected with the server
            //connected=1, connected with the server
        }cloudconn;
        struct {
            int result;
            //result=1, auth success
            //result=0, auth retry
            //result<0, auth failed
        }auth;
        struct {
            int activated;
            //activated=1,  FIRST activation after binding
            //activated=2,  alreay binded(not first activation after binding)
            //activated=0,  unbinded
            //activated<0,  failed
            const char *hubid;
        }activate;
        struct {
            int created;
            //created<0, device create failed
            //created=0, device create success
            //created=1, device already created
            const char *gadgetid;
        }creategadget;
        struct {
            int status;
            //status=2, recv check ack,adapter skip it
            //status=1, ota started
            //status=0, ota finished,success
            //status<0, ota finished,failed
        }ota;
        struct {
            int dm_errno;
            //dm_errno=0, no error
            //dm_errno=-1,cloud disconnect long time
        }error;
    }param;
}DMEvent;

/* -----------------------------------------------
 * Device Data
 *------------------------------------------------ */
typedef enum
{
    OCTDATATYPE_INTEGER = 0,       //integer data type, such as int, long
    OCTDATATYPE_FLOAT   = 1,       //float data type, such as float, double
    OCTDATATYPE_STRING  = 2,       //string data type, such as char*
    OCTDATATYPE_ARRAY_INTEGER = 3, //array values
    OCTDATATYPE_ARRAY_FLOAT   = 4, //array values
    OCTDATATYPE_LARGESTRING   = 5  //large string, it is only a buf pointer
}OCTDATATYPE;

#define OCTDATA_VALUE_LENGTH  (512)
//--attribute type value
typedef struct
{
    UINT32  Integer;
}OCTData_Integer;//integer value

typedef struct
{
    float  Float;
}OCTData_Float;//Float value

typedef struct
{
    char String[OCTDATA_VALUE_LENGTH];
}OCTData_String;//char * value

typedef struct
{
    char *LargeString;
}OCTData_LargeString;

typedef struct
{
    UINT32  length;
    union {
        UINT32  Integers[(OCTDATA_VALUE_LENGTH-sizeof(UINT32))/sizeof(UINT32)];
        float   Floats[(OCTDATA_VALUE_LENGTH-sizeof(UINT32))/sizeof(float)];
    }u;
}OCTData_Array;//array value

typedef struct
{
    union
    {
        OCTData_Integer         Integer;    //integer value
        OCTData_Float           Float;      //Float value
        OCTData_String          String;     //char * value
        OCTData_Array           Array;      //array  value
        OCTData_LargeString     LargeString;//large string
    }value;
    OCTDATATYPE  type;
}OCTData;

/* -----------------------------------------------
 * Device Informations
 *------------------------------------------------ */
typedef struct {
    UINT32 attibute_id;
    UINT32 data_type;
}DevAttribute;

typedef struct {
    UINT32 action_id;
    UINT32 data_type;
}DevAction;

typedef struct {
    DevAttribute *attributes;
    UINT32     attributes_count;
    DevAction    *actions;
    UINT32     actions_count;
}DeviceInformation;

#endif  //!_DEVICE_HEADER_
