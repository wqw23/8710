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
#include "schema_task.h"
#include "flash.h"
#include "lamp_effect.h"
#include "report_task.h"
#include "keyboard.h"
#include "hal_key.h"
#include "hal_dev.h"
#include "led.h"
#include "relay.h"
#include "product_config.h"
#include "factory_event_task.h"

#include "wifi_door_lock.h"

static DevAttribute g_attributes[] = DEVICE_ATTRIBUTES;
static DevAction g_actions[] = DEVICE_ACTIONS;

#define ATTRIBUTES_COUNT()    (sizeof(g_attributes)/sizeof(DevAttribute))
#define ACTIONS_COUNT()       (sizeof(g_actions)/sizeof(DevAction))

#define ASYNC_REPORT_ATTR_COUNT 3
#define STATIC_IP_FLAG          1//1使用静态IP,0是不使用静态IP

static xTaskHandle key_task_Handle;

static struct DeviceConfData *g_conf_data;
static int m_dev_status = 0;
static int m_dev_memory_mode = 0;

#define MEMORY_MODE_INIT_VALUE      0x02
#define GET_MEMORY_MODE_FROM_FLASH(x)              (x & 0x02 ? 1:0)
#define UPDATE_MEMORY_MODE(x,y)                    (x ? (y|=0x02) : (y&=0xFD))
#define GET_RELAY_STATUS_FROM_FLASH(x)             (x & 0x01)
#define UPDATE_RELAY_STATUS(x,y)                   (x ? (y|=0x01) : (y&=0xFE))

static xTaskHandle key_task_Handle;
extern int factory_mode;

static DeviceInformation g_dev_infor = {
    g_attributes,        //attributes;
    ATTRIBUTES_COUNT(),  //attributes_count;
    g_actions,           //actions;
    ACTIONS_COUNT(),     //actions_count;
};

void timer_actions(void *status){
    async_report_attr(GARDGET_DEVICE_ATTRIBUTE_DELAY_TIME, ASYNC_UPDATE_FLASH);
}
void _DeviceReset(void)
{
    log_debug0("%s entry\n", __FUNCTION__);

    struct DeviceConfData data;

    memset(&data,0,sizeof(struct DeviceConfData));

    //modify global variable.
    m_dev_memory_mode = 1;

    //modify flash data.
    UPDATE_MEMORY_MODE(m_dev_memory_mode,data.relaystatus);
    UPDATE_RELAY_STATUS(m_dev_status,data.relaystatus);
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
    g_conf_data = flash_export_user_data();
    g_conf_data->relaystatus = MEMORY_MODE_INIT_VALUE;

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

    if(GET_MEMORY_MODE_FROM_FLASH(g_conf_data->relaystatus)){
        m_dev_memory_mode = 1;
    }else{
        m_dev_memory_mode = 0;
    }
    m_dev_status = GET_RELAY_STATUS_FROM_FLASH(g_conf_data->relaystatus);

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
        schema_task_init(flash_export_timers(), timer_actions);
        } else {
        log_debug2("Current in OTA mode\n");
    }
}

int IOTDev_Init(void)
{
    log_debug0("%s IOTDev_Init \n", __FUNCTION__);
    report_init(ASYNC_REPORT_ATTR_COUNT);
    schema_task_init(flash_export_timers(), timer_actions);
    schema_task_resume();
    return E_SUCCESS;
}

void IOTDev_Exit(void)
{
    schema_task_suspend();
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
        if(IOTSDK_State() != STATE_WORK){
            break;
        }else if (event->param.network.connected < 0){//connected<0, network unconfigured

        }else if (event->param.network.connected == 0){//connected=0, network configured and is currently disconnected
            lamp_effect_set(LAMP_EFFECT_STATION_MODE, PERIOD);
        } else if(event->param.network.connected == 1){//connected=1, network configured and is currently connected
        }
        break;
    case IOTDM_EVENT_NETWORK_CONFIG:
        log_debug0("IOTDev_Event, NETWORK CONFIG STATUS =%d\n", event->param.nwconfig.state);
        //state==1, network NOT configed, start config
        if(event->param.nwconfig.state == 1){
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            lamp_effect_set(LAMP_EFFECT_SOFTAP_MODE, PERIOD);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);//启动softap模式
        }else if(event->param.nwconfig.state == 2){//state==2, network ALREADY configed, skip config
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);
        } else if(event->param.nwconfig.state < 0){//state<0,  network config FINISH, failed
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);
        }else if(event->param.nwconfig.state == 0){//state==0, network config FINISH, success
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            lamp_effect_set(LAMP_EFFECT_STATION_MODE, PERIOD);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_NETWORK_CONFIG,event->param.nwconfig.state);
        }
        break;
    case IOTDM_EVENT_CLOUD_CONNECT:
        log_debug0("IOTDev_Event, CLOUD CONNECT STATUS =%d\n", event->param.cloudconn.connected);
        if(event->param.cloudconn.connected == 1){
            lamp_effect_set(LAMP_EFFECT_OFF_MODE, PERIOD);
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_CLOUD_CONNECT,event->param.cloudconn.connected);
        }else if(event->param.cloudconn.connected == 0){
            Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_CLOUD_CONNECT,event->param.cloudconn.connected);
        }
        break;
    case IOTDM_EVENT_ACTIVATE:
        break;
    case IOTDM_EVENT_CREATEGADGET:
        Wifi_Door_Lock_Send_IOT_Event_Frame_To_Mcu(IOTDM_EVENT_CREATEGADGET,event->param.creategadget.created);
        Get_Gadgetid_Function(event->param.creategadget.gadgetid);
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
        case GARDGET_DEVICE_ATTRIBUTE_DELAY_TIME:
            attr->type = OCTDATATYPE_STRING;
            schema_task_get_delay_info(attr->value.String.String);
            break;
        case GARDGET_DEVICE_ATTRIBUTE_TIMEING:
            attr->type = OCTDATATYPE_STRING;
            schema_task_get_timer_info(attr->value.String.String);
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
#ifndef Wifi_Door_Lock_Open_Ignore_Event
        case GARDGET_DEVICE_ATTRIBUTE_CLEAR_PASSWORD:
        case GARDGET_DEVICE_ATTRIBUTE_CLEAR_FINGERPRINT:
#endif
            attr->type = OCTDATATYPE_STRING;
            iots_strcpy(attr->value.String.String,Wifi_Door_Lock_Get_String_Attribute(attribute_id));//从串口得到门锁的属性并存储
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
        case GARDGET_DEVICE_ACTION_SETDELAY_TIME:
            if(param->type != OCTDATATYPE_STRING) {
                os_printf("param error\n");
                return E_FAILED;
            }
            log_debug0("execute action(%08x) set delay=%s\n", action_id, param->value.String.String);
            schema_task_set_delay_info(param->value.String.String);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_DELAY_TIME, ASYNC_UPDATE_FLASH);
            break;
        case GARDGET_DEVICE_ACTION_SETTIMEING:
            if(param->type != OCTDATATYPE_STRING) {
                os_printf("param error\n");
                return E_FAILED;
            }
            log_debug0("execute action(%08x) set timing=%s\n", action_id, param->value.String.String);
            schema_task_set_timer_info(param->value.String.String);
            sync_report_attr(GARDGET_DEVICE_ATTRIBUTE_TIMEING, ASYNC_UPDATE_FLASH);
            break;
        case GARDGET_DEVICE_ACTION_OTA_CHECK:
            os_printf("execute action(%08x) ota check\n", action_id);
            char version[5];
            iots_sprintf(version, "%d", ADA_VERSION);
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

