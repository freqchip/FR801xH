#include <stdio.h>
#include <stdint.h>

#include "co_printf.h"
#include "driver_wdt.h"
#include "driver_plf.h"
//#include "apb2spi.h"
//#include "uart.h"

extern void platform_reset_patch(uint32_t error);
__attribute__((section("ram_code"))) void HardFault_Handler_C(unsigned int* hardfault_args)
{
    co_delay_100us(500);
    co_printf("Crash, dump regs:\r\n");
    co_printf("PC    = 0x%08X\r\n",hardfault_args[6]);
    co_printf("LR    = 0x%08X\r\n",hardfault_args[5]);
#if 0
    co_printf("R0    = 0x%08X\r\n",hardfault_args[0]);
    co_printf("R1    = 0x%08X\r\n",hardfault_args[1]);
    co_printf("R2    = 0x%08X\r\n",hardfault_args[2]);
    co_printf("R3    = 0x%08X\r\n",hardfault_args[3]);
    co_printf("R12   = 0x%08X\r\n",hardfault_args[4]);
    co_printf("PSR   = 0x%08X\r\n",hardfault_args[7]);
    co_printf("BFAR  = 0x%08X\r\n",*(unsigned int*)0xE000ED38);
    co_printf("CFSR  = 0x%08X\r\n",*(unsigned int*)0xE000ED28);
    co_printf("HFSR  = 0x%08X\r\n",*(unsigned int*)0xE000ED2C);
    co_printf("DFSR  = 0x%08X\r\n",*(unsigned int*)0xE000ED30);
    co_printf("AFSR  = 0x%08X\r\n",*(unsigned int*)0xE000ED3C);
    co_printf("SHCSR = 0x%08X\r\n",SCB->SHCSR);
    co_printf("dump sp stack[sp sp-512]:\r\n");
    uint16_t i = 0;
    do
    {
        co_printf("0x%08X,",*(hardfault_args++));
        i++;
        if(i%4 == 0)
            co_printf("\r\n");
    }
    while(i<128);
#endif

#ifdef USER_MEM_API_ENABLE
    show_ke_malloc();
#endif
#if 0

    #include "flash.h"
    uint8_t tmp[12] = {0x71,0x72,63,14,15,16,17,18,19,0x1A,0x1B,0x7C};
    flash_erase(USER_FLASH_BASE_ADDR, 0);
    flash_write(USER_FLASH_BASE_ADDR,12, &tmp[0]);
#endif

    //store_reset_info(RST_CAUSE_CRASH);
    platform_reset_patch(0);
    while(1);
}


