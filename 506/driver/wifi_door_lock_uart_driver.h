#ifndef WIFI_DOOR_LOCK_UART_DRIVER_H
#define WIFI_DOOR_LOCK_UART_DRIVER_H

#include "device.h"
#include "serial_api.h"
#include "datatype.h"
#include "message_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UART_WordLength_5b = 0x5,
    UART_WordLength_6b = 0x6,
    UART_WordLength_7b = 0x7,
    UART_WordLength_8b = 0x8
} UART_WordLength;

typedef enum {
    USART_StopBits_1   = 0x1,
    USART_StopBits_2   = 0x2,
} UART_StopBits;

typedef enum {
    USART_Parity_None = 0x0,
    USART_Parity_Odd = 0x1,
    USART_Parity_Even = 0x2
} UART_ParityMode;

typedef enum {
    BIT_RATE_300     = 300,
    BIT_RATE_600     = 600,
    BIT_RATE_1200    = 1200,
    BIT_RATE_2400    = 2400,
    BIT_RATE_4800    = 4800,
    BIT_RATE_9600    = 9600,
    BIT_RATE_19200   = 19200,
    BIT_RATE_38400   = 38400,
    BIT_RATE_57600   = 57600,
    BIT_RATE_74880   = 74880,
    BIT_RATE_115200  = 115200,
    BIT_RATE_230400  = 230400,
    BIT_RATE_460800  = 460800,
    BIT_RATE_921600  = 921600,
    BIT_RATE_1843200 = 1843200,
    BIT_RATE_3686400 = 3686400,
} UART_BautRate; //you can add any rate you need in this range

typedef enum {
    USART_HardwareFlowControl_None    = 0x0,
    USART_HardwareFlowControl_RTS     = 0x1,
    USART_HardwareFlowControl_CTS     = 0x2,
    USART_HardwareFlowControl_CTS_RTS = 0x3
} UART_HwFlowCtrl;

#define UART_SUCCESS 1  /*串口初始化成功*/
#define UART_FAILURE 0  /*串口初始化失败*/

#define UART_DATA_LEN_BYTE_LOCATION   3/*表示第几个字节代表数据的长度*/

typedef UINT8 (* uart_recv_callback)(void *arg,UINT32 len);
void Uart_Send(const UINT8* buff, UINT16 len);
UINT8 Uart_Init(uart_recv_callback cb,UINT8 uart_tx,UINT8 uart_rx,UINT32 baud_rate,UINT8 bit_len,UINT8 parity,UINT8 stop_bit);

#ifdef __cplusplus
}

#endif
#endif

