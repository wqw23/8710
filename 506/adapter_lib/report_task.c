//Standard head file

//RTK8710 head file
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
//IOT SDK head
#include "log.h"
#include "security_func.h"
//Adapter head file
#include "report_task.h"

static unsigned int *g_rinfo = NULL;
static unsigned int *g_rrinfo = NULL;
static int g_count = 0;
static int g_rcount = 0;
static unsigned char g_async_flag = 0;

static xSemaphoreHandle g_mutex;
static xTaskHandle arHandle;

void _async_report_work_thread(void *arg)
{
    while(1) {
        int rcount = 0;

        xSemaphoreTake(g_mutex, portMAX_DELAY);

        rcount = g_rcount;
        memset(g_rrinfo, 0, g_count * sizeof(unsigned int));
        iots_memcpy(g_rrinfo, g_rinfo, rcount * sizeof(unsigned int));

        //reset global variable
        memset(g_rinfo, 0, sizeof(unsigned int) * g_count);
        g_rcount = 0;

        xSemaphoreGive(g_mutex);

        IOTDM_AttributesChanged(g_rrinfo, rcount);

        vTaskDelay(3000/portTICK_RATE_MS);
    }
}

int report_init(int count)
{
    if (g_async_flag) {//task already run
        log_error("%s report task already init!\n", __FUNCTION__);
        return 0;
    }

    g_rinfo = (unsigned int *) malloc(sizeof(unsigned int) * count);
    if (g_rinfo == NULL) {
        return -1;
    }

    g_rrinfo = (unsigned int *) malloc(sizeof(unsigned int) * count);
    if (g_rrinfo == NULL) {
        free(g_rinfo);
        g_rinfo = NULL;
        return -1;
    }

    memset(g_rinfo, 0, sizeof(unsigned int) * count);
    g_count = count;
    g_mutex = xSemaphoreCreateMutex();
    xTaskCreate(_async_report_work_thread, "async_report", 512, NULL, 4, &arHandle);

    //set task is already init flag;
    g_async_flag = 1;

    log_debug2("%s report task init success!\n", __FUNCTION__);
    return 0;
}

int report_deinit(void)
{
    if (g_async_flag == 0) {//task not init
        log_error("%s report task not init!\n", __FUNCTION__);
        return 0;
    }

    xSemaphoreTake(g_mutex, portMAX_DELAY);

    vTaskDelete(arHandle);
    free(g_rinfo);
    free(g_rrinfo);
    g_rinfo = g_rrinfo = NULL;

    xSemaphoreGive(g_mutex);
    vSemaphoreDelete(g_mutex);

    g_async_flag = 0;

    log_debug2("%s report task deinit success!\n", __FUNCTION__);
    return 0;
}

void async_report_attr(unsigned int attr_id, REPORT_FLASH_FLAG flag)
{
    int i = 0;

    if (flag) {
        flash_update();
    }

    if (g_async_flag) {
        xSemaphoreTake(g_mutex, portMAX_DELAY);

        if (g_rinfo == NULL) {
            goto OUT;
        }

        for (i = 0; i < g_rcount; i++) {
            if (g_rinfo[i] == attr_id) {
                goto OUT;
            }
        }

        if (i < g_count) {
            g_rinfo[i] = attr_id;
            g_rcount++;
        } else {
            log_error("%s async report count over set max value!\n", __FUNCTION__);
        }
OUT:
        xSemaphoreGive(g_mutex);
    }
}

void sync_report_attr(unsigned int attr_id, REPORT_FLASH_FLAG flag)
{
    if (flag) {
        flash_update();
    }

    if (g_async_flag) {
        IOTDM_AttributesChanged(&attr_id, 1);
    }
}
