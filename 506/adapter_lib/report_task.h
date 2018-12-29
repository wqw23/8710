#ifndef _REPORT_TASK_H_
#define _REPORT_TASK_H_

typedef enum {
    ASYNC_NOT_UPDATE_FLASH,
    ASYNC_UPDATE_FLASH
}REPORT_FLASH_FLAG;

/***************************************
 * @brief report_init
 *        async report task init.
 * @param count async report use which include max count async report support
 * @return 0 success, -1 failed
 ***************************************/
int report_init(int count);

/***************************************
 * @brief report_deinit
 *        async report task deinit.
 * @return 0 success, -1 failed
 ***************************************/
int report_deinit(void);

/***************************************
 * @brief async_report_attr
 *        set async report attribute id.
 * @param attr_id device attribute id which need report to cloud
 * @param flag write flash flag @REPORT_FLASH_FLAG
 ***************************************/
void async_report_attr(unsigned int attr_id, REPORT_FLASH_FLAG flag);

/***************************************
 * @brief sync_report_attr
 *        set sync report attribute id.
 * @param attr_id device attribute id which need report to cloud
 * @param flag write flash flag @REPORT_FLASH_FLAG
 ***************************************/
void sync_report_attr(unsigned int attr_id, REPORT_FLASH_FLAG flag);
#endif
