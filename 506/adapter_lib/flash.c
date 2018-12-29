//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
//IOT SDK head
#include "datatype.h"
#include "security_func.h"
#include "log.h"
//Adapter head file
#include "flash.h"

#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_COLORTEMP  0

static struct FlashData *g_flash_data;
static int g_user_data_size;

struct TimerDelay *flash_export_timers(void)
{
    if (g_flash_data) {
        return g_flash_data->timers;
    } else {
        return NULL;
    }
}

void *flash_export_user_data(void)
{
    if (g_flash_data == NULL || g_user_data_size == 0) {
        return NULL;
    } else {
        return g_flash_data->user_data;
    }
}

int flash_get_version(void)
{
    if (g_flash_data) {
        return g_flash_data->version;
    } else {
        return E_FAILED;
    }
}

void flash_set_version(int version)
{
    if (g_flash_data) {
        g_flash_data->version = version;
    }
}

int flash_update(void)
{
    if (g_flash_data) {
        return IOTConf_Save((void *)g_flash_data, sizeof(struct FlashData) + g_user_data_size);
    } else {
        return E_FAILED;
    }
}

int flash_load(void)
{
    int ret;

    if (g_flash_data) {
        ret = IOTConf_Load(g_flash_data, sizeof(struct FlashData) + g_user_data_size);
        if(E_INVALID == ret){
            log_debug2("first read, no data!\n");
            ret = E_NO_DATA;
        } else if(E_FAILED == ret){
            log_debug2("load failed! error num %d\n", ret);
            ret = E_FAILED;
        } else if (ret == E_PARAM_ERROR) {
            log_debug2("load failed! error num %d\n", ret);
            ret = E_PARAM_ERROR;
        } else {
            log_debug2("Flash data version %u\n", g_flash_data->version);
            ret = g_flash_data->version;
        }

        return ret;
    } else {
        return E_NO_MEMORY;
    }
}

int flash_reset(void *dev_default_data)
{
    if (g_flash_data) {
        memset(g_flash_data->timers, 0, sizeof(g_flash_data->timers));

        if (g_user_data_size) {
            memset(g_flash_data->user_data, 0, g_user_data_size);
            if (dev_default_data != NULL) {
                iots_memcpy(g_flash_data->user_data, dev_default_data, g_user_data_size);
            }
        }

        return flash_update();
    } else {
        return E_FAILED;
    }
}

int flash_init(int data_size)
{
    g_user_data_size = 0;

    if (data_size >= 0) {
        g_flash_data = (struct FlashData *) malloc(sizeof(struct FlashData) + data_size);
        if (g_flash_data == NULL) {
            log_error("flash_init malloc error!\n");
            return E_FAILED;
        }

        g_user_data_size = data_size;
        memset(g_flash_data, 0, sizeof(struct FlashData) + g_user_data_size);
        return E_SUCCESS;
    } else {
        log_error("flash_init error, size %d\n", data_size);
        return E_FAILED;
    }
}

