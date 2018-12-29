#ifndef __LAMP_EFFECT_H_
#define __LAMP_EFFECT_H_

#define LAMP_EFFECT_OFF_MODE        0
#define LAMP_EFFECT_OTA_MODE        1
#define LAMP_EFFECT_SOFTAP_MODE     2
#define LAMP_EFFECT_STATION_MODE    4

typedef void (*lamp_effect_cb)();

typedef struct lamp_effect_callback
{
    lamp_effect_cb off_mode;
    lamp_effect_cb ota_mode;
    lamp_effect_cb softap_mode;
    lamp_effect_cb station_mode;
}LECB;

void lamp_effect_set(int mode,int period_m);
int lamp_effect_init(LECB cb);
int lamp_effect_exit();
#endif
