#ifndef _SCHEMA_TASK_H_
#define _SCHEMA_TASK_H_

#include "datatype.h"
#include "time.h"

struct TimerDelay
{
    UINT8    weeks;
    UINT8    status;
    UINT16   timer;
    time_t   set_utc;
};


typedef void (*schema_task_cb)(void *arg);
void schema_task_init(struct TimerDelay * timer_info, schema_task_cb cb);
void schema_task_resume(void);
void schema_task_suspend(void);
void schema_task_set_delay_info(char *delay_info);
void schema_task_get_delay_info(char *delay_info);
void schema_task_set_timer_info(char *timer_info);
void schema_task_get_timer_info(char *timer_info);
#endif
