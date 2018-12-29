#ifndef __JDQ_H__
#define __JDQ_H__

#include "gpio_api.h"
#include "datatype.h"

struct relay_gpio_info
{
    gpio_t gpio_relay;         //&gpio_relay
    INT32 relay_pin_num;       //GPIO_JDQ_NUM
    INT32 relay_pin_directionc;//PIN_OUTPUT
    INT32 relay_pin_mode;      //PullNone
    INT32 jdq_on;
    INT32 jdq_off;
};

void relay_trigger( UINT8 enable );
void relay_init(struct relay_gpio_info relay_init_info);
#endif

