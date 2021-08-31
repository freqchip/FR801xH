/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "demo_peripheral.h"
#include "co_printf.h"
#include "sys_utils.h"

#if (UART0_DEMO_ENABLE)
/*
PA3 as log output
pin mapping
UART_device 8016H
UART_RXD    PD7
UART_TXD    PD6
GND         GND
*/
#include "driver_iomux.h"
#include "driver_system.h"
#include "driver_uart.h"
#include "driver_plf.h"

__attribute__((section("ram_code"))) void uart0_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART0_BASE;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg->lsr & 0x01)
        {
            c = uart_reg->u1.data;
            uart_putc_noint_no_wait(UART0,c);
        }
    }
    else if(int_id == 0x06)
    {
        //uart_reg->u3.iir.int_id = int_id;
        //uart_reg->u2.ier.erlsi = 0;
        volatile uint32_t line_status = uart_reg->lsr;
    }
}
__attribute__((section("ram_code"))) void uart1_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART1_BASE;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg->lsr & 0x01)
        {
            c = uart_reg->u1.data;
            uart_putc_noint_no_wait(UART1,c);
        }
    }
    else if(int_id == 0x06)
    {
        //uart_reg->u3.iir.int_id = int_id;
        //uart_reg->u2.ier.erlsi = 0;
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

void demo_uart0(void)
{
    system_set_port_pull(GPIO_PD6, true);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_UART0_RXD);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_UART0_TXD);
    uart_init(UART0, BAUD_RATE_115200);
    co_printf("uart0 demo\r\n");

    system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    //uart_init(UART1, BAUD_RATE_115200);
    uart_param_t param =
    {
        .baud_rate = 115200,
        .data_bit_num = 8,
        .pari = 0,
        .stop_bit = 1,
    };
    uart_init1(UART1, param);


    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART1_IRQn);
    co_printf("uart1 demo\r\n");
}
#endif



#define FLASH_PAGE_SIZE (0x1000)
#define USER_FLASH_BASE_ADDR (0x32000)  //200K
#define USER_FLASH_MAX_PAGE_ADDR  (0x80000 - FLASH_PAGE_SIZE)   //512K-4K
uint8_t buf[FLASH_PAGE_SIZE];

#if (FLASH_DEMO_ENABLE)
#include "driver_flash.h"
#include "sys_utils.h"

void demo_flash(void)
{
    uint32_t id;
    co_printf("flash demo\r\n");

    qspi_flash_init(2);
    id = (id & 0xFFFFFF);
    co_printf("flash id:%x\r\n",id);

    flash_erase(USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE);
    uint32_t idx = 0;
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = idx;
    flash_write(USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE, buf);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = 0xff;
    flash_read(USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE, buf);
    co_printf("read reg:\r\n");
    show_reg(buf,5,1);
    show_reg(buf+4091,5,1);

    flash_erase(USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = idx+1;
    flash_write(USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE, buf);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = 0xfe;
    flash_read(USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE, buf);
    co_printf("read reg:\r\n");
    show_reg(buf,5,1);
    show_reg(buf+4091,5,1);


    flash_erase(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = idx+2;
    flash_write(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE, buf);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = 0xfd;
    flash_read(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE, buf);
    co_printf("read reg:\r\n");
    show_reg(buf,5,1);
    show_reg(buf+4091,5,1);


    flash_erase(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = idx+3;
    flash_write(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE, buf);
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = 0xfc;
    flash_read(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE, buf);
    co_printf("read reg:\r\n");
    show_reg(buf,5,1);
    show_reg(buf+4091,5,1);

    flash_erase(USER_FLASH_MAX_PAGE_ADDR, FLASH_PAGE_SIZE);
//flash_chip_erase();
}
#endif


#if (SPI_FLASH_DEMO_ENABLE)
/*
pin mapping
SPI_FLASH   8010H
CLK         PA4
CS#         PA5
DI          PA6
DO          PA7
VCC         VDDIO
VSS         GND
*/
#include "driver_plf.h"
#include "driver_flash_ssp.h"
#include "driver_system.h"
#include "driver_iomux.h"
#include "sys_utils.h"

#define FLASH_PAGE_SIZE (0x1000)
#define USER_FLASH_BASE_ADDR (0x32000)  //200K
#define USER_FLASH_MAX_PAGE_ADDR  (0x80000 - FLASH_PAGE_SIZE)   //512K-4K
uint8_t buf[FLASH_PAGE_SIZE];
void demo_spi_flash(void)
{
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSP0_CLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSP0_CSN);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSP0_DOUT);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_SSP0_DIN);

    ssp_init_(8,SSP_FRAME_MOTO, SSP_MASTER_MODE,1000000,2,NULL);
    uint32_t id = 0;
    ssp_flash_identify(SSP0_BASE,(uint8_t *)&id);
    co_printf("ssp_flash id:%x\r\n",(id & 0xFFFFFF));

    ssp_flash_erase(SSP0_BASE,USER_FLASH_BASE_ADDR, 0);
    co_printf("ssp_flash_erase:0x%X\r\n",USER_FLASH_BASE_ADDR);
    uint32_t idx = 0;
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = idx+1;
    ssp_flash_write(SSP0_BASE,USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE, buf);
    co_printf("ssp_flash_write\r\n");
    for(idx=0; idx<FLASH_PAGE_SIZE; idx++)
        buf[idx] = 0xff;
    ssp_flash_read(SSP0_BASE,USER_FLASH_BASE_ADDR, FLASH_PAGE_SIZE, buf);
    co_printf("ssp_flash_read reg:0x%X\r\n",USER_FLASH_BASE_ADDR);
    show_reg(buf,5,1);
    show_reg(buf+4091,5,1);
}
#endif



#if HWTIM_DEMO_ENABLE
#include "driver_timer.h"
static uint32_t tick0 = 0;
static uint32_t tick1 = 0;
static uint8_t tim0_run_num = 0;
__attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    timer_clear_interrupt(TIMER0);
    co_delay_10us(1);
    //timer_reload(TIMER0);
    if(tick0>10000) //1s
    {
        tick0 = 0;
        tim0_run_num++;
        if(tim0_run_num >= 4)
            timer_stop(TIMER0);
        co_printf("hwt0_us:%d,%d\r\n",timer_get_current_value(TIMER0),timer_get_load_value(TIMER0));
    }
    else
        tick0++;
}
__attribute__((section("ram_code"))) void timer1_isr_ram(void)
{
    timer_clear_interrupt(TIMER1);
    co_delay_10us(1);
    //timer_reload(TIMER1);
    if(tick1>20000) //4s
    {
        timer_stop(TIMER1);
        co_printf("hwt1_us:%d,%d\r\n",timer_get_current_value(TIMER1),timer_get_load_value(TIMER1));
    }
    else
        tick1++;
}

void demo_timer(void)
{
    co_printf("hardware timer demo\r\n");

    tick1 = 0;
    tick0 = 0;
    tim0_run_num = 0;
    timer_init(TIMER0,100,TIMER_PERIODIC);
    timer_run(TIMER0);

    timer_init(TIMER1,200,TIMER_PERIODIC);
    timer_run(TIMER1);

    NVIC_SetPriority(TIMER0_IRQn, 2);
    NVIC_EnableIRQ(TIMER0_IRQn);

    NVIC_SetPriority(TIMER1_IRQn, 2);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

#endif

#if I2C_DEMO_ENABLE
#include "driver_system.h"
#include "driver_iomux.h"

#include "driver_iic.h"
static const uint8_t LSM6DS33_ADDR = (0x6B<<1);
void demo_i2c(void)
{
    uint8_t i2c_test_no = 0;
    uint8_t buff1[6];
    uint8_t buff2[6];
    co_printf("IIC demo\r\n");

    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_I2C1_CLK);//Pc6
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_I2C1_DAT);//Pc7
    system_set_port_pull( (GPIO_PD6|GPIO_PD7), false);
//IIC1, 1M hz. slave addr: 0xd6
    iic_init(IIC_CHANNEL_1,1000,LSM6DS33_ADDR);
    iic_write_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x15,0x00);
    iic_write_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x16,0x00);
    iic_write_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x10,0x80);
    iic_write_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x11,0x80);
    iic_write_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x12,0x04);

    while(1)
    {
        for(uint8_t i=0; i<6; i++)
        {
            iic_read_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x28+i,&buff1[i]);
            iic_read_byte(IIC_CHANNEL_1,LSM6DS33_ADDR,0x22+i,&buff2[i]);
        }
        co_printf("X:%d |Y:%d |Z:%d\r\n",*(uint16_t *)buff1,*(uint16_t *)(buff1+2),*(uint16_t *)(buff1+4));
        co_printf("AccX:%d|Acc_Y:%d|Acc_Z:%d\r\n",*(uint16_t *)buff2,*(uint16_t *)(buff2+2),*(uint16_t *)(buff2+4));
        i2c_test_no++;
        if(i2c_test_no>3)
            break;
    }

}
#endif

#if ADC_DEMO_ENABLE
#include "driver_iomux.h"
#include "driver_adc.h"
#include "os_timer.h"

os_timer_t adc_timer;
void adc_tim_fn(void *arg)
{
    struct adc_cfg_t cfg;
    uint16_t result;
    memset((void*)&cfg, 0, sizeof(cfg));
    cfg.src = ADC_TRANS_SOURCE_PAD;
    cfg.ref_sel = ADC_REFERENCE_AVDD;
    cfg.channels = 0x01;
    cfg.route.pad_to_sample = 1;
    cfg.clk_sel = ADC_SAMPLE_CLK_24M_DIV13;
    cfg.clk_div = 0x3f;
    adc_init(&cfg);
    adc_enable(NULL, NULL, 0);
    adc_get_result(ADC_TRANS_SOURCE_PAD, 0x01, &result);
    adc_disable();
    co_printf("result:%d\r\n",result);
}
void demo_adc(void)
{
    co_printf("adc demo\r\n");
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_ADC0);
    os_timer_init(&adc_timer,adc_tim_fn,NULL);
    os_timer_start(&adc_timer,1000,1);
}
#endif

#if PWM_DEMO_ENABLE
/*
PA3 as log output
pin mapping
8016H
PD4  PWM4
PD5  PWM5
*/
#include "driver_pwm.h"
#include "driver_iomux.h"
#include "driver_system.h"
#include "sys_utils.h"

void demo_pwm(void)
{
    co_printf("digital_pwm demo\r\n");

    system_set_port_mux(GPIO_PORT_D,GPIO_BIT_4,PORTD4_FUNC_PWM4);
    system_set_port_mux(GPIO_PORT_D,GPIO_BIT_5,PORTD5_FUNC_PWM5);
    pwm_init(PWM_CHANNEL_4,1000,50);
    pwm_init(PWM_CHANNEL_5,1000,80);

    pwm_start(PWM_CHANNEL_4);
    pwm_start(PWM_CHANNEL_5);

    co_delay_100us(20000);  //1K hz for 2s
    pwm_update(PWM_CHANNEL_4,10000,50);
    pwm_update(PWM_CHANNEL_5,10000,80);
    co_delay_100us(10000);  //10K hz for 1s
    pwm_stop(PWM_CHANNEL_4);
    pwm_stop(PWM_CHANNEL_5);
    co_delay_100us(20000);  //stop for 2s
    pwm_start(PWM_CHANNEL_4);
    pwm_start(PWM_CHANNEL_5);
    co_delay_100us(10000);  //restart pwm with 10K hz for 1s
    pwm_stop(PWM_CHANNEL_4);
    pwm_stop(PWM_CHANNEL_5);
}
#endif

#if PMU_PWM_DEMO_ENABLE
/*
PA3 as log output
pin mapping
8016H
PD4  PWM4
PD5  PWM5
*/
#include "driver_pmu_pwm.h"
#include "driver_pmu.h"
#include "driver_iomux.h"
#include "driver_system.h"
#include "sys_utils.h"

void demo_pmu_pwm(void)
{
    co_printf("pmu_pwm demo\r\n");
    pmu_pwm_init();
    pmu_set_pin_to_PMU(GPIO_PORT_D, BIT(4)|BIT(5));
    pmu_set_pin_dir(GPIO_PORT_D, BIT(4)|BIT(5),GPIO_DIR_OUT);

    pmu_set_port_mux(GPIO_PORT_D,GPIO_BIT_4,PMU_PORT_MUX_PWM);
    pmu_set_port_mux(GPIO_PORT_D,GPIO_BIT_5,PMU_PORT_MUX_PWM);

    pmu_pwm_set_param(GPIO_PORT_D,GPIO_BIT_4,2000,1000);
    pmu_pwm_set_param(GPIO_PORT_D,GPIO_BIT_5,2000,1000);
//PD4,will keep output
    pmu_pwm_start(GPIO_PORT_D,GPIO_BIT_4,true,0);
//PD5,will keep output
    pmu_pwm_start(GPIO_PORT_D,GPIO_BIT_5,true,0);

    co_delay_100us(20000);  //for 2s
    pmu_pwm_set_param(GPIO_PORT_D,GPIO_BIT_4,1000,2000);
    pmu_pwm_set_param(GPIO_PORT_D,GPIO_BIT_5,1000,2000);
    co_delay_100us(10000);  //10K hz for 1s
    pmu_pwm_stop(GPIO_PORT_D,GPIO_BIT_4);
    pmu_pwm_stop(GPIO_PORT_D,GPIO_BIT_5);

    co_delay_100us(20000);  //stop for 2s
//PD4,will last only one period
    pmu_pwm_start(GPIO_PORT_D,GPIO_BIT_4,false,0);
//PD5,will last only one period
    pmu_pwm_start(GPIO_PORT_D,GPIO_BIT_5,false,0);
    co_delay_100us(10000);  //restart pwm for 1s
    pmu_pwm_stop(GPIO_PORT_D,GPIO_BIT_4);
    pmu_pwm_stop(GPIO_PORT_D,GPIO_BIT_5);
}
#endif


#if LVD_DEMO_ENABLE
#include "driver_pmu.h"
__attribute__((section("ram_code"))) void lvd_isr_ram(void)
{
    co_printf("lvd detected\r\n");
    pmu_disable_isr(PMU_ISR_LVD_EN);
}

void demo_lvd(void)
{
    co_printf("lvd demo\r\n");
    //lvd_set_voltage(0xf);
    //lvd_enable();
    pmu_enable_irq(PMU_ISR_BIT_LVD);
    NVIC_SetPriority(PMU_IRQn, 4);
    NVIC_EnableIRQ(PMU_IRQn);
}
#endif


#if KEYSCAN_TEST_ENABLE
/*
PA3 as log output
pin mapping
8016H
PA4~PA7  KEYSCAN_ROW
PD4~PD7  KEYSCAN_COL
*/
#include "driver_keyscan.h"
#include "driver_pmu_regs.h"
#include "driver_pmu.h"
#include "sys_utils.h"
#include "os_task.h"

static uint32_t keyscan_press[5] = {0};
uint8_t key_task_id = TASK_ID_FAIL;
static int key_task_func(os_event_t *msg)
{
    uint32_t key = *(uint32_t *)(msg->param);
    uint8_t key_group = ((msg->event_id)&0xff00)>>8;
    switch((msg->event_id)&0xff)
    {
        case 0:
            co_printf("l[%d]:%x\r\n",key_group,key);
            break;
        case 1:
            co_printf("p[%d]:%x\r\n",key_group,key);
            break;
        default:
            break;
    }
    return (EVT_CONSUMED);
}
__attribute__((section("ram_code"))) void keyscan_isr_ram(void)
{
    uint32_t value;
    uint8_t reg = PMU_REG_KEYSCAN_STATUS_0;

    for(uint8_t j=0; j<5; j++)
    {
        value = ool_read32(reg + (j<<2));
        if(value != keyscan_press[j])
        {
            os_event_t evt;
            if(value)
            {
                //co_printf("grp[%d]:0x%08x.\r\n",j+1,value);
                evt.event_id = ((j+1)<<8)|1;
            }
            else
            {
                //co_printf("grp[%d]:0x%08x.\r\n",j+1,value);
                evt.event_id = ((j+1)<<8);
            }
            evt.param_len = sizeof(value);
            evt.param = &value;
            evt.src_task_id = TASK_ID_FAIL;
            os_msg_post(key_task_id,&evt);
        }
        keyscan_press[j] = value;
    }
}

void demo_keyscan(void)
{
    keyscan_param_t param;
    co_printf("keyscan demo\r\n");

    param.row_en = 0xf0;    //PD4~PD7  as row
    param.col_en = 0xf0;    //PA4~PA7  as col
    param.row_map_sel = 0x0;
    pmu_set_pin_pull(GPIO_PORT_A, BIT(4)|BIT(5)|BIT(6)|BIT(7), true);
    pmu_set_pin_pull(GPIO_PORT_D, BIT(4)|BIT(5)|BIT(6)|BIT(7), true);

    keyscan_init(&param);
    key_task_id = os_task_create( key_task_func);

    system_sleep_enable();
}
#endif

#if PMU_QDEC_DEMO_ENABLE
/*
PA3 as log out
pin mapping
Qdec_dev    8016H
Line_A      PA4
Line_B      PA5
Line_C      PA7
VCC         VDDIO
GND         GND
*/
#include "driver_pmu_qdec.h"
#include "driver_iomux.h"
#include "driver_pmu_regs.h"
#include "sys_utils.h"
__attribute__((section("ram_code"))) void qdec_isr_ram(void)
{
    co_printf("%x,%x\r\n",pmu_qdec_get_cnt(QDEC_DIR_A),pmu_qdec_get_cnt(QDEC_DIR_B));
}
void demo_pmu_qdec(void)
{
    co_printf("pmu qdec demo\r\n");
    pmu_set_pin_to_PMU(GPIO_PORT_C,BIT(6)|BIT(5));
    pmu_set_pin_dir(GPIO_PORT_C,BIT(6)|BIT(5),GPIO_DIR_IN );
    pmu_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PMU_PORT_MUX_GPIO);
    pmu_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PMU_PORT_MUX_GPIO);
    //pmu_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PMU_PORT_MUX_GPIO);

    pmu_qdec_init();
    pmu_qdec_set_pin(PMU_QDEC_LA_PIN_PC6,PMU_QDEC_LB_PIN_PC5,PMU_QDEC_LC_PIN_PA7);
    pmu_qdec_autorest_cnt_flag(true,false);
    pmu_qdec_set_threshold(2);
    pmu_qdec_set_irq_type(PMU_ISR_QDEC_MULTI_EN);
    pmu_qdec_set_debounce_cnt(18);      //(2+18)*0.24 = 4.8ms
}

#endif

#if RTC_TEST_ENABLE
#include "driver_rtc.h"
#include "time_rtc.h"

__attribute__((section("ram_code"))) void rtc_isr_ram(uint8_t rtc_idx)
{
    if(rtc_idx == RTC_A)
    {
        co_printf("RTC_A\r\n");
    }
    if(rtc_idx == RTC_B)
    {
        co_printf("RTC_B\r\n");
    }
}

void demo_rtc(void)
{
    co_printf("rtc demo\r\n");
    rtc_init();
    rtc_alarm(RTC_A,1);
    rtc_alarm(RTC_B,1000);
}
#endif

#if (WDT_DEMO_ENABLE)
#include "driver_wdt.h"
__attribute__((section("ram_code"))) void wdt_isr_ram(unsigned int* hardfault_args)
{
    co_printf("wdt_rest\r\n\r\n");
    co_printf("PC    = 0x%08X\r\n",hardfault_args[6]);
    co_printf("LR    = 0x%08X\r\n",hardfault_args[5]);
    co_printf("R0    = 0x%08X\r\n",hardfault_args[0]);
    co_printf("R1    = 0x%08X\r\n",hardfault_args[1]);
    co_printf("R2    = 0x%08X\r\n",hardfault_args[2]);
    co_printf("R3    = 0x%08X\r\n",hardfault_args[3]);
    co_printf("R12   = 0x%08X\r\n",hardfault_args[4]);

    /* reset the system */
    ool_write(PMU_REG_RST_CTRL, ool_read(PMU_REG_RST_CTRL) & (~ PMU_RST_WDT_EN) );
}
void demo_wdt(void)
{
    co_printf("watchdog demo\r\n");
    wdt_init(WDT_ACT_CALL_IRQ, 4);
    wdt_start();
    co_delay_100us(50000);
    //wdt_stop();
}
#endif

#if GPIO_TEST_ENABLE
/*
PA3 as log output
pin mapping
8016H
PD4  Digital gpio output
PD5  Digital gpio output
PD6  Digital gpio input
PD7  Digital gpio input
*/
#include "driver_gpio.h"
#include "driver_system.h"
#include "sys_utils.h"
void demo_digital_gpio(void)
{
    co_printf("digital gpio demo\r\n");
    //digital gpio output
    system_set_port_mux(GPIO_PORT_D,GPIO_BIT_4,PORTD4_FUNC_D4);
    system_set_port_mux(GPIO_PORT_D,GPIO_BIT_5,PORTD5_FUNC_D5);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_4, GPIO_DIR_OUT);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_5, GPIO_DIR_OUT);

    gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_4,1);
    gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_5,1);
    co_delay_100us(10000);
    gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_4,0);
    gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_5,0);

    //digital gpio input
    system_set_port_mux(GPIO_PORT_D,GPIO_BIT_7,PORTD7_FUNC_D7);
    system_set_port_mux(GPIO_PORT_D,GPIO_BIT_6,PORTD6_FUNC_D6);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_7, GPIO_DIR_IN);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_6, GPIO_DIR_IN);
    system_set_port_pull( (GPIO_PD6|GPIO_PD7), false);
    co_printf("PD6:%d\r\n",gpio_get_pin_value(GPIO_PORT_D,GPIO_BIT_6) );
    co_printf("PD7:%d\r\n",gpio_get_pin_value(GPIO_PORT_D,GPIO_BIT_7) );
}

/*
PA3 as log output
pin mapping
8016H
PA4  PMU gpio output
PA5  PMU gpio output
PA6  PMU gpio input
PA7  PMU gpio input
*/
#include "driver_pmu.h"
void demo_pmu_gpio(void)
{
    co_printf("pmu gpio demo\r\n");

    //pmu gpio output
    pmu_set_port_mux(GPIO_PORT_A,GPIO_BIT_4,PMU_PORT_MUX_GPIO);
    pmu_set_port_mux(GPIO_PORT_A,GPIO_BIT_5,PMU_PORT_MUX_GPIO);
    pmu_set_pin_to_PMU(GPIO_PORT_A,BIT(4)|BIT(5) );
    pmu_set_pin_dir(GPIO_PORT_A,BIT(4)|BIT(5), GPIO_DIR_OUT);
    //plus pull may cost more 50ma during sleep.
    pmu_set_pin_pull(GPIO_PORT_A, BIT(4)|BIT(5), false);

    pmu_set_gpio_value(GPIO_PORT_A, BIT(4)|BIT(5), 1);
    co_delay_100us(10000);
    pmu_set_gpio_value(GPIO_PORT_A, BIT(4)|BIT(5), 0);

    //pmu gpio input
    system_set_port_mux(GPIO_PORT_A,GPIO_BIT_7,PORTA7_FUNC_A7);
    system_set_port_mux(GPIO_PORT_A,GPIO_BIT_6,PORTA6_FUNC_A6);
    pmu_set_pin_to_PMU(GPIO_PORT_A,BIT(6)|BIT(7) );
    pmu_set_pin_dir(GPIO_PORT_A,BIT(6)|BIT(7), GPIO_DIR_IN);

    pmu_set_pin_pull(GPIO_PORT_A, BIT(6)|BIT(7), false);
    co_printf("PA6:%d\r\n",pmu_get_gpio_value(GPIO_PORT_A,GPIO_BIT_6) );
    co_printf("PA7:%d\r\n",pmu_get_gpio_value(GPIO_PORT_A,GPIO_BIT_7) );
}

void demo_gpio(void)
{
    demo_digital_gpio();
    demo_pmu_gpio();
}
#endif

#if EXTI_ISR_TEST_ENABLE
/*
PA3 as log output
pin mapping
8016H
PD6  PMU exti input
PD7  PMU exti input
*/
#include "os_timer.h"
#include "driver_pmu.h"
#include "sys_utils.h"
static os_timer_t button_anti_shake_timer;
static uint32_t curr_button_before_anti_shake = 0;
static void button_anti_shake_timeout_handler(void *param)
{
    uint32_t pmu_int_pin_setting = ool_read32(PMU_REG_PORTA_TRIG_MASK);
    uint32_t gpio_value = ool_read32(PMU_REG_GPIOA_V);
    gpio_value &= pmu_int_pin_setting;
    gpio_value ^= pmu_int_pin_setting;

    if(gpio_value == curr_button_before_anti_shake)
    {
        gpio_value &= ~GPIO_PC4;    //ignore PC4, because of inner usage in lib
        if(gpio_value == 0)
        {
            co_printf("L\r\n");
        }
        else
        {
            co_printf("K:%08X\r\n",gpio_value);
        }
    }
}
__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
    uint32_t pmu_int_pin_setting = ool_read32(PMU_REG_PORTA_TRIG_MASK);
    uint32_t gpio_value = ool_read32(PMU_REG_GPIOA_V);

    ool_write32(PMU_REG_PORTA_LAST, gpio_value);
    uint32_t tmp = gpio_value & pmu_int_pin_setting;
    curr_button_before_anti_shake = tmp^pmu_int_pin_setting;
    os_timer_start(&button_anti_shake_timer, 10, false);
}

void demo_pmu_exti(void)
{
    co_printf("pmu exti isr\r\n");
    os_timer_init(&button_anti_shake_timer, button_anti_shake_timeout_handler, NULL);
    pmu_set_pin_pull(GPIO_PORT_D, BIT(6)|BIT(7), true);        //PD6 low voltage ->isr
    //pmu_set_pin_pull(GPIO_PORT_D, BIT(7), false);     //PD7 high voltage ->isr
    pmu_port_wakeup_func_set(GPIO_PD6|GPIO_PD7);    //PD7 pin should connect to GND with 4.7K resistor
}

/*
PA3 as log output
pin mapping
8016H
PD4  digital exti input
*/
#include "driver_exti.h"
#include "driver_system.h"
#include "driver_gpio.h"
static enum ext_int_type_t exti_type ;
__attribute__((section("ram_code"))) void exti_isr_ram(void)
{
    uint32_t exti_src;

    exti_src = ext_int_get_src();
    ext_int_clear(exti_src);
    co_printf("exti_key:%x\r\n",exti_src);
    if( exti_src & BIT(EXTI_12) )
    {
        if(exti_type == EXT_INT_TYPE_LOW)
            exti_type = EXT_INT_TYPE_HIGH;
        else if(exti_type == EXT_INT_TYPE_HIGH)
            exti_type = EXT_INT_TYPE_LOW;
        ext_int_set_type(EXTI_12, exti_type);
    }
    else if( exti_src & BIT(EXTI_13) )
    {


    }
}

void demo_digital_exti(void)
{
    co_printf("digital exti isr\r\n");

    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_D4);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_4, GPIO_DIR_IN);
    system_set_port_pull( GPIO_PD4, true);
    ext_int_set_port_mux(EXTI_12,EXTI_12_PD4);
    ext_int_set_type(EXTI_12, EXT_INT_TYPE_LOW);
    ext_int_set_control(EXTI_12, 1000, 4);
    ext_int_enable(EXTI_12);

    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_5, PORTD5_FUNC_D5);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_5, GPIO_DIR_IN);
    system_set_port_pull( GPIO_PD5, true);
    ext_int_set_port_mux(EXTI_13,EXTI_13_PD5);
    ext_int_set_type(EXTI_13, EXT_INT_TYPE_NEG);
    ext_int_set_control(EXTI_13, 1000, 4);
    ext_int_enable(EXTI_13);

    exti_type = EXT_INT_TYPE_LOW;
    NVIC_SetPriority(EXTI_IRQn, 4);
    NVIC_EnableIRQ(EXTI_IRQn);
}
void demo_exti_isr(void)
{
    co_printf("exti isr demo\r\n");
    demo_pmu_exti();
    demo_digital_exti();
}
#endif

#if LED_TEST_ENABLE
#include "driver_pmu.h"
#include "driver_pmu_pwm.h"

void demo_led(void)
{
    co_printf("led2 demo\r\n");
    pmu_set_led2_value(1);
    co_delay_100us(5000);
    pmu_set_led2_value(0);
    co_delay_100us(5000);
    pmu_set_led2_value(1);
    co_delay_100us(5000);
    pmu_set_led2_value(0);
    co_delay_100us(5000);
    pmu_set_led2_value(1);


//configure led2 pin as pmu_pwm2 output
    /*
        co_printf("led2 as pwm demo\r\n");
        pmu_set_led2_as_pwm();
        pmu_pwm_init();
        pmu_pwm_set_param(GPIO_PORT_A,GPIO_BIT_2,15625,15625);
        pmu_pwm_start(GPIO_PORT_A,GPIO_BIT_2,1,0);
    */
}
#endif

#if SSP_MASTER_TEST_ENABLE
#include "driver_ssp.h"
#include "driver_system.h"
#include "driver_iomux.h"
#include "driver_plf.h"

/*
PA3 as log output
pin mapping
8016H   slave ssp
PA4     CLK
PA5     CSn
PA6     D_In
PA7     D_Out
GND     GND
*/
void demo_ssp_master(void)
{
#define SSP_MASTER_TX 1
#define SSP_MASTER_RX 0
    co_printf("ssp_master demo\r\n");

    uint8_t buff[64] = {0};
    uint8_t dummy[32] = {0};
    uint8_t i = 0;
    memset(dummy,0xff,32);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_SSP0_CLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_SSP0_CSN);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_SSP0_DOUT);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_SSP0_DIN);

    ssp_init_(8,SSP_FRAME_MOTO, SSP_MASTER_MODE,3000000,2,NULL);
    while(1)
    {
#if SSP_MASTER_TX
/*
        memset(buff,i,32);
        ssp_send_bytes(buff,32);
        i++;
*/
          ssp_send_byte(0xff);
          ssp_wait_busy_bit(SSP0_BASE);
        for(uint16_t k=0; k<1024; k++)
        {
            for(uint8_t j=0; j<=0xff; j++)
            {
                //buff[j] = i+j;
                //ssp_send_bytes(buff,32);
                ssp_send_byte((j)&0xff);
                ssp_wait_busy_bit(SSP0_BASE);
                if(j==0xff)
                    break;
            }
        }
        co_printf("master_tx:%d\r\n",i);
        while(1);
        
        co_printf("master_tx:%d\r\n",i);
#endif
#if SSP_MASTER_RX
        ssp_send_bytes(dummy,32);   //send dummy 0xff data to produce clk,and recv data to fifo
        ssp_get_data_(buff,32);     //get data from fifo
        show_reg(buff,32,1);
#endif
        co_delay_100us(1000);
    }
}
#endif

void peripheral_demo(void)
{
#if (UART0_DEMO_ENABLE)
    demo_uart0();
#endif

#if (SPI_FLASH_DEMO_ENABLE)
    demo_spi_flash();
#endif

#if (FLASH_DEMO_ENABLE)
    demo_flash();
#endif

#if (HWTIM_DEMO_ENABLE)

    demo_timer();
#endif

#if (ADC_DEMO_ENABLE)
    demo_adc();
#endif

#if (PWM_DEMO_ENABLE)
    demo_pwm();
#endif

#if PMU_PWM_DEMO_ENABLE
    demo_pmu_pwm();
#endif

#if (LVD_DEMO_ENABLE)
    demo_lvd();
#endif

#if (WDT_DEMO_ENABLE)
    demo_wdt();
#endif

#if KEYSCAN_TEST_ENABLE
    demo_keyscan();
#endif

#if PMU_QDEC_DEMO_ENABLE
    demo_pmu_qdec();
#endif

#if GPIO_TEST_ENABLE
    demo_gpio();
#endif

#if EXTI_ISR_TEST_ENABLE
    demo_exti_isr();
#endif

#if LED_TEST_ENABLE
    demo_led();
#endif

#if RTC_TEST_ENABLE
    //demo_rtc();
    demo_rtc_start_timer();
#endif

#if (I2C_DEMO_ENABLE)
    demo_i2c();
#endif

#if SSP_MASTER_TEST_ENABLE
    demo_ssp_master();
#endif

}



