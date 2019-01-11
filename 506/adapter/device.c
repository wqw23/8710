#define  MODULE_TAG "ADA"
//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "device.h"
#include "diag.h"
#include "timer_api.h"
//IOT SDK head
#include "datatype.h"
#include "iotsdk.h"
#include "iotsdk_ota.h"
#include "iotplatform_rtos.h"
#include "iotplatform_wifi.h"
#include "log.h"
#include "security_func.h"
//Adapter head file
#include "adapter.h"
#include "version.h"
#include "flash.h"
#include "lamp_effect.h"
#include "report_task.h"
#include "hal_dev.h"
#include "led.h"
#include "product_config.h"

#include "wifi_door_lock.h"

static DevAttribute g_attributes[] = DEVICE_ATTRIBUTES;
static DevAction g_actions[] = DEVICE_ACTIONS;

#define ATTRIBUTES_COUNT()    (sizeof(g_attributes)/sizeof(DevAttribute))
#define ACTIONS_COUNT()       (sizeof(g_actions)/sizeof(DevAction))

#define ASYNC_REPORT_ATTR_COUNT 3
#define STATIC_IP_FLAG          1//1使用静态IP,0是不使用静态IP

static DeviceInformation g_dev_infor = {
    g_attributes,        //attributes;
    ATTRIBUTES_COUNT(),  //attributes_count;
    g_actions,           //actions;
    ACTIONS_COUNT(),     //actions_count;
};

void _DeviceReset(void)
{
    log_debug0("%s entry\n", __FUNCTION__);

    struct DeviceConfData data;

    memset(&data,0,sizeof(struct DeviceConfData));

    //modify flash data.
    flash_reset(&data);

    //modify hardware status.
}

void _EntrySoftap(void)
{
    if (IOTSDK_State() == STATE_WORK) {
        IOTWifi_Reset();
        IOTDM_Exit();
        _DeviceReset();
        IOTSys_Reboot();
    } else {
        os_printf("device already in softap mode!!!!\n");
    }
}
int IOTHAL_LoadFlashData(void)
{
    int ret = E_FAILED;

    ret = flash_init(sizeof(struct DeviceConfData));
    if (E_FAILED == ret) {
        log_debug2("flash init error!\n");
        return E_FAILED;
    }

    //Load data from flash, E_FAILED is hardware fault.
    //set default config before load flash.
    //g_conf_data = flash_export_user_data();

    ret = flash_load();
    if (E_PARAM_ERROR == ret) {
        //flash data size more than buffer size.
        //write by different circumstances at below.

        //set current version to the flash.
        flash_set_version(CURRENT_FLASH_VERSION);
    } else if (E_NO_DATA == ret || E_FAILED == ret) {
        //if flash hardware fault or no data in it, then set default data.

        //set current version to the flash.
        flash_set_version(CURRENT_FLASH_VERSION);
    } else if (CURRENT_FLASH_VERSION != ret) {
        //different flash data verion
        //write by different circumstances at below.

        //set current version to the flash.
        flash_set_version(CURRENT_FLASH_VERSION);
    }

    flash_update();

    return E_SUCCESS;
}

void HALDev_OTALampEffect(void)
{

    led_wifi_trigger(true);
    vTaskDelay(100/portTICK_RATE_MS);

    led_wifi_trigger(false);
    vTaskDelay(200/portTICK_RATE_MS);

    led_wifi_trigger(true);
    vTaskDelay(100/portTICK_RATE_MS);

    led_wifi_trigger(false);
    vTaskDelay(800/portTICK_RATE_MS);
}

void HALDev_APLampEffect(void)
{

    led_wifi_trigger(true);
    vTaskDelay(200/portTICK_RATE_MS);

    led_wifi_trigger(false);
    vTaskDelay(800/portTICK_RATE_MS);
}

void HALDev_StationLampEffect(void)
{
    led_wifi_trigger(true);
    vTaskDelay(100/portTICK_RATE_MS);

    led_wifi_trigger(false);
    vTaskDelay(200/portTICK_RATE_MS);
}

void HALDev_OffLampEffect(void)
{
    led_wifi_trigger(false);
}

void _LampEffectInit(void)
{
    LECB lcb;

    lcb.off_mode = HALDev_OffLampEffect;
    lcb.ota_mode = HALDev_OTALampEffect;
    lcb.softap_mode = HALDev_APLampEffect;
    lcb.station_mode = HALDev_StationLampEffect;

    lamp_effect_init(lcb);
}

void IOTHAL_Init()
{
    _LampEffectInit();
    HalDev_Init();
    Wifi_Door_Lock_Init();
    if (IOTSDK_State() != STATE_OTA) {//if current is ota mode, do not init flash and schema
        if (IOTSDK_State() == STATE_WORK){
            lamp_effect_set(LAMP_EFFECT_STATION_MODE, PERIOD);
        }
        } else {
        log_debug2("Current in OTA mode\n");
    }
}

int IOTDev_Init(void)
{
    log_debug0("%s IOTDev_Init \n", __FUNCTION__);
    report_init(ASYNC_REPORT_ATTR_COUNT);
    return E_SUCCESS;
}

void IOTDev_Exit(void)
{
    report_deinit();
    log_debug0("%s entry\n", __FUNCTION__);
}

void IOTDev_Event(DMEvent *event)
{
    if(event == NULL) {
        return;
    }
    switch(event->event) {
    case IOTDM_EVENT_NETWORK_CONNECT:
        log_debug0("IOTDev_Event, NETWORK CONNECT STATUS =%d\n", event->param.network.connected);
        if (event->param.network.connected == 0){//connected=0, network configured and is currently disconnected
            lamp_effect_set(LAMP_EFFECT_STATION_MODE, PERIOD);
        }else if(event->param.network.connected == 1){//connected=1, network configured and is currently connected
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONNECT,event->param.network.connected);
        }else if(event->param.network.connected == -1){//connected=-1, network connect failed
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONNECT,event->param.network.connected);
        }
        break;
    case IOTDM_EVENT_NETWORK_CONFIG:
        log_debug0("IOTDev_Event, NETWORK CONFIG STATUS =%d\n", event->param.nwconfig.state);
        //state==1, network NOT configed, start config
        if(event->param.nwconfig.state == 1){
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            lamp_effect_set(LAMP_EFFECT_SOFTAP_MODE, PERIOD);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);//启动softap模式
        }else if(event->param.nwconfig.state == 3){//state==3, network, receive ssid passwd
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);
        }else if(event->param.nwconfig.state == 0){//state==0, network config FINISH, success
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            lamp_effect_set(LAMP_EFFECT_STATION_MODE, PERIOD);
        }else if(event->param.nwconfig.state == -1){//state==-1, network config password error, failed
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);
        }else if(event->param.nwconfig.state == -2){//state==-2, network config can't find special ssid, failed
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);
        }
        break;
    case IOTDM_EVENT_CLOUD_CONNECT:
        log_debug0("IOTDev_Event, CLOUD CONNECT STATUS =%d\n", event->param.cloudconn.connected);
        if(event->param.cloudconn.connected == 1){//connected with the server
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_CLOUD_CONNECT,event->param.cloudconn.connected);
        }else if(event->param.cloudconn.connected == -1){//connected with the server falied
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_CLOUD_CONNECT,event->param.cloudconn.connected);
        }
        break;
    case IOTDM_EVENT_AUTH:
        if(event->param.auth.result < 0){//auth failed
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_AUTH,event->param.auth.result);
        }
        break;
    case IOTDM_EVENT_ACTIVATE:
        break;
    case IOTDM_EVENT_CREATEGADGET:
        Get_Gadgetid_Function(event->param.creategadget.gadgetid);
        Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_CREATEGADGET,event->param.creategadget.created);
        break;
    case IOTDM_EVENT_FORCEBIND: //仅清除设备信�?
        break;
    case IOTDM_EVENT_RESET:    //清除 设备信息+配网信息
        //device reset
        _DeviceReset();
        break;
    case IOTDM_EVENT_OTA_STATUS:
        //status=1, ota started
        if(event->param.ota.status == 1){
            lamp_effect_set(LAMP_EFFECT_OTA_MODE, PERIOD);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_OTA_STATUS,event->param.ota.status);
        }else if(event->param.ota.status == 0){//status=0, ota finished,success
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            //Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_OTA_STATUS,event->param.ota.status);
        }else if(event->param.ota.status < 0){//status<0, ota finished,failed
            //Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_OTA_STATUS,event->param.ota.status);
        }
        break;
    default:
        break;
    }
}

DeviceInformation *IOTDev_DeviceInformation(void)
{
    return &g_dev_infor;
}

int IOTDev_GetAttribute(UINT32 attribute_id, OCTData *attr)
{
    if(attr == NULL) {
        return E_FAILED;
    }
    switch (attribute_id) {
        case GARDGET_DEVICE_ATTRIBUTE_FW_VERSION:
            attr->type = OCTDATATYPE_STRING;
            char version[5];
            iots_sprintf(version, "%d", ADA_VERSION);
            iots_strcpy(attr->value.String.String, version);
            break;
        case GARDGET_DEVICE_ATTRIBUTE_SN:
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String, "1234567890");
            break;
        case GARDGET_DEVICE_ATTRIBUTE_IPADDR:
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String, IOTWifi_LocalIP());
            break;
        case GARDGET_DEVICE_ATTRIBUTE_MAC:
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String, IOTSys_Mac());
            break;
        case GARDGET_DEVICE_ATTRIBUTE_HW_VERSION:
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String, "hw version");
            break;

        case GARDGET_DEVICE_ATTRIBUTE_OFFON:
        case GARDGET_DEVICE_ATTRIBUTE_PASSWORD:
        case GARDGET_DEVICE_ATTRIBUTE_FINGERPRINT:
        case GARDGET_DEVICE_ATTRIBUTE_WARNING:
        case GARDGET_DEVICE_ATTRIBUTE_CAR:
        case GARDGET_DEVICE_ATTRIBUTE_KEY:
        case GARDGET_DEVICE_ATTRIBUTE_BATTERY_LEVEL:
            attr->type = OCTDATATYPE_INTEGER;
            attr->value.Integer.Integer = Wifi_Door_Lock_Get_Int_Attribute(attribute_id);//从串口得到门锁的属性并存储
            break;

        case GARDGET_DEVICE_ATTRIBUTE_SET_PASSWORD:
        case GARDGET_DEVICE_ATTRIBUTE_SET_FINGERPRINT:
        case GARDGET_DEVICE_ATTRIBUTE_SET_CARD:
        case GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD:
        case GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT:
        case GARDGET_DEVICE_ATTRIBUTE_CLEAR_CARD:
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String,Wifi_Door_Lock_Get_String_Attribute(attribute_id));//从串口得到门锁的属性并存储
            break;
        case GARDGET_DEVICE_ATTRIBUTE_SERIAL_CODE:
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String,Wifi_Door_Lock_Get_SERIAL_CODE_From_MCU());//从串口得到门锁串码的属性
            break;

        default:
            return E_FAILED;
    }
    return E_SUCCESS;
}

int IOTDev_ExecuteAction(UINT32 action_id, OCTData *param)
{
    if(param == NULL) {
        return E_FAILED;
    }

    switch (action_id) {
        case GARDGET_DEVICE_ACTION_OTA_CHECK:
            os_printf("execute action(%08x) ota check\n", action_id);
            char version[5];
            iots_sprintf(version, "%d", ADA_VERSION);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_OTA_STATUS,1);
            IOTOTA_Start(version);
            break;
        default:
            return E_FAILED;
    }

    return E_SUCCESS;
}

const char* IOTDev_AdapterVersion(char * version)
{
    iots_sprintf(version, "%d", ADA_VERSION);
    return version;
}
void IOTDev_LoopCall()
{
    //log_debug0("IOTDev_LoopCall--MIN FREE MEMORY: %d\n", IOTSysP_GetFreeHeapSize());
}

int IOTDev_FastConnEnable(void)
{
    return STATIC_IP_FLAG;
}

