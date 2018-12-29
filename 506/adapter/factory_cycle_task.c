//Standard head file

//RTK8710 head file
#include "FreeRTOS.h"
//IOT SDK head
#include "iotdevice.h"
#include "iotsdk.h"
#include "iotsdk_wifi.h"
#include "log.h"
//Adapter head file
#include "factory_cycle_task.h"
#include "wifi_utils.h"
#include "led.h"
#include "relay.h"
#include "product_config.h"

extern int factory_mode;
UINT8 pcba_factory_key_status=0;
UINT8 pcba_factory_key_flag=0;

UINT8 PcbaIsKeyStatusHigh(void)
{
    return pcba_factory_key_flag;
}
void factory_cycle_auto_task(void *arg)  //进入工厂模式，周期执行自动功能
{
    log_debug0("factory_cycle_auto_task....\n");
    pcba_factory_key_flag=1;
    while(factory_mode)
    {
        if(pcba_factory_key_status)
        {
            log_debug0(" factory mode test case for JDQ on/off three times \n");
            relay_trigger(true);
            vTaskDelay(500/portTICK_RATE_MS);
            relay_trigger(false);
            vTaskDelay(500/portTICK_RATE_MS);
            relay_trigger(true);
            vTaskDelay(500/portTICK_RATE_MS);
            relay_trigger(false);
            vTaskDelay(500/portTICK_RATE_MS);
            relay_trigger(true);
            vTaskDelay(500/portTICK_RATE_MS);
            relay_trigger(false);
            factory_mode = false;
            log_debug0("Exit factory mode !!!\n");
            led_wifi_trigger(false);
            led_power_trigger(false);
            vTaskDelete(NULL);

        }else{
            log_debug0("PCBA Factory test case runing ....\n");
            led_wifi_trigger(false);
            led_power_trigger(true);
            vTaskDelay(1000/portTICK_RATE_MS);
            led_power_trigger(false);
            led_wifi_trigger(true);
            vTaskDelay(1000/portTICK_RATE_MS);
        }

    }
}

