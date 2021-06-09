/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"

#include "os_timer.h"
#include "os_mem.h"
#include "sys_utils.h"
#include "jump_table.h"

#include "driver_system.h"
#include "driver_uart.h"
#include "driver_pmu.h"
#include "demo_peripheral.h"

const struct jump_table_version_t _jump_table_version __attribute__((section("jump_table_3"))) =
{
    .firmware_version = 0x00000000,
};

const struct jump_table_image_t _jump_table_image __attribute__((section("jump_table_1"))) =
{
    .image_type = IMAGE_TYPE_APP,
    .image_size = 0x19000,
};

void user_custom_parameters(void)
{
    __jump_table.addr.addr[0] = 0x01;
    __jump_table.addr.addr[1] = 0x01;
    __jump_table.addr.addr[2] = 0x01;
    __jump_table.addr.addr[3] = 0x01;
    __jump_table.addr.addr[4] = 0x01;
    __jump_table.addr.addr[5] = 0xc1;
    __jump_table.system_clk = SYSTEM_SYS_CLK_48M;
   // __jump_table.system_option &= ~(SYSTEM_OPTION_PRINT_UART1);   //no log
}

__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
    uart_putc_noint_no_wait(UART1, 's');
}

__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
    /* set PA2 and PA3 for AT command interface */
    system_set_port_pull(GPIO_PA2, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);

    uart_init(UART1, BAUD_RATE_115200);
    NVIC_EnableIRQ(UART1_IRQn);

    uart_putc_noint_no_wait(UART1, 'w');
    NVIC_EnableIRQ(PMU_IRQn);
}

void user_entry_before_ble_init(void)
{
    /* set system power supply in BUCK mode */
    pmu_set_sys_power_mode(PMU_SYS_POW_BUCK);

    pmu_enable_irq(PMU_ISR_BIT_ACOK
                   | PMU_ISR_BIT_ACOFF
                   | PMU_ISR_BIT_ONKEY_PO
                   | PMU_ISR_BIT_OTP
                   | PMU_ISR_BIT_LVD
                   | PMU_ISR_BIT_BAT
                   | PMU_ISR_BIT_ONKEY_HIGH);
    NVIC_EnableIRQ(PMU_IRQn);

    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_UART1_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_UART1_TXD);
    uart_init(UART1, BAUD_RATE_115200);

    ool_write(PMU_REG_ADKEY_ALDO_CTRL, ool_read(PMU_REG_ADKEY_ALDO_CTRL) & (~(1<<3)));
    //ool_write(PMU_REG_ADKEY_ALDO_CTRL, (ool_read(PMU_REG_ADKEY_ALDO_CTRL)&0x0F));
    //pmu_set_aldo_voltage(PMU_ALDO_MODE_NORMAL,PMU_ALDO_VOL_2_1);
}

void user_entry_after_ble_init(void)
{
    co_printf("user_entry_after_ble_init\r\n");

#if 1
//stop sleep
    system_sleep_disable();
#else
    if(__jump_table.system_option & SYSTEM_OPTION_SLEEP_ENABLE)
    {
        co_printf("\r\na");
        co_delay_100us(10000);       //must keep it, or pressing reset key may block .
        co_printf("\r\nb");
        co_delay_100us(10000);
        co_printf("\r\nc");
        co_delay_100us(10000);
        co_printf("\r\nd");
    }
#endif


    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = 1600;
    adv_param.adv_intv_max = 1600;
    gap_set_advertising_param(&adv_param);

    uint8_t adv_data[]="\x09\x08\x46\x52\x38\x30\x31\x30\x48\x00";
    uint8_t rsp_data[]="\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x45";
    gap_set_advertising_data(adv_data,sizeof(adv_data) -1 );
    gap_set_advertising_rsp_data(rsp_data,sizeof(rsp_data) -1 );
#if 0
    gap_start_advertising(0);
#endif

    peripheral_demo();
}


