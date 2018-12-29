
#define  MODULE_TAG "KEY"
//Standard head file

//RTK8710 head file
#include "device.h"
#include "gpio_api.h"   // mbed
#include "gpio_irq_api.h"   // mbed
#include "diag.h"
#include "gpio_irq_ex_api.h"
#include "timers.h"
#include "timer_api.h"
#include "main.h"
//IOT SDK head
#include "log.h"
//Adapter head file
#include "keyboard.h"
#include "product_config.h"

#define KEY_IGNORE_SHAKE_TIMER 10

int key_press_flag = false;

key_event_callback key_action_callback;
int key_irq_flag = 0;
static TimerHandle_t key_ignore_shake_timer;
static struct key_gpio_info g_key_gpio_info;

void gpio_key_irq_handler()
{
    if(key_irq_flag && gpio_read(&g_key_gpio_info.gpio_key))
    {
        //log_debug0("plug key action up! \n");
        if( !key_press_flag )
        {
            //log_debug0("ignore hardware key action up!\n");
            gpio_irq_enable(&g_key_gpio_info.gpio_key);
            return;
        }
        //log_debug0("plug key action up \n");
        key_press_flag = false;

        key_action_callback(KEY_UP);// IRQ_RISE

        //set trigger by IRQ_FALL
        gpio_irq_set(&g_key_gpio_info.gpio_key,KEY_DOWN , 1);//IRQ_FALL
        key_irq_flag = 0;
    }
    else if(!key_irq_flag && !gpio_read(&g_key_gpio_info.gpio_key))
    {
        //log_debug0("plug key action down \n");
        key_press_flag = true;

        key_action_callback(KEY_DOWN);

        //set trigger by IRQ_RISE
        gpio_irq_set(&g_key_gpio_info.gpio_key, KEY_UP, 1);
        key_irq_flag = 1;
    }
    gpio_irq_enable(&g_key_gpio_info.gpio_key);

    xTimerDelete(key_ignore_shake_timer,portMAX_DELAY);
}

void key_irq_function(uint32_t id, gpio_irq_event event)
{
    //log_debug0("%s==>%d\n", __FUNCTION__,event);

    gpio_irq_disable(&g_key_gpio_info.gpio_key);

    key_ignore_shake_timer = xTimerCreate( "KeyIgnoreShakeTimer",( KEY_IGNORE_SHAKE_TIMER/ portTICK_PERIOD_MS),pdFALSE,0,gpio_key_irq_handler);
    if(key_ignore_shake_timer == NULL){
        log_debug0("The KeyIgnoreShakeTimer timer was not created.\n");
    }else{
        if(xTimerStart(key_ignore_shake_timer,0) != pdPASS){
            log_debug0("The KeyIgnoreShakeTimer timer could not be set into the Active state.\n");
        }
    }
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void key_init(key_event_callback cb,struct key_gpio_info key_init_info)
{
    log_debug0("%s==>\n", __FUNCTION__);
    key_action_callback = cb;
    g_key_gpio_info = key_init_info;

    // Initial Push Button pin as interrupt source
    gpio_irq_init(&g_key_gpio_info.gpio_key, g_key_gpio_info.switch_pin_num, key_irq_function, NULL);
    gpio_irq_set(&g_key_gpio_info.gpio_key, KEY_DOWN, 1);   // Falling Edge Trigger
    gpio_irq_enable(&g_key_gpio_info.gpio_key);

}

