
#ifndef __IOT_PLATFORM_LOG_H
#define __IOT_PLATFORM_LOG_H

#define os_printf printf
char *rtos_time_print(void);
#define LOG_TIME()  rtos_time_print()

#endif 





