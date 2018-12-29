#ifndef _FLASH_H_
#define _FLASH_H_

#include "schema_task.h"

//If you want to change struct size, you MUST to update @CURRENT_FLASH_VERSION at flash.h head file.
struct FlashData
{
    UINT32 version;
    struct TimerDelay timers[14];
    UINT8 user_data[0];
};

struct TimerDelay *flash_export_timers(void);
void *flash_export_user_data(void);
void flash_set_version(int version);

int flash_update(void);
int flash_load(void);
int flash_reset(void *dev_default_data);
int flash_init(int data_size);
#endif

