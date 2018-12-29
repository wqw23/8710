#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
#include "datatype.h"

#define KEY_IGNORE_SHAKE_TIMER (10*1000)

struct key_gpio_info
{
    gpio_irq_t gpio_key;
    INT32 switch_pin_num;
};

typedef enum {
    KEY_UP=1,
    KEY_DOWN
}KEYACTION;

typedef void (*key_event_callback) ( KEYACTION key_action);
void key_init(key_event_callback cb,struct key_gpio_info key_init_info);

#endif
