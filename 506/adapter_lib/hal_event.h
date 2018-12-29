#ifndef _HAL_EVENT_H_
#define _HAL_EVENT_H_

typedef struct {
    int sub_type;
    void *data;
}HALEventInfo;

typedef void (*DeviceEventCallback)(char *event_name, HALEventInfo *data);
#endif
