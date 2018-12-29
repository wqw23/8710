//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include "timers.h"
#include "time.h"
//IOT SDK head
#include "log.h"
#include "security_func.h"
//Adapter head file
#include "schema_task.h"

#define EAST_8ZONE_OFFSET_MINUTES      (8 * 60)
#define EAST_8ZONE_OFFSET_SECONDS      (8 * 60 * 60)

//TimerDelay status define
#define TIMER_STATUS_ENABLE     0x01
#define TIMER_STATUS_ON         0x02

#define TASK_ALLOW_DELAY_RETURN_TIME  (5)

#define TASK_WORK_FOR_BOTH            (0)
#define TASK_WORK_ONLY_FOR_DELAY      (1)

static struct TimerDelay *g_timer_info;
schema_task_cb func_schema_task_cb = NULL;
static xTaskHandle stHandle;

//calculate seconds for next delay function
int __get_delay_function_delay_sec(time_t zone_time)
{
    int i;
    int s_sec;//single delay time
    int min_sec;

    min_sec = 0;

    for (i = 0; i < 2; i++) {
        s_sec = 0;
        if (g_timer_info[i].status & TIMER_STATUS_ENABLE) {
            s_sec = g_timer_info[i].set_utc + g_timer_info[i].timer * 60 - zone_time;
            if (min_sec > 0) {
                min_sec = min_sec <= s_sec ? min_sec : s_sec;
            } else {
                min_sec = s_sec;
            }
        }
    }

    return min_sec;
}

//calibrate delay time, return seconds which be away from next minute.
int __calibrate_task_periods_run_sec(void)
{
    time_t cur_t, next_t;

    cur_t = IOTSys_time(NULL);

    //Get next minute utc seconds value.
    next_t = (cur_t / 60 + 1) * 60;

    return (next_t - cur_t);
}

int _calculate_delay_time(time_t zone_time, int *work_flag)
{
    int d_time;
    int t_time;

    //Get delay work delay time for task.
    d_time = __get_delay_function_delay_sec(zone_time);

    //Get seconds for task next periods run time.
    t_time = __calibrate_task_periods_run_sec();

    //if delay work delay time more than zero and less than periods work delay time.
    if ((d_time > 0) && (d_time < t_time)) {
        *work_flag = TASK_WORK_ONLY_FOR_DELAY;
        return d_time;
    } else {
        *work_flag = TASK_WORK_FOR_BOTH;
        return t_time;
    }
}

int _check_delay_work(time_t zone_time, int *action)
{
    int perform_flag;
    int i;
    int s_sec;//single delay time

    perform_flag = 0;

    for (i = 0; i < 2; i++) {
        s_sec = 0;
        if (g_timer_info[i].status & TIMER_STATUS_ENABLE) {
            s_sec = g_timer_info[i].set_utc + g_timer_info[i].timer * 60 - zone_time;
            os_printf("[delay work] config sec %d, zone minutes %d, %d, action %s\n",
            g_timer_info[i].set_utc + g_timer_info[i].timer * 60,
            zone_time, s_sec, g_timer_info[i].status & TIMER_STATUS_ON ? "On" : "Off");
            if (s_sec == 0) {//same time
                *action = g_timer_info[i].status & TIMER_STATUS_ON ? 1 : 0;
                memset(&g_timer_info[i], 0, sizeof(g_timer_info[i]));
                perform_flag = 1;
            } else if ((s_sec < 0) && (abs(s_sec) < TASK_ALLOW_DELAY_RETURN_TIME)) {
                //Delay return which because of system reason,
                //but less than 5's, then it's same as same time.
                *action = g_timer_info[i].status & TIMER_STATUS_ON ? 1 : 0;
                memset(&g_timer_info[i], 0, sizeof(g_timer_info[i]));
                perform_flag = 1;
            } else if ((s_sec < 0) && (abs(s_sec) > TASK_ALLOW_DELAY_RETURN_TIME)) {
                //invalid delay info
                memset(&g_timer_info[i], 0, sizeof(g_timer_info[i]));
            }
        }
    }

    return perform_flag;
}
int _check_timer_work(int wday, int cur_time, int *action)
{
    int i;
    int ttd_time;
    int perform_flag;

    perform_flag = 0;

    for (i = 2; i < 14; i++) {
        ttd_time = 0;
        if ((g_timer_info[i].weeks & (0x1 << wday))
            && (g_timer_info[i].status & TIMER_STATUS_ENABLE)) {
            ttd_time = cur_time - g_timer_info[i].timer * 60;
            os_printf("[timer work] current minutes %d, config minutes %d, action %s\n",
            cur_time, g_timer_info[i].timer * 60,
            g_timer_info[i].status & TIMER_STATUS_ON ? "On" : "Off");
            if ((ttd_time >= 0) && (ttd_time < TASK_ALLOW_DELAY_RETURN_TIME)) {
                //Delay return which because of system reason,
                //but less than 5's, then it's same as same time.
                *action = g_timer_info[i].status & TIMER_STATUS_ON ? 1 : 0;
                perform_flag = 1;
            }
        }
    }

    return perform_flag;
}

void _get_cur_time_info(time_t *zone_time, int *wday, int *cur_sec)
{
    time_t t;
    struct tm *p;

    t = IOTSys_time(NULL);

    *zone_time = t - EAST_8ZONE_OFFSET_SECONDS;

    p = (struct tm *)IOTSys_localtime(NULL);
    *wday = p->tm_wday;
    *cur_sec = p->tm_hour * 60 * 60 + p->tm_min * 60 + p->tm_sec;
}

void _schema_task_thread(void *arg)
{
    time_t zone_time;
    int current_time =0;
    int wday = 0;
    int td_time;//task delay time
    int perform_flag = 0;
    int work_flag;
    int action = 0;

    work_flag = TASK_WORK_FOR_BOTH;

    while (1) {
        td_time = 0;
        perform_flag = 0;

        //Get current time info first.
        _get_cur_time_info(&zone_time, &wday, &current_time);

        log_debug2("timer task run, current_time: %d second\n",current_time);
        log_debug2("current_weeks: %d \n",wday);

        //check delay function
        perform_flag = _check_delay_work(zone_time, &action);

        //check timer function, if work_flag set TASK_WORK_FOR_BOTH.
        if (TASK_WORK_FOR_BOTH == work_flag) {
            perform_flag = _check_timer_work(wday, current_time, &action);
        }

        if (perform_flag) {
            func_schema_task_cb((void *)&action);
        }

        //Recalculate task delay seconds for next work.
        td_time = _calculate_delay_time(zone_time, &work_flag);
        os_printf("delay seconds %d, and task next-time work flag %d\n", td_time, work_flag);

        vTaskDelay(1000 / portTICK_RATE_MS * td_time);
    }
}

void schema_task_init(struct TimerDelay * timer_info, schema_task_cb cb)
{
    g_timer_info = timer_info;
    func_schema_task_cb = cb;
}

void schema_task_resume(void)
{
    char *name = "schema_task";

    log_debug2("Create schema task in second\n");
    xTaskCreate(_schema_task_thread, (const signed char * const)name, 512, NULL, 4, &stHandle);
}

void schema_task_suspend(void)
{
    vTaskDelete(stHandle);
}

void schema_task_set_delay_info(char *delay_info)
{
    char *item;
    char *str;
    char *saveStr;
    int i = 0;
    time_t utc = 0;
    int timer = 0, enable = 0, action = 0;

    for (i = 0, str = delay_info; ; i++, str = NULL) {
        item = strtok_r(str, ",", &saveStr);
        if (item == NULL) {
            break;
        }
        iots_sscanf(item,"%ld:%d:%d:%d", &utc, &timer, &enable, &action);
        g_timer_info[i].set_utc = utc;
        g_timer_info[i].timer = timer;
        g_timer_info[i].status = 0;
        g_timer_info[i].status |= enable ? TIMER_STATUS_ENABLE : 0;
        g_timer_info[i].status |= action ? TIMER_STATUS_ON : 0;
    }
}

void schema_task_get_delay_info(char *delay_info)
{
    int i = 0;

    for (i = 0; i < 2; i++) {
        iots_sprintf(delay_info + iots_strlen(delay_info), "%d:%d:%d:%d,",
                    g_timer_info[i].set_utc, g_timer_info[i].timer,
                    g_timer_info[i].status & TIMER_STATUS_ENABLE ? 1 : 0,
                    g_timer_info[i].status & TIMER_STATUS_ON ? 1 : 0);
    }

    delay_info[iots_strlen(delay_info) - 1] = '\0';
}

void schema_task_set_timer_info(char *timer_info)
{
    char *item;
    char *str;
    char *saveStr;
    int i = 0;
    int week = 0, timer = 0, enable = 0, action = 0;

    for (i = 0, str = timer_info; ; i++, str = NULL) {
        item = strtok_r(str, ",", &saveStr);
        if (item == NULL) {
            break;
        }
        iots_sscanf(item,"%d:%d:%d:%d", &week, &timer, &enable, &action);
        g_timer_info[i + 2].weeks = week;
        g_timer_info[i + 2].timer = timer;
        g_timer_info[i + 2].status = 0;
        g_timer_info[i + 2].status |= enable ? TIMER_STATUS_ENABLE : 0;
        g_timer_info[i + 2].status |= action ? TIMER_STATUS_ON : 0;
    }
}

void schema_task_get_timer_info(char *timer_info)
{
    int i = 0;

    for (i = 2; i < 14; i++) {
        iots_sprintf(timer_info + iots_strlen(timer_info), "%d:%d:%d:%d,",
                    g_timer_info[i].weeks, g_timer_info[i].timer,
                    g_timer_info[i].status & TIMER_STATUS_ENABLE ? 1 : 0,
                    g_timer_info[i].status & TIMER_STATUS_ON ? 1 : 0);
    }

    timer_info[iots_strlen(timer_info) - 1] = '\0';
}

