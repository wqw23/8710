//Standard head file

//RTK8710 head file
#include "device.h"
#include "serial_api.h"
//IOT SDK head
#include "main.h"
#include "datatype.h"
#include "log.h"
//Adapter head file
#include "led.h"
#include "uart_driver.h"

serial_t   sobj0;
UINT8 fifo_recive[BUF_LEN] = {0};

void uart_recv_string_done(uint32_t id)
{
    serial_t    *sobj = (void*)id;
    enable_coulomb_stream();
}

void uart_init(UINT8 uart_tx,UINT8 uart_rx,UINT32 baud_rate,UINT8 bit_len,UINT8 parity,UINT8 stop_bit)
{
    serial_init(&sobj0,uart_tx,uart_rx);
    serial_baud(&sobj0,baud_rate);
    serial_format(&sobj0,bit_len, parity,stop_bit);
    serial_recv_comp_handler(&sobj0, (void*)uart_recv_string_done, (UINT32) &sobj0);
}

