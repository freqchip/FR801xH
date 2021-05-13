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

#include "driver_ssp.h"
#include "driver_plf.h"
#include "driver_system.h"
#include "driver_gpio.h"

/*
 * MACROS 
 */
/*
 * TYPEDEFS
 */
typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

struct ssp_cr0
{
    u32 dss:4;  /* data size select : = DATASIZE - 1*/

    u32 frf:2;  /* frame format */

    u32 spo:1;  /* sclk polarity */
    u32 sph:1;  /* sclk phase */
    u32 scr:8;  /* serial clock rate */
    u32 unused:16;
};

struct ssp_cr1
{
    u32 rie:1;
    u32 tie:1;
    u32 rorie:1;

    u32 lbm:1;  /* loop back mode */
    u32 sse:1;  /* synchronous serial port enable*/

    u32 ms:1;   /* master mode or slave mode */
    u32 sod:1;  /* output disable in slave mode */

    u32 unused:25;
};

struct ssp_dr
{
    u32 data;
};

struct ssp_sr
{
    u32 tfe:1;  /* transmit fifo empty */
    u32 tnf:1;  /* transmit fifo not full */
    u32 rne:1;  /* receive fifo not empty */
    u32 rff:1;  /* receive fifo full */
    u32 bsy:1;  /* ssp busy flag */

    u32 unused:27;
};

struct ssp_cpsr
{
    u32 cpsdvsr:8;  /* clock prescale divisor 2-254 */

    u32 unused:24;
};

struct ssp_iir
{
    uint32_t ris:1;
    uint32_t tis:1;
    uint32_t roris:1;
    uint32_t reserved:29;
};

struct ssp
{
    struct ssp_cr0 ctrl0;
    struct ssp_cr1 ctrl1; /*is also error clear register*/
    struct ssp_dr data;
    struct ssp_sr status;
    struct ssp_cpsr clock_prescale;
    struct ssp_iir iir;
};

struct ssp_env_t
{
    void (*ssp_cs_ctrl)(uint8_t);
};

/*
 * LOCAL VARIABLES (���ر���)
 */
static struct ssp_env_t ssp_env;

/*********************************************************************
 * @fn      ssp_send_then_recv
 *
 * @brief   send data to peripheral, then receive data from peripheral,
 *          used for some cases as flash read operation.
 *
 * @param   tx_buffer   - data pointer to be send at first
 *          n_tx        - how many bytes to be send
 *          rx_buffer   - data pointer to store received data
 *          n_rx        - how many bytes to be received
 *
 * @return  None.
 */
void ssp_send_then_recv(uint8_t* tx_buffer, uint32_t n_tx, uint8_t* rx_buffer, uint32_t n_rx)
{
    volatile uint8_t temp;
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    uint32_t blocks;

    if(ssp_env.ssp_cs_ctrl)
    {
        ssp_env.ssp_cs_ctrl(SSP_CS_ENABLE);
    }

    //ssp->ctrl1.sse = 1;

    /* send data first */
    blocks = n_tx / SSP_FIFO_SIZE;
    while(blocks)
    {
        for(uint8_t i=0; i<SSP_FIFO_SIZE; i++)
        {
            ssp->data.data = *tx_buffer++;
        }
        while(ssp->status.rne == 0);
        while(ssp->status.rne)
        {
            temp = ssp->data.data;
        }
        blocks--;
        n_tx -= SSP_FIFO_SIZE;
    }

    while(n_tx)
    {
        ssp->data.data = *tx_buffer++;
        n_tx--;
    }
    while(ssp->status.rne == 0);
    while(ssp->status.rne)
    {
        temp = ssp->data.data;
    }

    /* receive data */
    blocks = n_rx / SSP_FIFO_SIZE;
    while(blocks)
    {
        for(uint8_t i=0; i<SSP_FIFO_SIZE; i++)
        {
            ssp->data.data = 0;
        }
        while(ssp->status.rne == 0);
        while(ssp->status.rne)
        {
            *rx_buffer++ = ssp->data.data;
        }
        blocks--;
        n_rx -= SSP_FIFO_SIZE;
    }

    while(n_rx)
    {
        ssp->data.data = 0;
        n_rx--;
    }
    while(ssp->status.rne == 0);
    while(ssp->status.rne)
    {
        *rx_buffer++ = ssp->data.data;
    }

    while(ssp->status.bsy);
    //ssp->ctrl1.sse = 0;

    if(ssp_env.ssp_cs_ctrl)
    {
        ssp_env.ssp_cs_ctrl(SSP_CS_DISABLE);
    }
}
void ssp_get_data_(unsigned char* buf, uint32_t size)
{
    unsigned char c;
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    while(size > 0)
    {
        //while((ssp->status.rne == 0) && (ssp->status.tnf == 1));
        while(ssp->status.rne == 0);
        if(ssp->status.rne != 0)    // receive fifo is not empty
        {
            c = ssp->data.data;
            *buf++ = c;
            size --;
        }
    }
}

/*********************************************************************
 * @fn      ssp_recv_data
 *
 * @brief   receive data from peripheral.
 *
 * @param   buffer  - data pointer to store received data
 *          length  - how many bytes to be send
 *
 * @return  None.
 */
void ssp_recv_data(uint8_t *buffer, uint32_t length)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    uint32_t blocks;

    blocks = length / SSP_FIFO_SIZE;

    if(ssp_env.ssp_cs_ctrl)
    {
        ssp_env.ssp_cs_ctrl(SSP_CS_ENABLE);
    }

    // ssp->ctrl1.sse = 1;

    while(blocks)
    {
        for(uint8_t i=0; i<SSP_FIFO_SIZE; i++)
        {
            ssp->data.data = 0;
        }
        while(ssp->status.rne == 0);
        while(ssp->status.rne)
        {
            *buffer++ = ssp->data.data;
        }
        blocks--;
        length -= SSP_FIFO_SIZE;
    }

    while(length)
    {
        ssp->data.data = 0;
        length--;
    }
    while(ssp->status.rne == 0);
    while(ssp->status.rne)
    {
        *buffer++ = ssp->data.data;
    }

    while(ssp->status.bsy);
    //ssp->ctrl1.sse = 0;

    if(ssp_env.ssp_cs_ctrl)
    {
        ssp_env.ssp_cs_ctrl(SSP_CS_DISABLE);
    }
}

/*********************************************************************
 * @fn      ssp_send_data
 *
 * @brief   send data to peripheral.
 *
 * @param   buffer  - data pointer to be send
 *          length  - how many bytes to be send
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void ssp_send_data(uint8_t *buffer, uint32_t length)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    uint32_t blocks;
    uint8_t *tx_fifo_addr;

    blocks = length / SSP_FIFO_SIZE;
    tx_fifo_addr = (uint8_t *)&ssp->data;

    if(ssp_env.ssp_cs_ctrl)
    {
        ssp_env.ssp_cs_ctrl(SSP_CS_ENABLE);
    }

    //ssp->ctrl1.sse = 1;
    while(blocks)
    {
        while(ssp->status.tfe == 0);
        for(uint8_t i=0; i<SSP_FIFO_SIZE/16; i++)
        {
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
            *tx_fifo_addr = *buffer++;
        }
        blocks--;
        length -= SSP_FIFO_SIZE;
    }
    while(length)
    {
        while(ssp->status.tnf == 0);
        ssp->data.data = *buffer++;
        length--;
    }
    while(ssp->status.bsy);
    //ssp->ctrl1.sse = 0;


    if(ssp_env.ssp_cs_ctrl)
    {
        ssp_env.ssp_cs_ctrl(SSP_CS_DISABLE);
    }
}
__attribute__((section("ram_code"))) void ssp_send_bytes(const uint8_t *tx_buf, uint32_t length)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    uint8_t *tx_fifo_addr = (uint8_t *)&ssp->data;

    while(length--)
    {
        while(ssp->status.tnf == 0);
        *tx_fifo_addr = *tx_buf++;
    }
}

void ssp_put_byte(uint32_t ssp_addr,const u16 c);
__attribute__((section("ram_code"))) void ssp_send_byte(const uint16_t tx_value)
{
    ssp_put_byte(SSP0_BASE,tx_value);
}
__attribute__((section("ram_code"))) void ssp_wait_send_end(void)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    while(ssp->status.tfe == 0);
}

__attribute__((section("ram_code"))) void ssp_send_120Bytes(const uint8_t *tx_buf)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    uint8_t *tx_fifo_addr = (uint8_t *)&ssp->data;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    //80~120
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;
    *tx_fifo_addr= *tx_buf++;

}

/*********************************************************************
 * @fn      ssp_init_
 *
 * @brief   Initialize ssp instance.
 *
 * @param   bit_width   - trans or recv bits witdh
 *          frame_type  -
 *          ms          - indicate ssp controller working mode
 *          bit_rate    - ssp bus frame clock, min_bus_clk = 184HZ, when prescale=254,cpu_clk = 12M;
 *                        max_bus_clk = 24MHZ, when prescale=2,cpu_clk = 48M; 						
 *          prescale    - ssp controller prescale
 *          ssp_cs_ctrl - if cs is controlled by software, this parameter
 *                        should be corresponding function.
 *
 * @return  None.
 */
void ssp_init_(uint8_t bit_width, uint8_t frame_type, uint8_t ms, uint32_t bit_rate, uint8_t prescale, void (*ssp_cs_ctrl)(uint8_t))
{
    volatile uint8_t data;
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;

    /* reset all */
    *(int *)(&(ssp->ctrl0)) = 0;
    *(int *)(&(ssp->ctrl1)) = 0;
    ssp->ctrl1.sse = 0;

    /* data size */
    ssp->ctrl0.dss = bit_width - 1;

    /* frame type */
    ssp->ctrl0.frf = frame_type;
    ssp->ctrl0.spo = 1;     //mode 3, clk idle is high
    ssp->ctrl0.sph = 1;     //mode 3, clk rising edge to sample

    /* clock rate */
    ssp->clock_prescale.cpsdvsr = prescale;
    ssp->ctrl0.scr = (system_get_pclk()/(prescale*bit_rate)) - 1;

    if(ms == 0x1)
        ssp->ctrl1.sod = 0x0;   //slave mode, slave can send
    else
        ssp->ctrl1.sod = 0x1;   //master mode, slave can't send
    /* working mode */
    ssp->ctrl1.ms = ms;

    ssp->ctrl1.sse = 1;

    /* clear rx fifo */
    while(ssp->status.rne == 1)
        data = ssp->data.data;

    //wait tx fifo to empty
    while(ssp->status.tfe == 0)
        ;

    ssp_env.ssp_cs_ctrl = ssp_cs_ctrl;

}

/*********************************************************************
 * @fn      ssp_enable_interrupt
 *
 * @brief   set ssp interrupt condition.
 *
 * @param   ints  - reference enum ssp_int_type_t
 *			
 * @return  None.
 */
void ssp_enable_interrupt(uint8_t ints)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    
    if(ints & SSP_INT_RX_FF) {
        ssp->ctrl1.rie = 1;
    }
    if(ints & SSP_INT_TX_FF) {
        ssp->ctrl1.tie = 1;
    }
    if(ints & SSP_INT_RX_FFOV) {
        ssp->ctrl1.rorie = 1;
    }
}

/*********************************************************************
 * @fn      ssp_disable_interrupt
 *
 * @brief   clear ssp interrupt condition.
 *
 * @param   ints  - reference enum ssp_int_type_t
 *			
 * @return  None.
 */
void ssp_disable_interrupt(uint8_t ints)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    
    if(ints & SSP_INT_RX_FF) {
        ssp->ctrl1.rie = 0;
    }
    if(ints & SSP_INT_TX_FF) {
        ssp->ctrl1.tie = 0;
    }
    if(ints & SSP_INT_RX_FFOV) {
        ssp->ctrl1.rorie = 0;
    }
}

/*********************************************************************
 * @fn      ssp_get_isr_status
 *
 * @brief   get current interrupt status
 *
 * @param   None
 * 
 * @return  current status ,@ref ssp_int_status_t.
 */
__attribute__((section("ram_code"))) uint32_t ssp_get_isr_status(void)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    return *(volatile uint32_t *)&ssp->iir;
}

/*********************************************************************
 * @fn      ssp_clear_isr_status
 *
 * @brief   used to clear interrupt status
 *
 * @param   status  - which status should be cleard, @ref ssp_int_status_t
 *
 * @return  None
 */
__attribute__((section("ram_code"))) void ssp_clear_isr_status(uint32_t status)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;
    *(volatile uint32_t *)&ssp->iir = status;
}

/*********************************************************************
 * @fn      ssp_put_data_to_fifo
 *
 * @brief   put data to send fifo, this function will return when all data have
 *          been put into tx fifo or tx fifo is full
 *
 * @param   buffer  - data pointer to be send
 *          length  - how many bytes to be send.
 *
 * @return  how many data have been put into tx fifo.
 */
__attribute__((section("ram_code"))) void ssp_put_data_to_fifo(uint8_t *buffer, uint16_t length)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP0_BASE;

    for(uint16_t i=0; i<length; i++) {
        *(volatile uint32_t *)&ssp->data = (uint32_t)*buffer++;
    }
}

__attribute__((section("ram_code"))) void ssp_cs_ctrl_function(uint8_t op)
{
    if(op == SSP_CS_ENABLE)
    {
        gpio_porta_write(gpio_porta_read() & 0xDF);
    }
    else
    {
        gpio_porta_write(gpio_porta_read() | 0x20);
    }
}


#if 0
__attribute__((section("ram_code"))) void ssp_test(uint8_t *buffer, uint32_t length)
{
    //uint8_t test_buffer[530];
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSP0_CLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_A5);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSP0_DOUT);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_SSP0_DIN);
    gpio_set_dir(GPIO_PORT_A, GPIO_BIT_5, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() | 0x20);

    /* working in master mode */
    ssp_init_(8, SSP_FRAME_MOTO, SSP_MASTER_MODE, 24000000, 2, ssp_cs_ctrl_function);

    #if 1
    if((uint32_t)buffer >= 0x01100000) {
    }
    else {
        uint32_t space_to_boundary = 0x01100000 - (uint32_t)buffer;
        if(space_to_boundary >= length) {
            ssp_send_data(buffer, length);
            return;
        }
        else {
            ssp_send_data(buffer, space_to_boundary);
            buffer = (uint8_t *)0x01100000;
            length -= space_to_boundary;
        }
    }
    
    // out of 8Mb range
    uint32_t single_trans_length = 0x1000;
    while(length > 0) {
        if(length > single_trans_length) {
            single_trans_length = 0x1000;
        }
        else {
            single_trans_length = length;
        }
        GLOBAL_INT_DISABLE();
        // invalid cache first
        *(volatile uint32_t *)0x500a0000 = 0x3c;
        while((*(volatile uint32_t *)0x500a0004) & 0x03);
        *(volatile uint32_t *)0x500a0000 = 0x3e;
        while((*(volatile uint32_t *)0x500a0000) & 0x02);
        *(volatile uint32_t *)0x500a0000 = 0x3d;
        while(((*(volatile uint32_t *)0x500a0004) & 0x03) != 0x02);
        
        // set qspi dac address to 0x00F00000
        *(uint32_t *)0x500B0024 = (uint32_t)buffer & 0xFFF00000;
        ssp_send_data(buffer-0x00100000, single_trans_length);
        // set qspi dac address to 0x01000000
        *(uint32_t *)0x500B0024 = 0x01000000;
        
        // invalid cache
        *(volatile uint32_t *)0x500a0000 = 0x3c;
        while((*(volatile uint32_t *)0x500a0004) & 0x03);
        *(volatile uint32_t *)0x500a0000 = 0x3e;
        while((*(volatile uint32_t *)0x500a0000) & 0x02);
        *(volatile uint32_t *)0x500a0000 = 0x3d;
        while(((*(volatile uint32_t *)0x500a0004) & 0x03) != 0x02);
        GLOBAL_INT_RESTORE();
        length -= single_trans_length;
        buffer += single_trans_length;
    }
    //ssp_send_data(buffer, length);
    #else
    while(length > 512) {
        memcpy(test_buffer, buffer, 512);
        ssp_send_data(test_buffer, 512);
        length -= 512;
        buffer += 512;
    }
    if(length) {
        memcpy(test_buffer, buffer, length);
        ssp_send_data(test_buffer, length);
    }
    #endif
}
#endif

