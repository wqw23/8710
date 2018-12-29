#ifndef _HAL_KEY_H_
#define _HAL_KEY_H_

#include "hal_event.h"
#include "keyboard.h"

typedef enum {
    KEY_PRESS5S=3
}KEY_EVENT_TYPE;

void HALKey_Init(DeviceEventCallback cb,struct key_gpio_info key_init_info);

#endif