/***********************************************************************************************************************************
**注：目前该例程只能工作在48MHz主频下
*************************************************************************************************************************************/


#include "driver_ir_send.h"
#include "driver_pwm.h"
#include "driver_plf.h"
#include "driver_system.h"
#include "driver_timer.h"
#include "driver_pmu.h"
#include "sys_utils.h"
#include "driver_exti.h"
#include "driver_gpio.h"
#include <math.h>
#include <string.h>
#include "os_mem.h"
#include "os_task.h"
#include "os_msg_q.h"

#include "co_printf.h"
//#include "user_task.h"


#define IR_DBG     FR_DBG_OFF
#define IR_LOG     FR_LOG(IR_DBG)


#define IR_LEARN_PIN_INIT()      do{ \
                                pmu_set_port_mux(GPIO_PORT_C,GPIO_BIT_7,PMU_PORT_MUX_GPIO); \
                                pmu_set_pin_to_PMU(GPIO_PORT_C , BIT(7) ); \
                                pmu_set_pin_dir(GPIO_PORT_C, BIT(7), GPIO_DIR_OUT);  \
                                pmu_set_pin_pull(GPIO_PORT_C, BIT(7), false);    \
                                }while(0)
#define IR_LEARN_DISENABLE()   pmu_set_gpio_value(GPIO_PORT_C, BIT(7), 1)
#define IR_LEARN_ENABLE()     pmu_set_gpio_value(GPIO_PORT_C,  BIT(7), 0)

uint16_t user_ir_task_id = TASK_ID_NONE;
static uint8_t ir_mode = IR_SEND;
TYPEDEFIRPWMTIM IR_PWM_TIM= {0};
TYPEDEFIRLEARNDATA ir_learn_data= {0};

static TYPEDEFIRLEARN *ir_learn  = NULL;
static uint8_t ir_data_check(void);
static uint8_t sys_clk;
static uint8_t sys_clk_cfg;


struct pwm_ctrl_t
{
    uint32_t en:1;
    uint32_t reserved0:2;
    uint32_t out_en:1;
    uint32_t single:1;
    uint32_t reservaed1:27;
};

struct pwm_elt_t
{
    uint32_t cur_cnt;
    uint32_t high_cnt;
    uint32_t total_cnt;
    struct pwm_ctrl_t ctrl;
};

struct pwm_regs_t
{
    struct pwm_elt_t channel[PWM_CHANNEL_MAX];
};

static struct pwm_regs_t *pwm_ctrl = (struct pwm_regs_t *)PWM_BASE;


struct timer_lvr_t
{
    uint32_t load: 16;
    uint32_t unused: 16;
};

struct timer_cvr_t
{
    uint32_t count: 16;
    uint32_t unused: 16;
};

struct timer_cr_t
{
    uint32_t reserved1: 2;
    uint32_t pselect: 2;
    uint32_t reserved2: 2;
    uint32_t count_mode: 1;
    uint32_t count_enable: 1;
    uint32_t unused: 24;
};

struct timer_icr_t
{
    uint32_t data: 16;
    uint32_t unused: 16;
};

struct timer
{
    struct timer_lvr_t load_value;
    struct timer_cvr_t count_value;
    struct timer_cr_t control;
    struct timer_icr_t interrupt_clear;
};

volatile struct timer *timerp_ir = (volatile struct timer *)TIMER0;


/*********************************************************************
 * @fn      IR_decode
 *
 * @brief   The infrared transmission data is converted into multiple square wave intervals
 *
 *
 *
 *
 * @param   ir_data         - Pointer to the infrared data that needs to be sent
 *          data_size       - The amount of infrared data to be sent
 *          IR_Send_struct  - The structure that holds the decoded data
 * @return  None.
 */
void IR_decode(uint8_t *ir_data,uint8_t data_size,TYPEDEFIRPWMTIM *IR_Send_struct)
{
    uint8_t i=0,j=0;
    //memset(&IR_PWM_TIM,0,sizeof(IR_PWM_TIM));
    //IR_PWM_TIM.IR_Busy =true;
    if((data_size*2+4) > LERANDATACNTMAX)
    {
        co_printf("data_size oversize!\r\n ");
        return ;
    }
    IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRLEADLOW;
    IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRLEADHIGHT;
    for(i=0; i<data_size; i++)
    {
        for(j=0; j<8; j++)
        {
            if((ir_data[i]>>j) & 0x01)
            {
                IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRLOGIC1LOW;
                IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRLOGIC1HIGHT;
            }
            else
            {
                IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRLOGIC0LOW;
                IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRLOGIC0HIGHT;
            }
        }
    }
    IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRSTOPLOW;
    IR_Send_struct->IR_Pwm_State_Date[IR_Send_struct->IR_pwm_Num++] = IRSTOPHIGHT;
}

/*********************************************************************
 * @fn      IR_start_send
 *
 * @brief   Start infrared transmission
 *
 * @param   IR_Send_struct  - This structure contains parameters related to infrared transmission
 *
 * @return  None.
 */

void IR_start_send(TYPEDEFIRPWMTIM *IR_Send_struct)
{
    if(((ir_learn_data.IR_learn_state & BIT(1)) == BIT(1)))
    {
        co_printf("ir_learn busy!\r\n");
        return;
    }
    if(IR_PWM_TIM.IR_Busy)
    {
        co_printf("IR send busy!\r\n");

        return;
    }

    memcpy(&IR_PWM_TIM,IR_Send_struct,sizeof(TYPEDEFIRPWMTIM));
    IR_PWM_TIM.IR_Busy =true;
    IR_PWM_TIM.IR_pwm_SendNum = 0;//已发送间隔数量  ，清零
#if IR_SLEEP_EN
    system_sleep_disable();
#endif
    ir_mode = IR_SEND;
    IR_LOG("IR_Busy:%d ir_carrier_fre:%d IR_pwm_SendNum:%d IR_pwm_state:%d\r\n",IR_PWM_TIM.IR_Busy,IR_PWM_TIM.ir_carrier_fre,IR_PWM_TIM.IR_pwm_SendNum,IR_PWM_TIM.IR_pwm_state);
    IR_LOG("IR Send data: IR_pwm_Num%d\r\n",IR_PWM_TIM.IR_pwm_Num);
//  uint16_t short_data_cnt = 0;
    for(uint8_t i=0; i < IR_PWM_TIM.IR_pwm_Num; i++)
    {
        IR_LOG("[%d %d],",IR_PWM_TIM.IR_Pwm_State_Date[i],i);
    }
    IR_LOG("IR Send start\r\n");
    IR_PWM_TIM.total_count = system_get_pclk() / IR_PWM_TIM.ir_carrier_fre;
    IR_PWM_TIM.high_count_half = IR_PWM_TIM.total_count * (100-50) / 100;//50%duty
    timer_init(TIMER0,IR_PWM_TIM.IR_Pwm_State_Date[IR_PWM_TIM.IR_pwm_SendNum++],TIMER_PERIODIC);
    NVIC_EnableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 2);


    pmu_set_pin_to_CPU(GPIO_PORT_C,(1<<GPIO_BIT_5));
    system_set_port_mux(GPIO_PORT_C,GPIO_BIT_5,PORTC5_FUNC_PWM5);
    pwm_init(PWM_CHANNEL_5, IR_PWM_TIM.ir_carrier_fre, 50);
    pwm_start(PWM_CHANNEL_5);
    timer_run(TIMER0);


}

/*********************************************************************
 * @fn      IR_stop_send
 *
 * @brief   For loop sending, Stop the loop sending infrared
 *
 * @param   None
 *
 * @return  None.
 */
void IR_stop_send(void)
{
    IR_PWM_TIM.loop =false;
}
/********************************************timer_init_count_us_reload*********************************************************/
__attribute__((section("ram_code")))uint8_t timer_init_count_us_reload(uint32_t timer_addr, uint32_t count_us)
{
//    uint8_t sysclk;
    uint16_t count;
    uint8_t prescale;

    if(count_us < 100)
    {
        return false;
    }
    const uint32_t timer_max_period[][TIMER_PRESCALE_MAX] =
    {
        1365, 21800, 349000,    // 48M
        2730, 43600, 699000,    // 24M
        5461, 87300, 1398000,   // 12M
        10922, 174700, 2796000
    };// 6M
    if(timer_max_period[sys_clk_cfg][TIMER_PRESCALE_256] < count_us)
    {
        count = (timer_max_period[sys_clk_cfg][TIMER_PRESCALE_256] * sys_clk) >> 8;
        prescale = TIMER_PRESCALE_256;
    }
    else if(timer_max_period[sys_clk_cfg][TIMER_PRESCALE_16] < count_us)
    {
        count = (count_us * sys_clk) >> 8;
        prescale = TIMER_PRESCALE_256;

    }
    else if(timer_max_period[sys_clk_cfg][TIMER_PRESCALE_1] < count_us)
    {
        count = (count_us * sys_clk) >> 4;
        prescale = TIMER_PRESCALE_16;
    }
    else
    {
        count = count_us*sys_clk;
        prescale = TIMER_PRESCALE_1;
    }

    volatile struct timer *timerp = (volatile struct timer *)timer_addr;

    //timerp->control.count_enable = 0;
    //timerp->control.count_mode = run_mode;
    timerp->control.pselect = prescale;
    timerp->interrupt_clear.data = 0x01;
    timerp->load_value.load = count;
    //timerp->control.count_enable = 1;

    return true;
}

/*********************************************************************
 * @fn      timer0_isr_ram
 *
 * @brief   timer0 Interrupt handler
 *
 *
 *
 *
 * @param   None
 *
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void timer0_isr_ram(void)
{
    uint32_t ir_carrier_cycle = 0;
    uint16_t i=0;
    timer_clear_interrupt(TIMER0);

    if(ir_mode == IR_SEND)
    {
        if(IR_PWM_TIM.IR_pwm_SendNum < IR_PWM_TIM.IR_pwm_Num)
        {
            timer_init_count_us_reload(TIMER0,IR_PWM_TIM.IR_Pwm_State_Date[IR_PWM_TIM.IR_pwm_SendNum++]);
            IR_PWM_TIM.IR_pwm_state =!IR_PWM_TIM.IR_pwm_state;
            //GLOBAL_INT_DISABLE();
            NVIC_DisableIRQ(BLE_IRQn);
            if(!IR_PWM_TIM.IR_pwm_state)
            {
                //pwm_update(PWM_CHANNEL_5, IR_PWM_TIM.ir_carrier_fre, 50);
                pwm_ctrl->channel[PWM_CHANNEL_5].total_cnt = IR_PWM_TIM.total_count;
                pwm_ctrl->channel[PWM_CHANNEL_5].high_cnt = IR_PWM_TIM.high_count_half;
            }
            else
            {
                //pwm_update(PWM_CHANNEL_5, IR_PWM_TIM.ir_carrier_fre, 0);
                pwm_ctrl->channel[PWM_CHANNEL_5].total_cnt = IR_PWM_TIM.total_count;
                pwm_ctrl->channel[PWM_CHANNEL_5].high_cnt = IR_PWM_TIM.total_count;
            }
            NVIC_EnableIRQ(BLE_IRQn);
            //GLOBAL_INT_RESTORE();
        }
        else if(IR_PWM_TIM.loop)  //return send
        {
            IR_PWM_TIM.IR_pwm_SendNum = 0;
            IR_PWM_TIM.IR_pwm_state = 0;
            timer_init_count_us_reload(TIMER0,IR_PWM_TIM.IR_Pwm_State_Date[IR_PWM_TIM.IR_pwm_SendNum++]);
            //pwm_update(PWM_CHANNEL_5, IR_PWM_TIM.ir_carrier_fre, 50);
            //GLOBAL_INT_DISABLE();
            NVIC_DisableIRQ(BLE_IRQn);
            pwm_ctrl->channel[PWM_CHANNEL_5].total_cnt = IR_PWM_TIM.total_count;
            pwm_ctrl->channel[PWM_CHANNEL_5].high_cnt = IR_PWM_TIM.high_count_half;
            NVIC_EnableIRQ(BLE_IRQn);
            //GLOBAL_INT_RESTORE();
        }
        else  //结束后由PMU控制将该引脚拉低
        {
            /*
            pmu_set_pin_to_PMU(GPIO_PORT_C,(1<<GPIO_BIT_5));
            pmu_set_port_mux(GPIO_PORT_C,GPIO_BIT_5,PMU_PORT_MUX_GPIO);
            pmu_set_pin_dir(GPIO_PORT_C,BIT(5),GPIO_DIR_IN);
            //pmu_set_gpio_value(GPIO_PORT_C, (1<<GPIO_BIT_5), 0);
            IR_LOG("IR Send End \r\n");
            IR_PWM_TIM.IR_Busy = false;
            timer_stop(TIMER0);
            NVIC_DisableIRQ(TIMER0_IRQn);
            //NVIC_SetPriority(TIMER0_IRQn, 2);
            #if IR_SLEEP_EN
                system_sleep_enable();
            #endif
            */
            os_event_t ir_event;

            ir_event.event_id = 0;
            ir_event.src_task_id = TASK_ID_NONE;
            ir_event.param = NULL;
            ir_event.param_len = 0;

            os_msg_post(user_ir_task_id, &ir_event);
        }
    }
    else if(ir_mode == IR_LEARN)  //学习模式
    {
        if(ir_learn == NULL)
        {
            co_printf("ir_learn malloc error!\r\n");
            return;
        }

        switch(ir_learn->ir_learn_step)
        {

            case IR_WAIT_STOP://100ms以上无波动，进入获取模式
                IR_LOG("T");
                ir_learn->ir_timer_cnt ++;
                if(ir_learn->ir_timer_cnt > 200) //超过200ms,开始等待中断学习
                {
                    ir_learn->ir_learn_step = IR_LEARN_GET_DATA;
                    co_printf("ir_learn->ir_learn_step = IR_LEARN_GET_DATA;\r\n");
                    ir_learn->ir_learn_start = 0;

                }
                break;
            case IR_LEARN_GET_DATA:
                ir_learn->ir_timer_cnt ++;
                if((ir_learn->ir_timer_cnt >= 300)&& ir_learn->ir_learn_start) //300ms结束标志
                {
                    ir_learn->ir_timer_cnt = 0;

                    IR_LOG("IR learn stop  ir_learn_cnt= %d\r\n",ir_learn->ir_learn_data_buf_cnt);

                    uint32_t ir_carrier_cycle_data_sum=0;
                    uint8_t ir_carrier_cycle_data_cnt=0;
                    for(i = 0; i < 100; i++)
                    {
                        //IR_LOG("[%d %d] ",i,ir_learn->ir_learn_data_buf[i]);
                        if((ir_learn->ir_learn_data_buf[i] > 700)&&(ir_learn->ir_learn_data_buf[i] < 1429)) //36k:1333  38k:1263  56k:857
                        {
                            ir_carrier_cycle_data_sum +=ir_learn->ir_learn_data_buf[i];
                            ir_carrier_cycle_data_cnt++;
                        }
                    }
                    ir_carrier_cycle_data_sum /= ir_carrier_cycle_data_cnt;
                    ir_learn->ir_carrier_fre = 48000000/ir_carrier_cycle_data_sum;
                    IR_LOG("ir_learn->ir_carrier_fre =%d Hz  \r\n",ir_learn->ir_carrier_fre);
                    if(ir_learn->ir_carrier_fre > (IR_CF_56K-1000))
                    {
                        ir_learn->ir_carrier_fre = IR_CF_56K;
                    }
                    else if(ir_learn->ir_carrier_fre > (IR_CF_38K-1000))
                    {
                        ir_learn->ir_carrier_fre = IR_CF_38K;
                    }
                    else if(ir_learn->ir_carrier_fre > (IR_CF_36K-1000))
                    {
                        ir_learn->ir_carrier_fre = IR_CF_36K;
                    }
                    IR_LOG("ir_learn->ir_carrier_fre =%d Hz  \r\n",ir_learn->ir_carrier_fre);
                    ir_learn_data.ir_carrier_fre = ir_learn->ir_carrier_fre;
                    for(i =0; i < ir_learn->ir_learn_data_buf_cnt; i++)
                    {
                        if((ir_learn->ir_learn_data_buf[i] > 700)&&(ir_learn->ir_learn_data_buf[i] < 1429) /*abs((int)ir_learn->ir_learn_data_buf[i]-ir_learn->ir_carrier_cycle)<2*/) //误差约4us//这里可以计算载波
                        {
                            ir_learn->ir_carrier_times ++;
                        }
                        else if(ir_learn->ir_learn_data_buf[i] > 9600)  //大于200us  ，200*48=9600
                        {
                            if(ir_learn->ir_learn_data_cnt < (LERANDATACNTMAX-1))
                            {
                                ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt++] = (ir_learn->ir_carrier_times+0.5)*1000000/ir_learn->ir_carrier_fre;
                                ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt++] = (ir_learn->ir_learn_data_buf[i]>>16)*1000+(ir_learn->ir_learn_data_buf[i]&0xffff)/48-0.5*1000000/ir_learn->ir_carrier_fre;
                            }
                            //IR_LOG("TV:%d  CT:%d ",ir_learn->ir_learn_data_buf[i],ir_learn->ir_carrier_times);
                            ir_learn->ir_carrier_times = 0;
                            //ir_learn->ir_timer_cnt = 0;
                        }
                    }
                    if((ir_learn->ir_learn_data_cnt < LERANDATACNTMAX)&&(ir_learn->ir_carrier_times>0)) //>10才是正常的间隔长度   ，这个需要补最后的一位是
                    {

                        ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt++] = (ir_learn->ir_carrier_times+0.5)*1000000/ir_learn->ir_carrier_fre;
                        IR_LOG("ir_learn_Date[%d] = %d\r\n",ir_learn->ir_learn_data_cnt-1,ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt-1]);
                    }
                    if(/*(ir_learn->ir_learn_data_buf_cnt <LERANDATABUFMAX)&&*/(ir_learn->ir_learn_data_cnt < LERANDATACNTMAX)) //补个long stop bit
                    {
                        ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt++] = 100000;
                        IR_LOG("ir_learn_Date[%d] = %d\r\n",ir_learn->ir_learn_data_cnt-1,ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt-1]);
                    }
                    else
                    {
                        ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt-1] = 100000;
                        IR_LOG("ir_learn_Date[%d] = %d\r\n",ir_learn->ir_learn_data_cnt-1,ir_learn->ir_learn_Date[ir_learn->ir_learn_data_cnt-1]);
                    }
                    IR_LOG("ir_learn_data_cnt %d\r\n",ir_learn->ir_learn_data_cnt);
                    for(i =0; i < ir_learn->ir_learn_data_cnt; i++)
                    {
                        IR_LOG("[%d %d] ",i,ir_learn->ir_learn_Date[i]);
                        if(i%10 == 9)
                            IR_LOG("\r\n");
                    }
                    IR_LOG("\r\n");
                    uint8_t IR_learn_state =ir_data_check();//判断红外码是否有效


                    //如果红外码无效，则继续学习
                    //if(ir_learn_data.IR_learn_state == 0){
                    if(IR_learn_state == true)
                    {
                        //GLOBAL_INT_DISABLE();
                        timer_stop(TIMER0);
                        ext_int_disable(EXTI_6);
                        NVIC_DisableIRQ(TIMER0_IRQn);
                        ir_learn_data.IR_learn_state |= BIT(0);
                        ir_learn_data.ir_learn_data_cnt = ir_learn->ir_learn_data_cnt;
                        memcpy(ir_learn_data.ir_learn_Date,ir_learn->ir_learn_Date,ir_learn->ir_learn_data_cnt*sizeof(uint32_t));
                        //GLOBAL_INT_RESTORE();
                        IR_LOG("ir learn success one times!  ir_learn_data_cnt = %d\r\n",ir_learn_data.ir_learn_data_cnt);
                    }
                    else
                    {
                        IR_LOG("ir learn failed!,again!\r\n");
                        ir_learn->ir_learn_step = IR_WAIT_STOP;
                    }
                    //  }

                    IR_LOG("*****************************************\r\n");

                    //memset(&ir_learn,0,sizeof(TYPEDEFIRLEARN));
                }
                break;
        }
    }
}



#define IR_EXTI_DBG     FR_DBG_OFF
#define IR_EXTI_LOG     FR_LOG(IR_EXTI_DBG)

/*********************************************************************
 * @fn      exti_isr_ram
 *
 * @brief   External interrupt handler
 *
 *
 *
 *
 * @param   None
 *
 *
 * @return  None.
 */
volatile struct ext_int_t *const ext_int_reg_ir = (struct ext_int_t *)EXTI_BASE;

__attribute__((section("ram_code"))) void exti_isr_ram(void)
{
    uint32_t status,timer_value =0;
    status = ext_int_reg_ir->ext_int_status;
    ext_int_reg_ir->ext_int_status = status;
    //status = ext_int_get_src();
    //ext_int_clear(status);
    if((status>>EXTI_6) &0x01)
    {
        //IR_LOG("I");
        //IR_EXTI_LOG("I");
        if(ir_learn == NULL)
        {
            co_printf("ir_learn malloc error!\r\n");
            return;
        }
        switch(ir_learn->ir_learn_step)
        {
            case IR_WAIT_STOP://
                IR_EXTI_LOG("P");
                ir_learn->ir_timer_cnt = 0;
                //timer_reload(TIMER0);
                timerp_ir->load_value.load = timerp_ir->load_value.load;
                break;
            case IR_LEARN_GET_DATA:
                IR_EXTI_LOG("D");
                timer_value =48000 - timerp_ir->count_value.count;//timer_get_current_value(TIMER0);//48000 = 1000 * 48 /1  分频：1
                //timer_reload(TIMER0);
                timerp_ir->load_value.load = timerp_ir->load_value.load;

                if(ir_learn->ir_learn_start == 0)
                {
                    ir_learn->ir_learn_start = 1;
                    ir_learn->ir_carrier_times =0;
                    ir_learn->ir_timer_cnt = 0;
                    ir_learn->ir_learn_data_buf_cnt = 0;
                    ir_learn->ir_learn_data_cnt = 0;
                    ir_learn_data.ir_learn_data_cnt =0;
                    IR_EXTI_LOG("S");
                }
                else
                {

                    if(ir_learn->ir_learn_data_buf_cnt < LERANDATABUFMAX)
                    {
                        ir_learn->ir_learn_data_buf[ir_learn->ir_learn_data_buf_cnt++] =  (ir_learn->ir_timer_cnt<<16)|timer_value;
                        ir_learn->ir_timer_cnt = 0;
                    }
                }
                break;
        }
        //timer_reload(TIMER0);

        //IR_LOG("ir_timer_cnt=%d\r\n",ir_timer_cnt);
    }
}


/*********************************************************************
 * @fn      IR_start_learn
 *
 * @brief   Start the infrared learning
 *
 *
 *
 *
 * @param   None
 *
 *
 * @return  None.
 */
uint8_t IR_start_learn(void)
{
    if(((ir_learn_data.IR_learn_state & BIT(1)) == BIT(1)))
    {
        co_printf("ir_learn busy!\r\n");
        return false;
    }
    ir_learn = os_zalloc(sizeof(TYPEDEFIRLEARN));
    if(ir_learn == NULL)
    {
        co_printf("ir_learn malloc error!\r\n");
        return false;
    }
    ir_mode = IR_LEARN;
    memset(&ir_learn_data,0,sizeof(ir_learn_data));
    ir_learn_data.IR_learn_state |= BIT(1);//开始学习
#if IR_SLEEP_EN
    system_sleep_disable();
#endif

    pmu_set_pin_to_CPU(GPIO_PORT_C, BIT(6) );
    system_set_port_mux(GPIO_PORT_C,GPIO_BIT_6,PORTC6_FUNC_C6);
    gpio_set_dir(GPIO_PORT_C,GPIO_BIT_6,GPIO_DIR_IN);
    system_set_port_pull(GPIO_PC6, true);
    ext_int_set_port_mux(EXTI_6,EXTI_6_PC6);
    ext_int_set_type(EXTI_6, EXT_INT_TYPE_NEG);
    ext_int_enable(EXTI_6);

    NVIC_SetPriority(TIMER0_IRQn, 0);
    NVIC_SetPriority(EXTI_IRQn, 0);

    timer_init(TIMER0, 1000, TIMER_PERIODIC);
    timer_run(TIMER0);

    IR_LEARN_ENABLE();

    NVIC_EnableIRQ(EXTI_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);

    return true;
}

/*********************************************************************
 * @fn      IR_stop_learn
 *
 * @brief   Stop the infrared learning
 *
 *
 *
 *
 * @param   None
 *
 *
 * @return  None.
 */
void IR_stop_learn(void)
{

    ir_learn_data.IR_learn_state &= (~ BIT(1));
    timer_stop(TIMER0);
    ext_int_disable(EXTI_6);
    NVIC_DisableIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 2);
    NVIC_SetPriority(EXTI_IRQn,2);
    if(((ir_learn_data.IR_learn_state & BIT(0)) == BIT(0)))
    {
        co_printf("IR learn success! ir_learn_data.ir_carrier_fre = %d\r\n",ir_learn_data.ir_carrier_fre);
        for(uint8_t i = 0; i < ir_learn_data.ir_learn_data_cnt; i++)
        {
            co_printf(" %d",ir_learn_data.ir_learn_Date[i]);
        }
    }
    else
    {
        co_printf("IR learn failed!\r\n");
    }
    IR_LEARN_DISENABLE();
    if(ir_learn!=NULL)
    {
        os_free(ir_learn);
    }

#if IR_SLEEP_EN
    system_sleep_enable();
#endif

    //if(ir_learn ==NULL)
    co_printf("ir_learn = %x\r\n",ir_learn);
}

static int IR_task_func(os_event_t *param)
{
    switch(param->event_id)
    {
        case 0:
            pmu_set_pin_to_PMU(GPIO_PORT_C,(1<<GPIO_BIT_5));
            pmu_set_port_mux(GPIO_PORT_C,GPIO_BIT_5,PMU_PORT_MUX_GPIO);
            pmu_set_pin_dir(GPIO_PORT_C,BIT(5),GPIO_DIR_IN);
            //pmu_set_gpio_value(GPIO_PORT_C, (1<<GPIO_BIT_5), 0);
            co_printf("IR Send End \r\n");
            IR_PWM_TIM.IR_Busy = false;
            timer_stop(TIMER0);
            NVIC_DisableIRQ(TIMER0_IRQn);
            //NVIC_SetPriority(TIMER0_IRQn, 2);
#if IR_SLEEP_EN
            system_sleep_enable();
#endif
            break;
    }

    return EVT_CONSUMED;
}

void IR_init(void)
{
    if(user_ir_task_id == TASK_ID_NONE)
        user_ir_task_id = os_task_create(IR_task_func);
    sys_clk = system_get_pclk_config();
    if(sys_clk == 6)
        sys_clk_cfg = 3;
    else if(sys_clk == 12)
        sys_clk_cfg = 2;
    else if(sys_clk == 24)
        sys_clk_cfg = 1;
    else
        sys_clk_cfg = 0;
    IR_LEARN_PIN_INIT();
    IR_LEARN_DISENABLE();
}


/***********************************************Test demo************************************************************/
#include "os_timer.h"
uint8_t ir_testdata[10]= {0x01,0x02,0x03,0x04,0x05,0x06,0x07};
os_timer_t os_timer_IR_test;
void os_timer_IR_test_cb(void *arg)
{
    co_printf("IR_stop_learn!\r\n");
    IR_stop_learn();
}
//发送红外测试码
void IR_test_demo0(void)
{
#if 1
    TYPEDEFIRPWMTIM IR_send_data= {0};
    IR_decode(ir_testdata,4,&IR_send_data);
    IR_send_data.ir_carrier_fre = IR_CARRIER_FRE;
    IR_send_data.loop = true;//开启循环发送
    IR_start_send(&IR_send_data);
#endif
}


//红外学习，只能学习一个键的内容，并通过 IR_test_demo2()把学到的内容发出去。 调用后 5s结束，只要被学习的遥控器按一次按键即
//算学习成功。
void IR_test_demo1(void)
{
#if 1
    if(IR_start_learn())
    {
        co_printf("IR_start_learn!\r\n");
        os_timer_init(&os_timer_IR_test, os_timer_IR_test_cb, NULL);
        os_timer_start(&os_timer_IR_test, 5000, 0);//5s后退出红外学习模式
    }
    else
    {
        co_printf("IR_start_learn Fail!\r\n");
    }

#endif

}

//发送红外学习码， 只能发送刚刚学习到的红外码，与 IR_test_demo1() 红外学习配套使用。
void IR_test_demo2(void)
{
#if 1
    TYPEDEFIRPWMTIM IR_send_data= {0};
    if(((ir_learn_data.IR_learn_state & BIT(0)) == BIT(0)))
    {
        co_printf("send IR learn data,ir_learn_data.ir_learn_data_cnt=%d\r\n",ir_learn_data.ir_learn_data_cnt);
        IR_send_data.ir_carrier_fre = ir_learn_data.ir_carrier_fre;
        IR_send_data.IR_pwm_Num = ir_learn_data.ir_learn_data_cnt;
        memcpy(IR_send_data.IR_Pwm_State_Date,ir_learn_data.ir_learn_Date,IR_send_data.IR_pwm_Num*sizeof(uint32_t));
        IR_start_send(&IR_send_data);
    }
    else
    {
        co_printf("Please perform IR learn first\r\n");
    }

#endif
}

//检查红外码 是否有效
static uint8_t ir_data_check(void)
{
    uint16_t i=0;
    uint8_t find_stop_bit_flag =0;
//  uint8_t repeat_code =0;
    uint8_t ir_code_bit_cnt=0;//红外有效编码计数
    uint8_t IR_learn_state =false;

    for( i = 0; i < ir_learn->ir_learn_data_cnt; i++)
    {
        IR_LOG("<%d %d> ",i,ir_learn->ir_learn_Date[i]);
        if((ir_learn->ir_learn_Date[i] <100000)&&(ir_learn->ir_learn_Date[i] > 12000)) //大于12ms判定为短停止位
        {

            find_stop_bit_flag++; //已经获取短停止位
            ir_code_bit_cnt = i+1;
            if(find_stop_bit_flag >= 4) //超过4个短停止位，结束判断
            {
                ir_learn->ir_learn_data_cnt = i+1;
                IR_learn_state = true;//学习成功
                IR_LOG("T1\r\n");
                break;
            }
            //break;
        }
        else if(ir_learn->ir_learn_Date[i] < 100)  //异常数据
        {
            IR_learn_state = false;
            IR_LOG("T2\r\n");
            break;
        }
        else if(ir_learn->ir_learn_Date[i] >=100000)  //结束标志
        {
            if((i>6)&&(find_stop_bit_flag))
            {
                uint8_t ir_data_len1 = i+1-ir_code_bit_cnt;
                uint8_t ir_data_len2 = ir_code_bit_cnt/find_stop_bit_flag;
                IR_LOG("**********************->:%d %d\r\n",ir_data_len2,ir_data_len1);
                if((ir_data_len1 == ir_data_len2) ||(ir_data_len1>10)&&(ir_data_len2 == 22))//第二个条件针对于Gemini-C10
                    ir_learn->ir_learn_data_cnt = i+1;//保留有效数据
                else
                    ir_learn->ir_learn_data_cnt = ir_code_bit_cnt;//舍弃无效数据
                if(ir_data_len2 > 10)
                    IR_learn_state = true;//学习成功
            }
            else if((find_stop_bit_flag == 0)&&(i>10))
            {
                IR_learn_state = true;//学习成功
            }


            IR_LOG("T3\r\n");
            break;
        }
    }
    IR_LOG("i = %d\r\n",i);
    return IR_learn_state;
}


