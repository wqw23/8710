//Standard head file

//RTK8710 head file
#include <lwip_netconf.h>
#include <example_entry.h>
#include <dhcp/dhcps.h>
#include <wlan/wlan_test_inc.h>
#include <wifi/wifi_conf.h>
#include <wifi/wifi_util.h>
#include "FreeRTOS.h"
#include "task.h"
#include "diag.h"
#include "semphr.h"
#include "log_service.h"
#include "atcmd_wifi.h"
#include "tcpip.h"
#include "device.h"
#include "main.h"
//IOT SDK head
#include "log.h"
#include "iotsdk_wifi.h"
#include "datatype.h"
#include "iotsdk.h"
//Adapter head file
#include "adapter.h"
#include "udp_task.h"
#include "wifi_utils.h"
#include "factory_net_task.h"
#include "factory_cycle_task.h"
#include "product_config.h"

#define FORCE_NETWORK_CONFIG   1

int IOTConf_QuickRestart(void);

void main_task(void *arg)
{
#if 1
    int retval = E_SUCCESS;

    do {
        //if is not soft restart and not sdk restart and softap flag is true.
        if(!IOTConf_QuickRestart() && Wifi_Door_Lock_Enter_Softap()){
            if (STATE_SOFTAP == IOTSDK_State() ) {
                do {
                    //进入网络配置
                    retval = IOTWifi_Start(FORCE_NETWORK_CONFIG);
                    log_debug2("Softap config return %d\n", retval);
                    if (retval == E_SUCCESS) {
                        IOTDM_Loop();  // 解绑后Loop会退出，Loop内会清除station_config
                        break;
                    }
                    vTaskDelay(1000/portTICK_RATE_MS);
                }while (retval != E_SUCCESS);
            }
        }

        if(STATE_SOFTAP != IOTSDK_State()){
            // 进入LOOP
            IOTDM_Loop();  // 解绑后Loop会退出，Loop内会清除station_config
        }
        vTaskDelay(1000/portTICK_RATE_MS);
    }while(1);

#else

    int retval = E_SUCCESS;

    log_debug0("Enter main_task....\n");

    do {
        //判断是否配置过网络
        if(IOTSDK_State() == STATE_SOFTAP) {
            if(!IOTConf_QuickRestart() && Wifi_Door_Lock_Enter_Softap() &&retval == E_SUCCESS)
            {
                wifi_on(RTW_MODE_STA); //set station mode
            }
            log_debug0("Start softap!\n");
            //进入网络配置
            retval = IOTWifi_Start(FORCE_NETWORK_CONFIG);
        }

        if(retval == E_SUCCESS) {
            // 进入LOOP
            IOTDM_Loop();  // 解绑后Loop会退出，Loop内会清除station_config
        } else {
            log_debug0("wifi config failed, reboot...\n");
        }
        vTaskDelay(1000/portTICK_RATE_MS);
    }while(1);

    IOTSys_Reboot();
    vTaskDelete(NULL);

#endif
}

void _normal_mode(void) //未扫描到，进入正常模式
{
    log_debug0("init_task....normal_mode\n");
    // create sdk task
    if(xTaskCreate(main_task, ((const char*)"main_task"), 1024*1 + 512, NULL, tskIDLE_PRIORITY + 3 + PRIORITIE_OFFSET, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
    log_debug0("create main task  ....\n");
}
void _factory_mode(void)
{
    log_debug0("_factory_mode...........\n");
    if(connect_wifi(FACTORY_WIFI_SSID,PASSWD)==true)
    {
        xTaskCreate(factory_connect_net_task,"factory_connect_net_task",512,NULL,4,NULL);
    }else{
        xTaskCreate(factory_cycle_auto_task,"factory_cycle_auto_task",512,NULL,4,NULL);
    }
}

void init_task(void *arg)
{
    //wifi_on(RTW_MODE_STA); //set station mode
    log_debug0("init_task....\n");
    if(IOTSDK_SoftStart()) {
        _normal_mode();
    } else {
#ifdef ENADLE_FACTORY_MODE
        if (isFactoryMode_wifi()) { //扫描到SSID进入工厂模式
            _factory_mode();
        } else {
            _normal_mode();
        }
#else
        _normal_mode();
#endif
    }
    vTaskDelete(NULL);
}
void user_task(void *arg)
{
    IOTDM_LogLevel(2);
    //rtw_msleep_os(4000);
    // initialize
    if(IOTRTOS_Init()!= E_SUCCESS)
    {
        log_error("RTOS init failed\n");
        return;
    }

    if(IOTSDK_Init()!= E_SUCCESS)
    {
        log_error("confit init failed\n");
        return;
    }

    IOTHAL_Init();
    xTaskCreate(init_task,"init_task",512,NULL,4,NULL);


    vTaskDelete(NULL);
}


void User_task()
{
    if(xTaskCreate(user_task, ((const char*)"init"), 512, NULL, tskIDLE_PRIORITY + 3 + PRIORITIE_OFFSET, NULL) != pdPASS)
        printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
}


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
    /* Initialize log uart and at command service */
    //console_init();
    ReRegisterPlatformLogUart();

    /* pre-processor of application example */
    //pre_example_entry();

    /* wlan intialization */
#if defined(CONFIG_WIFI_NORMAL) && defined(CONFIG_NETWORK)
    wlan_network();
#endif
    rtc_init();
    User_task();
    //example_ssl_server();
    /* Execute application example */
    //example_entry();
    /*Enable Schedule, Start Kernel*/
#if defined(CONFIG_KERNEL) && !TASK_SCHEDULER_DISABLED
#ifdef PLATFORM_FREERTOS
    vTaskStartScheduler();
#endif
#else
    RtlConsolTaskRom(NULL);
#endif
}
