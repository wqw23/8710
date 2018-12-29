//Standard head file
#include <string.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include "timers.h"
//IOT SDK head
#include "datatype.h"
#include "log.h"
//Adapter head file
#include "hal_event.h"
#include "keyboard.h"
#include "hal_key.h"


#define KEY5S_TIMER (5 * 1000)

static DeviceEventCallback f_event_cb;

static UINT8 m_key5s_flag = 0;
static TimerHandle_t m_key5s_timer;

void _k5sTimer(xTimerHandle xTimer, void *callback_arg)
{
    log_debug0("press key is 5s !\n");
    m_key5s_flag = 1;
}

UINT8 _isKeyPress5s(void)
{
    UINT8 flag = m_key5s_flag;

    m_key5s_flag = 0;

    log_error("key press 5s flag %d\n", flag);
    return flag;
}

void _k5sTimerStart(void)
{
    m_key5s_flag = 0;

    if (m_key5s_timer) {
        xTimerResetFromISR(m_key5s_timer, 0);
    }

}

void _k5sTimerStop(void)
{
    if (m_key5s_timer) {
        xTimerStopFromISR(m_key5s_timer, 0);
    }

}

void _keyAction(KEYACTION key_action)
{
    HALEventInfo info;

    memset(&info, 0, sizeof(info));
    if (_isKeyPress5s()) {
        info.sub_type = KEY_PRESS5S;
    } else {
        info.sub_type = key_action;
    }
    f_event_cb("key", &info);
}

void _eventCallback(KEYACTION key_action)
{
    if(key_action == KEY_UP) {
        _k5sTimerStop();
        _keyAction(key_action);
    } else if(key_action == KEY_DOWN) {
        _k5sTimerStart();
        _keyAction(key_action);
    }
}

void HALKey_Init(DeviceEventCallback cb,struct key_gpio_info key_init_info)
{
    f_event_cb = cb;
    m_key5s_timer = xTimerCreate(
            "Key5sTimer", KEY5S_TIMER / portTICK_RATE_MS,
            pdFALSE, (void *)NULL, _k5sTimer);
    key_init(_eventCallback,key_init_info);
}

