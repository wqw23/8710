#ifndef DRIVER_CONFIG_H
#define DRIVER_CONFIG_H

#include "FreeRTOS.h"
#include "log.h"
#include "semphr.h"
#include "cmsis_os.h"


/*重新定义数据类型名称，driver相关文件中使用*/

#define DRIVER_DEBUG

#define DRIVER_MALLOC(size)   malloc(size)
#define DRIVER_FREE(ptr)      free(ptr)

#define DRIVER_PRINTF(fmt, arg...)  log_debug0(fmt, ##arg)

#ifdef DRIVER_DEBUG
#define DRIVER_DEBUG_PRINTF(fmt, arg...)    DRIVER_PRINTF("<%s:%d>:"fmt, __FUNCTION__, __LINE__,  ##arg)
#else
#define DRIVER_DEBUG_PRINTF(fmt, arg...)
#endif

/*dirver中用到了下述函数，需要根据不同的平台进行定义*/
#ifndef DRIVER_MALLOC
#error "DRIVER_MALLOC is undefined, like:#define DRIVER_MALLOC(size)  malloc(size)"
#endif

#ifndef DRIVER_FREE
#error "DRIVER_FREE is undefined, like:#define DRIVER_FREE(prt)  free(ptr)"
#endif

#ifndef DRIVER_PRINTF
#error "DRIVER_PRINTF is undefined, like:#define DRIVER_PRINTF(fmt, arg...) printf(fmt, ##arg)"
#endif
#endif
