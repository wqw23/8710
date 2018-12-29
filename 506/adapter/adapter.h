#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include "stdbool.h"
struct DeviceConfData {
    UINT8 relaystatus;
    UINT8 reserver_1;
    UINT8 reserver_2;
    UINT8 reserver_3;
    int   reserver_4;
    int   reserver_5;

};

#define bool	_Bool
#define true	1
#define false	0

//typedef unsigned char           uint8_t;
typedef unsigned char           uint8;
typedef char                    int8;
//typedef unsigned short int      uint16_t;
typedef unsigned short int      uint16;
typedef short int               int16;
//typedef unsigned int            uint32_t;
typedef unsigned int            uint32;
typedef int                     int32;
//typedef unsigned long long      uint64_t;
typedef unsigned long long      uint64;
typedef long long               int64;

void IOTHAL_Init();

#endif  //!_DEVICE_MANAGER_H_
