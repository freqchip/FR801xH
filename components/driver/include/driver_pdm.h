#ifndef _DRIVER_PDM_H
#define _DRIVER_PDM_H

#include <stdint.h>           // standard integer functions

#include "driver_plf.h"

enum pdm_freq
{
    PDM_FREQ_16000 = 0,
    PDM_FREQ_8000,
};
enum pdm_mode
{
    PDM_MODE_STEREO = 0,    //double channel, PDM_FFRXDATA right&left channel data is valid
    PDM_MODE_MONO = 1,      //single channel, PDM_FFRXDATA left channel data is valid
};

#define PDM_REG_BASE_ADDRESS        PDM_BASE

#define PDM_REG_CFG                 PDM_REG_BASE_ADDRESS
#define PDM_REG_VOLGAIN             (PDM_REG_BASE_ADDRESS+0x04)
#define PDM_REG_VOLSTEP             (PDM_REG_BASE_ADDRESS+0x08)
#define PDM_REG_FFRXDATA            (PDM_REG_BASE_ADDRESS+0x0C)
#define PDM_REG_FFCLR               (PDM_REG_BASE_ADDRESS+0x10)
#define PDM_REG_FFIND               (PDM_REG_BASE_ADDRESS+0x14)
#define PDM_REG_INTEN               (PDM_REG_BASE_ADDRESS+0x18)
#define PDM_AFLR_INTEN              (PDM_REG_BASE_ADDRESS+0x1C)  // FIFO half full LEVEL

/*bitmap*/
/*CONTROL REG*/
#define bmCTRL_PDMEN                (1<<0)
#define bmCTRL_MONO                 (1<<1)
#define bmCTRL_LEFTRISING           (1<<2)
#define bmCTRL_SRMODE               (1<<3) // 0,16KHz;1,8KHz
#define bmCTRL_HPFEN                (1<<4)
#define bmCTRL_ZERODETEN            (1<<5)

/*FIFO CLEAR reg*/
#define bmSTATUS_FFWRCLR            (1<<0)
#define bmSTATUS_FFRDCLR            (1<<1)

/*interrupt enable reg*/
#define bmINTEN_RXFFFULLINTEN       (1<<0)
#define bmINTEN_RXFFAFULLINTEN      (1<<1)
#define bmINTEN_RXFFEMPTYINTEN      (1<<2)

/*status reg*/
#define bmSTATUS_RXFFFULL           (1<<0)
#define bmSTATUS_RXFFHFULL          (1<<1)
#define bmSTATUS_RXFFEMPTY          (1<<2)

void pdm_start(enum pdm_freq freq,enum pdm_mode mode);
void  pdm_fifo_flush(void);
void  pdm_rxfifo_wptr_clr(void);
void  pdm_rxfifo_rptr_clr(void);
void  pdm_volume_ctrl(uint32_t stepctrl, uint32_t volgain_r, uint32_t volgain_l);
void pdm_stop(void);


#endif

