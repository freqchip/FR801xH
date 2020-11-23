#ifndef _DEMO_PERIPHERAL_H
#define _DEMO_PERIPHERAL_H

#define UART0_DEMO_ENABLE (0)
//for inner flash test through inner Qspi interface
#define FLASH_DEMO_ENABLE (0)
//For exteral spi flash test through ssp interface
#define SPI_FLASH_DEMO_ENABLE (0)
#define HWTIM_DEMO_ENABLE (0)
#define PWM_DEMO_ENABLE (0)
#define PMU_PWM_DEMO_ENABLE (0)
#define KEYSCAN_TEST_ENABLE (0)
#define RTC_TEST_ENABLE (0)
#define WDT_DEMO_ENABLE (0)
#define LED_TEST_ENABLE (0)
#define GPIO_TEST_ENABLE (0)
#define EXTI_ISR_TEST_ENABLE (0)
#define PMU_QDEC_DEMO_ENABLE (0)
#define I2C_DEMO_ENABLE (0)



//for SSP work as master test
#define SSP_MASTER_TEST_ENABLE (1)

//NOTE: 8010H can't work as SSP slaver 
//#define SSP_SLAVE_TEST_ENABLE (0)

//#define ADC_DEMO_ENABLE (0)
//#define LVD_DEMO_ENABLE (0)

void peripheral_demo(void);

#endif  //_DEMO_PERIPHERAL_H
