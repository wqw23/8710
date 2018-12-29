#ifndef __UDP_H__
#define __UDP_H__

#include "datatype.h"

#define ListenPort   8088
#define SEND_PORT    10878
#define SEND_IP "224.0.0.1"
#define FACTORY_WIFI_SSID "Lim_Fac-Mode"
#define PASSWD "Lim_Fac-Wirless"

void udp_recive_task(void *arg);
void udp_send_task(void *arg);
void factory_mode_auto_task(void *arg);

void SendKeyStatusToUDP(UINT8 key_status);
void factory_key_event_task(void *arg);
int isLedStatusHigh(int led_status);

#endif

