#define  MODULE_TAG "LAMP"
//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
//IOT SDK head
#include "iotsdk.h"
#include "datatype.h"
#include "log.h"
//Adapter head file
#include "lamp_effect.h"

#define false 0
#define true 1

static int x_mode;
LECB x_cb;
int task_run_flag = false;
xTaskHandle lamp_effect_handle;
static int period;
TimerHandle_t lamp_effect_timer = NULL;

void _lamp_effect_thread(void *arg)
{
    while(1) {
        switch(x_mode)
        {
            case LAMP_EFFECT_OTA_MODE:
                x_cb.ota_mode();
                break;
            case LAMP_EFFECT_SOFTAP_MODE:
                x_cb.softap_mode();
                break;
            case LAMP_EFFECT_STATION_MODE:
                x_cb.station_mode();
                break;
            case LAMP_EFFECT_OFF_MODE:
                x_cb.off_mode();
                vTaskSuspend(lamp_effect_handle);
                break;
            default:
                break;

        }
        vTaskDelay(period/portTICK_RATE_MS);
    }
}

void lamp_effect_set(int mode,int period_m)
{
    period = period_m;
    x_mode = mode;
    vTaskResume(lamp_effect_handle);
}

int is_running_task()
{
    return task_run_flag;
}

int lamp_effect_init(LECB cb)
{
    x_mode = 0;
    if(is_running_task() == false)
    {
        x_cb = cb;
        if(IOTSDK_State() == STATE_OTA){
            xTaskCreate(_lamp_effect_thread,"lamp_effect_thread",256,NULL,tskIDLE_PRIORITY + 3 + PRIORITIE_OFFSET,&lamp_effect_handle);
        }else{
            xTaskCreate(_lamp_effect_thread,"lamp_effect_thread",256,NULL,4,&lamp_effect_handle);
        }
        task_run_flag = true;
    }
    return 0;
}

int lamp_effect_exit()
{
    task_run_flag = false;
    x_mode = 0;
    vTaskDelete(lamp_effect_handle);
    return 0;
}
