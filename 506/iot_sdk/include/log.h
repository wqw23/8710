#ifndef _LOG_HEADER_
#define _LOG_HEADER_

#include <assert.h>
#include "iotplatform_log.h"

#undef LOGTAG
#ifdef MODULE_TAG
#define LOGTAG  MODULE_TAG": "
#else
#define LOGTAG
#endif

extern int g_log_level;
#define log_debug0(fmt, arg...)  do{if(g_log_level >= 0)  {os_printf("[D]%s" LOGTAG fmt,LOG_TIME(), ##arg);}}while(0)
#define log_debug1(fmt, arg...)  do{if(g_log_level >= 1)  {os_printf("[D]%s" LOGTAG fmt,LOG_TIME(), ##arg);}}while(0)
#define log_infor(fmt, arg...)   do{if(g_log_level >= -1) {os_printf("[I]%s" LOGTAG fmt,LOG_TIME(), ##arg);}}while(0)
#define log_warning(fmt, arg...) do{if(g_log_level >= -2) {os_printf("[W]%s" LOGTAG "<%s(%d)> " fmt,LOG_TIME(), __FUNCTION__, __LINE__, ##arg);}}while(0)
#define log_error(fmt, arg...)   do{if(g_log_level >= -3) {os_printf("[E]%s" LOGTAG "<%s(%d)> " fmt,LOG_TIME(), __FUNCTION__, __LINE__, ##arg);}}while(0)
#ifndef BUILD_MODE_RELEASE
#define log_debug2(fmt, arg...)  do{if(g_log_level >= 2)  {os_printf("[D]%s" LOGTAG fmt,LOG_TIME(), ##arg);}}while(0)
#define log_assert(x)  do{if(!(x)){log_error("assert==============\n");} assert(x);}while(0)
#else
#define log_debug2(fmt, arg...)  do{}while(0)
#define log_assert(x)  do{}while(0)
#endif  //!BUILD_MODE_RELEASE

#endif  //!IOT_COMMON_HEADER

