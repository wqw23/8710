
#define  MODULE_TAG "LED"
//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "device.h"
//#include "gpio_api.h"
#include "sys_api.h"    // for sys_jtag_off()
#include "main.h"
//IOT SDK head
#include "datatype.h"
#include "log.h"
//Adapter head file
#include "led.h"
#include "product_config.h"

static struct led_gpio_info g_reg_info_led;

void led_init(struct led_gpio_info led_init_info){
    log_debug0("Led Driver Init \n");
    g_reg_info_led = led_init_info;
    //jtag and led is same gpio,use led should close jtag first.
    sys_jtag_off();

    //Init BLUE LED control pin

    gpio_init(&g_reg_info_led.gpio_led_wifi, g_reg_info_led.gpio_pin_num_wifi);
    gpio_dir(&g_reg_info_led.gpio_led_wifi, g_reg_info_led.gpio_pin_directionc_wifi);    // Direction: Output
    gpio_mode(&g_reg_info_led.gpio_led_wifi, g_reg_info_led.gpio_pin_mode_wifi);     // No Pull
    led_wifi_trigger(false);

#if ((defined POWER_CONTROL_INDIVIDUAL) && (POWER_CONTROL_INDIVIDUAL == 1))
    //Init RED LED control pin
    gpio_init(&g_reg_info_led.gpio_led_power, g_reg_info_led.gpio_pin_num_power);
    gpio_dir(&g_reg_info_led.gpio_led_power, g_reg_info_led.gpio_pin_directionc_power);    // Direction: Output
    gpio_mode(&g_reg_info_led.gpio_led_power, g_reg_info_led.gpio_pin_mode_power);     // No Pull
    led_power_trigger(false);
#endif
}

void led_wifi_trigger( UINT8 isEnable ){
    if(isEnable){
        //log_debug0("Turn on BLUE Led Lamp \n");
        gpio_write(&g_reg_info_led.gpio_led_wifi, g_reg_info_led.led_wifi_on);
    }else{
        //log_debug0("Turn off BLUE Led Lamp \n");
        gpio_write(&g_reg_info_led.gpio_led_wifi, g_reg_info_led.led_wifi_off);
    }
}

void led_power_trigger( UINT8 isEnable ){
#if ((defined POWER_CONTROL_INDIVIDUAL) && (POWER_CONTROL_INDIVIDUAL == 1))
    if(isEnable){
        //log_debug0("Turn on RED Led Lamp \n");
        gpio_write(&g_reg_info_led.gpio_led_power, g_reg_info_led.led_power_on);
    }else{
        //log_debug0("Turn off RED Led Lamp \n");
        gpio_write(&g_reg_info_led.gpio_led_power,  g_reg_info_led.led_power_off);
    }

#endif
}

