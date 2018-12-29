//Standard head file

//RTK8710 head file
#include "FreeRTOS.h"
#include "task.h"
//IOT SDK head
#include "iotdevice.h"
#include "iotsdk.h"
#include "iotsdk_wifi.h"
#include "log.h"
//Adapter head file
#include "factory_net_task.h"
#include "udp_task.h"
#include "wifi_utils.h"

void factory_connect_net_task(void *arg) //连接到网络进行创建udp接收发送task
{
    log_debug0("factory_connect_net_task....\n");
    xTaskCreate(udp_recive_task,"udp_recive_task",512,NULL,4,NULL);
    xTaskCreate(udp_send_task,"udp_send_task",512,NULL,4,NULL);
    xTaskCreate(factory_mode_auto_task,"factory_mode_auto_task",512,NULL,4,NULL);
    xTaskCreate(factory_key_event_task,"factory_key_event_task",512,NULL,4,NULL);
    vTaskDelete(NULL);
}

