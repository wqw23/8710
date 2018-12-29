
/******************************************************************************
**文 件 名: uart_driver.c
**作     者: wqw
**生成日期: 2018年11月11日
**功能描述: 串口驱动文件，提供初始化和收发功能
******************************************************************************/
//Standard head file
#include <string.h>
#include <stdio.h>
//RTK8710 head file
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "task.h"
#include "queue.h"
#include "device.h"
#include "serial_api.h"
//IOT SDK head
#include "main.h"
#include "datatype.h"
#include "log.h"
//Adapter head file
#include "led.h"
#include "wifi_door_lock_uart_driver.h"
#include "driver_config.h"
#include "message_queue.h"

serial_t   sobj0;
#define RECV_BUF_SIZE         (128)    /*接收缓冲区大小*/

uart_recv_callback  uart0_recv_cb=NULL;
static xSemaphoreHandle s_Uart_Send_Lock;   /*串口发送时用的锁*/

/*****************************************************************************
**函 数 名: void uart_irq(uint32_t id, SerialIrq event)
**输入参数: UINT8 *buf:接收的数据缓冲区
           UINT32 size:接收的数据长度
**输出参数: 无
**返 回 值: static
**功能描述: 串口中断接收数据
**作     者: wqw
*****************************************************************************/
UINT8 data_buf[RECV_BUF_SIZE]={0};
void uart_irq(uint32_t id, SerialIrq event)
{
    serial_t *sobj = (void*)id;
    static UINT8 buf_idx=0;
    static UINT8 buf_len=0;

    if(event == RxIrq)
    {
        data_buf[buf_idx++]=serial_getc(sobj);
        if(UART_DATA_LEN_BYTE_LOCATION==(buf_idx-1)){
            buf_len=data_buf[UART_DATA_LEN_BYTE_LOCATION];
        }
        if(buf_idx==buf_len && buf_len!=0){
            uart0_recv_cb(data_buf,buf_len);
            memset(data_buf, 0, sizeof(data_buf));
            buf_idx=0;
            buf_len=0;
        }
    }
}

/*****************************************************************************
**函 数 名: Uart_Send
**输入参数: const char* buff:需要发送的数据
             size_t len :发送的数据长度
**输出参数: 无
**返 回 值:
**功能描述: 使用uart发送。调用库文件发送时，请选择阻塞式发送函数。
**作     者: wqw
*****************************************************************************/
void Uart_Send(const UINT8* buff, UINT16 len)
{
    xSemaphoreTake(s_Uart_Send_Lock, portMAX_DELAY);
    DRIVER_PRINTF("serial_send_stream=crc=0x%x\n",buff[len-1]);
    serial_send_stream(&sobj0,(s8 *)buff, len);
    xSemaphoreGive(s_Uart_Send_Lock);
}

/*****************************************************************************
**函 数 名: Uart_Init
**输入参数: UINT32 baudrate:串口波特率(9600bps)
**输出参数: 无
**返 回 值:
**功能描述: 初始化与MCU通信串口，波特率为9600bps。
**作     者: wqw
*****************************************************************************/
UINT8 Uart_Init(uart_recv_callback cb,UINT8 uart_tx,UINT8 uart_rx,UINT32 baud_rate,UINT8 bit_len,UINT8 parity,UINT8 stop_bit)
{
    DRIVER_PRINTF("enter uart_init-----------------\n");

    serial_init(&sobj0,uart_tx,uart_rx);
    serial_baud(&sobj0,baud_rate);
    serial_format(&sobj0, bit_len, parity, stop_bit);
    uart0_recv_cb = cb;

    serial_irq_handler(&sobj0, uart_irq, (uint32_t)&sobj0);
    serial_irq_set(&sobj0, RxIrq, UART_SUCCESS);

    s_Uart_Send_Lock = xSemaphoreCreateMutex();
    if(NULL == s_Uart_Send_Lock)
    {
        goto UART_ERR;
    }

        return UART_SUCCESS;
    UART_ERR:
        return UART_FAILURE;
}
