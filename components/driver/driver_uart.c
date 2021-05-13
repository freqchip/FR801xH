/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

/*
 * INCLUDES
 */
#include <stdint.h>
#include "driver_plf.h"
#include "driver_uart.h"

#define UART_CLK        14745600
#define UART_FIFO_TRIGGER (FCR_RX_TRIGGER_00|FCR_TX_TRIGGER_10)


__attribute__((weak)) __attribute__((section("ram_code"))) void uart1_isr_ram(void)
{
    uint8_t int_id;
    volatile uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART1_BASE;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        c = uart_reg->u1.data;
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

__attribute__((weak)) __attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        c = uart_reg->u1.data;
        uart_putc_noint(UART0,c);
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

void uart_init1(uint32_t uart_addr, uart_param_t param)
{
    int uart_baud_divisor;
    volatile struct uart_reg_t * const uart_reg = (volatile struct uart_reg_t *)uart_addr;

    /* wait for tx fifo is empty */
    while(!(uart_reg->lsr & 0x40));

    volatile uint32_t misc_data;
    REG_PL_WR(&(uart_reg->u2.ier),0);
    REG_PL_WR(&(uart_reg->u3.fcr),0x06);
    REG_PL_WR(&(uart_reg->lcr),0);
    misc_data = REG_PL_RD(&(uart_reg->lsr));
    misc_data = REG_PL_RD(&(uart_reg->mcr));

    uart_baud_divisor = UART_CLK/(16*param.baud_rate); 
    
    while(!(uart_reg->lsr & 0x40));

    /* baud rate */
    uart_reg->lcr.divisor_access = 1;
    uart_reg->u1.dll.data = uart_baud_divisor & 0xff;
    uart_reg->u2.dlm.data = (uart_baud_divisor>>8) & 0xff;
    uart_reg->lcr.divisor_access = 0;

    /*word len*/
    uart_reg->lcr.word_len = param.data_bit_num-5;
    if(param.pari == 0)
    {
        uart_reg->lcr.parity_enable = 0;
        uart_reg->lcr.even_enable = 0;
    }
    else
    {
        uart_reg->lcr.parity_enable = 1;
        if(param.pari == 1)
            uart_reg->lcr.even_enable = 1;
        else
            uart_reg->lcr.even_enable = 0;
    }

    uart_reg->lcr.stop = param.stop_bit;

    /*fifo*/
    uart_reg->u3.fcr.data = UART_FIFO_TRIGGER | FCR_FIFO_ENABLE;

    /*auto flow control*/
    uart_reg->mcr.afe = 0;

    //flush rx fifo.
    volatile uint8_t data;
    while (uart_reg->lsr & 0x01)    //UART_RX_FIFO_AVAILABLE
    {
        data = uart_reg->u1.data;
    }

    /*enable recv and line status interrupt*/
    uart_reg->u2.ier.erdi = 1;
    uart_reg->u2.ier.erlsi = 1;
}





