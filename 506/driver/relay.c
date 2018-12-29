
#define  MODULE_TAG "JDQ"
//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "gpio_api.h"
//IOT SDK head
#include "datatype.h"
#include "log.h"
//Adapter head file
#include "relay.h"
#include "product_config.h"

static struct relay_gpio_info  g_relay_gpio_info;

void relay_init(struct relay_gpio_info relay_init_info){
    log_debug0("relay driver init \n");
    g_relay_gpio_info = relay_init_info;
    gpio_init(&g_relay_gpio_info.gpio_relay, g_relay_gpio_info.relay_pin_num);
    gpio_dir(&g_relay_gpio_info.gpio_relay, g_relay_gpio_info.relay_pin_directionc);    // Direction: Output
    gpio_mode(&g_relay_gpio_info.gpio_relay, g_relay_gpio_info.relay_pin_mode);         // No pull
    //gpio_write(&g_relay_gpio_info.gpio_relay, g_relay_gpio_info.jdq_off);
}

void relay_trigger( UINT8 enable ){
    if(enable){
        gpio_write(&g_relay_gpio_info.gpio_relay, g_relay_gpio_info.jdq_on);
        log_debug0("Turn on JDQ \n");
    }else{
        gpio_write(&g_relay_gpio_info.gpio_relay, g_relay_gpio_info.jdq_off);
        log_debug0("Turn off JDQ \n");
    }
}

