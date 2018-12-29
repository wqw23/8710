
#ifndef __LED_H__
#define __LED_H__

#include "datatype.h"
#include "gpio_api.h"
#include "product_config.h"

struct led_gpio_info
{
    gpio_t gpio_led_wifi;             //&gpio_wifi_led
    INT32 gpio_pin_num_wifi;          //GPIO_LED_WIFI
    INT32 gpio_pin_directionc_wifi;   //PIN_OUTPUT
    INT32 gpio_pin_mode_wifi;         //PullNone
    INT32 led_wifi_on;
    INT32 led_wifi_off;
#if ((defined POWER_CONTROL_INDIVIDUAL) && (POWER_CONTROL_INDIVIDUAL == 1))
    gpio_t gpio_led_power;          //&gpio_power_led
    INT32 gpio_pin_num_power;       //GPIO_LED_POWER
    INT32 gpio_pin_directionc_power;//PIN_OUTPUT
    INT32 gpio_pin_mode_power;      //PullNone
    INT32 led_power_on;
    INT32 led_power_off;
#endif
};

void led_init(struct led_gpio_info led_init_info);
void led_wifi_trigger( UINT8 isEnable );
void led_power_trigger( UINT8 isEnable );
#endif

