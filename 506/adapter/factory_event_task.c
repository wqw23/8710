//Standard head file

//RTK8710 head file
#include "timer_api.h"
#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
#include "gpio_irq_ex_api.h"
//IOT SDK head
#include "log.h"
//Adapter head file
#include "led.h"
#include "relay.h"
#include "keyboard.h"
#include "lamp_effect.h"
#include "product_config.h"
#include "adapter.h"


extern uint8 pcba_factory_key_status;
extern int led_relay_onoff;  //记录继电器灯的高低电平.

void FactoryKeyEvent(int* m_dev_status)
{
    *m_dev_status = *m_dev_status ? 0 : 1;
    log_debug0("FactoryKeyEvent=====m_dev_status=%d,factory_led_relay_onoff=%d",*m_dev_status,led_relay_onoff);
    if(*m_dev_status==0){//actions up
        led_relay_onoff = led_relay_onoff ? 0 : 1;
        if(PcbaIsKeyStatusHigh())
            pcba_factory_key_status=true;
        else
            SendKeyStatusToUDP(*m_dev_status);

        log_debug0("FactoryKeyEvent plug key action up \n");

        relay_trigger(led_relay_onoff);
        led_power_trigger(led_relay_onoff);
    }else{//actions down
        if(PcbaIsKeyStatusHigh())
            pcba_factory_key_status=true;
        else
            SendKeyStatusToUDP(*m_dev_status);
        log_debug0("FactoryKeyEvent plug key action down \n");
    }

}

