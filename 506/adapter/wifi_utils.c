//Standard head file

//RTK8710 head file
#include "FreeRTOS.h"
#include "wifi_conf.h"
#include "wifi_structures.h"
#include "wifi_constants.h"
#include "lwip_netconf.h"
//IOT SDK head
#include "iotdevice.h"
#include "iotsdk.h"
#include "iotsdk_wifi.h"
#include "log.h"
//Adapter head file
#include "wifi_utils.h"
#include "udp_task.h"
#include "product_config.h"

int factory_mode = false;
static int scan_wifi_finish = false;

UINT8 connect_wifi(char *ssid,char *passwd)  //连接到指定的SSID
{
    UINT8 count=3;
    UINT8 cycle_flag=0;
    UINT8 cycle_flag_count=7;
    rtw_wifi_config_t config;
    UINT8 val=0;

    memset(&config,0,sizeof(config));
    memcpy(config.ssid,ssid,strlen(ssid));
    memcpy(config.password,passwd,strlen(passwd));//配置路由器密码
    log_debug0("connect_ssid is ok\n");
    if(wifi_on(RTW_MODE_STA) < 0)
        log_debug0("\n\r[WLAN_SCENARIO_EXAMPLE] ERROR: wifi_on failed\n");

    do{//扫描3次连接SSID
        val=wifi_connect(config.ssid,RTW_SECURITY_WPA2_AES_PSK,config.password,strlen(config.ssid),strlen(config.password),-1,NULL);
        if(val != RTW_SUCCESS)
        {
            log_error("wifi_station_connect error!!!%d\n",count);
            cycle_flag=0;
        }else{
            log_debug0("connect to %s success!!!!!\n",config.ssid);
            LwIP_DHCP(0, DHCP_START);  //分配ip的接口
            cycle_flag=1;
            count=1;
        }
    }while(--count);
    if(cycle_flag)
    {
        do{
            if(IOTWifiP_NetworkIsOK())//返回值为1 说明密码正确,返回0 密码错误 E_SUCCESS != IOTWifiP_NetworkIsOK()
            {
                log_debug2("connect_wifi_PASSWD_success !PASSWD=%s\n",config.password);
                return 1;
            }else{
                log_debug2("connect_continue!!!!!!%d--IOTWifiP_NetworkIsOK()=%d\n",cycle_flag_count,IOTWifiP_NetworkIsOK());
                vTaskDelay(1000/portTICK_RATE_MS);
            }
        }while(--cycle_flag_count);
    }
    log_debug2("connect_wifi_error!!!%d\n",cycle_flag_count);
    return 0;
}

static rtw_result_t scan_result_handler(rtw_scan_handler_result_t* malloced_scan_result)
{
    if (malloced_scan_result->scan_complete != RTW_TRUE) {
        rtw_scan_result_t* record = &malloced_scan_result->ap_details;
        record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */
        log_debug0("ssid is %s\n", record->SSID.val);
        log_debug0("record->signal_strength is %d\n", record->signal_strength);
        if ((strcmp(record->SSID.val,FACTORY_WIFI_SSID)) == 0) {
            log_debug0("factory wifi ssid is %s is found !!! \n", record->SSID.val);
            factory_mode = true;
            scan_wifi_finish = true;
            return RTW_SUCCESS;
        }
    }
    return RTW_ERROR;
}

UINT8 isFactoryMode_wifi(void)//在main.c中调用，判断是否扫描到SSID
{
    if(wifi_on(RTW_MODE_STA)<0){ //set station mode
        log_debug0("\n\r[WLAN_SCENARIO_EXAMPLE] ERROR: wifi_on failed\n");
    }
    log_debug0("isFactoryMode_wifi \n");
    scan_wifi_finish = false;

    if(wifi_scan_networks(scan_result_handler, NULL) != RTW_SUCCESS){
        log_debug0("\n\r[WLAN_SCENARIO_EXAMPLE] ERROR: wifi_scan_networks() failed\n");
    }

    while(!scan_wifi_finish){
        log_debug2("scan task running....\n");
        vTaskDelay(1000/portTICK_RATE_MS);
    }

    return factory_mode;
}

