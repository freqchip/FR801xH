#include "driver_pdm.h"
#include "driver_plf.h"
//#include "Reg_access.h"
#include "driver_system.h"

void pdm_start(enum pdm_freq freq,enum pdm_mode mode)
{
    system_regs->rst.pdm_soft_rst = 0;
    system_regs->rst.pdm_soft_rst = 1;
    // PDM basic configuration
    REG_PL_WR(PDM_REG_CFG,  \
              bmCTRL_PDMEN+   \
              (mode<<1)+    \
              bmCTRL_LEFTRISING+  \
              (freq<<3)+    \
              bmCTRL_HPFEN );

    // Interrupt type
    REG_PL_WR(PDM_REG_INTEN,
              //bmINTEN_RXFFFULLINTEN+
              bmINTEN_RXFFAFULLINTEN
              //bmINTEN_RXFFEMPTYINTEN
             );

    // set almost full Threshold level
    // Control the level of entries (or above) that triggers the
    // AFULL interrupt. The valid range is 0-63.
    // Reset value: 0x20
    REG_PL_WR(PDM_AFLR_INTEN,32);

    // RXFIFO pointer clear
    pdm_rxfifo_wptr_clr();
    pdm_rxfifo_rptr_clr();
}
void pdm_stop(void)
{
    // PDM basic configuration
    REG_PL_WR(PDM_REG_CFG, REG_PL_RD(PDM_REG_CFG)& ~(1<<0) );
    REG_PL_WR(PDM_REG_INTEN,0);
}

void  pdm_rxfifo_wptr_clr(void)
{
    REG_PL_WR(PDM_REG_FFCLR,bmSTATUS_FFWRCLR);
    REG_PL_WR(PDM_REG_FFCLR,0x0);
}

void  pdm_rxfifo_rptr_clr(void)
{
    REG_PL_WR(PDM_REG_FFCLR,bmSTATUS_FFRDCLR);
    REG_PL_WR(PDM_REG_FFCLR,0x0);
}

// volgain range from 0~4095, 0 denotes mute, default value: 0x172
// volgain Vs dB: 20xlog10((volgain)/256) dB
// stepctrl used to control volume changing speed: each (stepctrl+1)
// clk samples(8k/16k), volume ++ or --
// stepctrl range from 0 ~ 1023, default value 0
void  pdm_volume_ctrl(uint32_t stepctrl, uint32_t volgain_r, uint32_t volgain_l)
{
    REG_PL_WR(PDM_REG_VOLSTEP, stepctrl);
    REG_PL_WR(PDM_REG_VOLGAIN, (volgain_r << 16)+volgain_l);
}

//static uint16_t rx_fifo_data_pdm[64];
//static uint8_t  wr_index_pdm = 0;

#if 0
__attribute__((weak)) __attribute__((section("ram_code"))) void pdm_isr_ram(void)
{
    while((REG_PL_RD(PDM_REG_FFIND)&bmSTATUS_RXFFEMPTY) == 0)
    {
  //      rx_fifo_data_pdm[wr_index_pdm++] = REG_PL_RD(PDM_REG_FFRXDATA);
  //      if(wr_index_pdm == 64)
        {
  //          wr_index_pdm = 0;
        }
    }
}
#endif
