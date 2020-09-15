#ifndef _DRIVER_SSP_H
#define _DRIVER_SSP_H

#include <stdint.h>      // standard integer definition

/*
 * TYPEDEFS (类型定义)
 */
#define SSP_FIFO_SIZE           128

enum ssp_frame_type_t
{
    SSP_FRAME_MOTO,
    SSP_FRAME_SS,
    SSP_FRAME_NATTIONAL_M,
};

enum ssp_ms_mode_t
{
    SSP_MASTER_MODE,
    SSP_SLAVE_MODE,
};

enum ssp_cs_ctrl_op_t
{
    SSP_CS_ENABLE,
    SSP_CS_DISABLE,
};

enum ssp_int_type_t {
    SSP_INT_RX_FF   = (1<<0),
    SSP_INT_TX_FF   = (1<<1),
    SSP_INT_RX_FFOV = (1<<2),
};

enum ssp_int_status_t {
    SSP_INT_STATUS_RX       = (1<<0),
    SSP_INT_STATUS_TX       = (1<<1),
    SSP_INT_STATUS_RX_FFOV  = (1<<2),
};

/*
 * PUBLIC FUNCTIONS (全局函数)
 */
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
void ssp_send_then_recv(uint8_t* tx_buffer, uint32_t n_tx, uint8_t* rx_buffer, uint32_t n_rx);

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
void ssp_recv_data(uint8_t *buffer, uint32_t length);

/*********************************************************************
 * @fn      ssp_send_data
 *
 * @brief   Immediately send a lot of bytes data on ssp0 device and will block until data is sent out
 *
 * @param   buffer  - Pointer to transmit buffer
 *          length  - Total data length, in bytes
 *
 * @return  None.
 */
void ssp_send_data(uint8_t *buffer, uint32_t length);
/**
 * @brief   Immediately send a byte data on ssp0 device and will not block until data is sent out
 *
 * @note    Normally a device cannot start (queue) polling and interrupt
 *          transactions simutanuously.
 *
 * @param   tx_value -tx data value, can be 8bit or 16 bit value
 *
 * @return  None
 */
void ssp_send_byte(const uint16_t tx_value);
/**
 * @brief   Immediately send a lot of bytes data on ssp0 device and will not block until data is sent out
 *
 * @note    Normally a device cannot start (queue) polling and interrupt
 *          transactions simutanuously.
 *
 * @param   tx_buf -Pointer to transmit buffer
 *          length  - Total data length, in bytes
 *
 * @return None
 */
void ssp_send_bytes(const uint8_t *tx_buf, uint32_t length);
/**
 * @brief   Immediately send 120 bytes data on ssp0 device and will not block until data is sent out
 *
 * @note    this function is only for the case, which require sending 120 bytes one time.
 *          for example, push data for tft screen refresh. 
 *
 * @param   tx_buf -Pointer to transmit buffer
 *
 * @return  None
 */
void ssp_send_120Bytes(const uint8_t *tx_buf);
/**
 * @brief will block until data is sent out
 *
 * @note    Normally call this function after "ssp_send_byte" or "ssp_send_bytes" or "ssp_send_120Bytes"
 *          to block until data is sent out 
 *
 * @param   None
 *
 * @return  None
 */
void ssp_wait_send_end(void);

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
void ssp_init_(uint8_t bit_width, uint8_t frame_type, uint8_t ms, uint32_t bit_rate, uint8_t prescale, void (*ssp_cs_ctrl)(uint8_t));

/*********************************************************************
 * @fn      ssp_enable_interrupt
 *
 * @brief   set ssp interrupt condition.
 *
 * @param   ints  - reference enum ssp_int_type_t
 *			
 * @return  None.
 */
void ssp_enable_interrupt(uint8_t ints);

/*********************************************************************
 * @fn      ssp_disable_interrupt
 *
 * @brief   clear ssp interrupt condition.
 *
 * @param   ints  - reference enum ssp_int_type_t
 *			
 * @return  None.
 */
void ssp_disable_interrupt(uint8_t ints);

/*********************************************************************
 * @fn      ssp_get_isr_status
 *
 * @brief   get current interrupt status
 *
 * @param   None
 * 
 * @return  current status ,@ref ssp_int_status_t.
 */
__attribute__((section("ram_code"))) uint32_t ssp_get_isr_status(void);

/*********************************************************************
 * @fn      ssp_clear_isr_status
 *
 * @brief   used to clear interrupt status
 *
 * @param   status  - which status should be cleard, @ref ssp_int_status_t
 *
 * @return  None
 */
__attribute__((section("ram_code"))) void ssp_clear_isr_status(uint32_t status);

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
__attribute__((section("ram_code"))) void ssp_put_data_to_fifo(uint8_t *buffer, uint16_t length);

void ssp_test(uint8_t *buffer, uint32_t length);
void ssp_get_data_(unsigned char* buf, uint32_t size);

#endif

