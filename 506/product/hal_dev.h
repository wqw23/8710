#ifndef _HAL_DEV_H_
#define _HAL_DEV_H_

#include "iotsdk.h"
//current flash data type version
#define CURRENT_FLASH_VERSION   2

/* ----------------------------------------------------
 * Device information
 * ---------------------------------------------------- */
#define GARDGET_DEVICE_ATTRIBUTE_FW_VERSION  (0x10000001)
#define GARDGET_DEVICE_ATTRIBUTE_SN          (0x10000004)
#define GARDGET_DEVICE_ATTRIBUTE_IPADDR      (0x1000000b)
#define GARDGET_DEVICE_ATTRIBUTE_MAC         (0x1000000c)
#define GARDGET_DEVICE_ATTRIBUTE_HW_VERSION  (0x10000010)

#define GARDGET_DEVICE_ATTRIBUTE_OFFON              (0x000c0000)//门开关状态
#define GARDGET_DEVICE_ATTRIBUTE_PASSWORD           (0x000c0005)//门锁密码开锁
#define GARDGET_DEVICE_ATTRIBUTE_FINGERPRINT        (0x000c0008)//指纹开锁

#define GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD       (0x000c0009)//设置密码状态上报
#define GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT    (0x000c000c)//设置指纹状态上报
#define GARDGET_DEVICE_ATTRIBUTE_SET_CARD           (0x000c0015)//设置IC卡状态上报

#define GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD     (0x000c000d)//密码已经清除报告
#define GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT  (0x000c0010)//指纹已经清除报告
#define GARDGET_DEVICE_ATTRIBUTE_CLEAR_CARD         (0x000c0016)//IC卡已经清除报告

#define GARDGET_DEVICE_ATTRIBUTE_WARNING            (0x000c0011)//门锁警告状态上报
#define GARDGET_DEVICE_ATTRIBUTE_CAR                (0x000c0017)//IC卡开锁
#define GARDGET_DEVICE_ATTRIBUTE_KEY                (0x000c0013)//钥匙开锁
#define GARDGET_DEVICE_ATTRIBUTE_SERIAL_CODE        (0x000c0014)//门锁串号信息上报

#define GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL      (0x000c1001)//电池电量


#define GARDGET_DEVICE_ACTION_OTA_CHECK      (0x1000000a)

#define GARDGET_DEVICE_ACTION_OFFON             (0x000c0000)//开门动作，只支持1开门
#define GARDGET_DEVICE_ACTION_SET_PASSWORD      (0x000c0009)//设置常用密码
#define GARDGET_DEVICE_ACTION_CLEAR_PASSWORD    (0x000c000d)//删除常用密码

#define DEVICE_ATTRIBUTES {\
    { GARDGET_DEVICE_ATTRIBUTE_FW_VERSION,  OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_SN,          OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_IPADDR,      OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_MAC,         OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_HW_VERSION,  OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_OFFON,               OCTDATATYPE_INTEGER },\
    { GARDGET_DEVICE_ATTRIBUTE_PASSWORD,            OCTDATATYPE_INTEGER },\
    { GARDGET_DEVICE_ATTRIBUTE_FINGERPRINT,         OCTDATATYPE_INTEGER }, \
    { GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD,        OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT,     OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_SET_CARD,            OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD,      OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT,   OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_CLEAR_CARD,          OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_SERIAL_CODE,         OCTDATATYPE_STRING }, \
    { GARDGET_DEVICE_ATTRIBUTE_WARNING,             OCTDATATYPE_INTEGER }, \
    { GARDGET_DEVICE_ATTRIBUTE_CAR,                 OCTDATATYPE_INTEGER },\
    { GARDGET_DEVICE_ATTRIBUTE_KEY,                 OCTDATATYPE_INTEGER }, \
    { GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL,       OCTDATATYPE_INTEGER }\
}

#define DEVICE_ACTIONS {\
    { GARDGET_DEVICE_ACTION_OTA_CHECK,      OCTDATATYPE_INTEGER },\
    { GARDGET_DEVICE_ACTION_OFFON,          OCTDATATYPE_INTEGER },\
    { GARDGET_DEVICE_ACTION_SET_PASSWORD,   OCTDATATYPE_STRING },\
    { GARDGET_DEVICE_ACTION_CLEAR_PASSWORD, OCTDATATYPE_INTEGER }\
}
#define PERIOD 10

/*************************
 * HAL Interface
 *************************/
void HalDev_Init(void);
#endif
