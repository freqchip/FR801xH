#ifndef _DRIVER_IR_SEND_H
#define _DRIVER_IR_SEND_H
#include <stdint.h>
#include <stdbool.h>


#define IRLEADLOW     9000     //引导码
#define IRLEADHIGHT   4500

#define IRLOGIC0LOW   560      //0
#define IRLOGIC0HIGHT 560

#define IRLOGIC1LOW    560     //1
#define IRLOGIC1HIGHT  1680

#define IRSTOPLOW    600        //停止位
#define IRSTOPHIGHT     13930+500 //停止标志位长度   也是红外学习中判定为停止位的时间长度

#define IR_CF_36K   36000
#define IR_CF_38K   38000
#define IR_CF_56K   56000



#define IR_CARRIER_FRE       IR_CF_38K


#define IR_SLEEP_EN 1  //系统开启sleep的情况下，需要将该宏定义为1，否则定义为0
#define LERANDATABUFMAX 3000
#define LERANDATACNTMAX 200


//红外发送参数结构体
typedef struct
{
    uint8_t IR_Busy;//IR 发送busy
    uint8_t IR_pwm_state;//高为1，低为0
    uint8_t IR_pwm_Num;//IR 待发送间隔
    uint8_t IR_pwm_SendNum;//IR 已发送间隔
    uint32_t IR_Pwm_State_Date[LERANDATACNTMAX];//保存将红外编码转成定时时间的数据
    uint32_t ir_carrier_fre;//载波频率
    uint32_t total_count;//pwm total_count
    uint32_t high_count_half;//pwm high_count
    bool loop;//IR是否循环发送 true:循环发送 false:单次发送
} TYPEDEFIRPWMTIM;

extern TYPEDEFIRPWMTIM IR_PWM_TIM;

//红外学习后的数据结果结构体
typedef struct
{
    uint8_t IR_learn_state;//IR 学习状态，bit0:0：未学习， 1：已学习;  bit1 1:学习中,0:不在学习中
    uint32_t ir_learn_Date[LERANDATACNTMAX];//保存将红外学习来的时间间隔保存到该数据中，可以直接发送
    uint8_t ir_learn_data_cnt;//学习间隔个数
    uint32_t ir_carrier_fre;//载波频率
} TYPEDEFIRLEARNDATA;
extern TYPEDEFIRLEARNDATA ir_learn_data;

//红外学习相关的结构体
typedef struct
{
    uint8_t ir_learn_step;//IR学习步骤
    uint32_t ir_carrier_fre;//载波频率
    uint8_t ir_carrier_cycle;//载波周期
    uint8_t ir_learn_start;//1:一开始 0：未开始
    uint16_t ir_carrier_times;//载波计数
    uint16_t ir_timer_cnt;//定时器计数
//  uint8_t ir_learn_data_cnt;//学习间隔个数
//  uint32_t ir_learn_Date[100];//保存红外学习的间隔
    uint32_t ir_learn_data_buf[LERANDATABUFMAX];//ir脉冲间隔缓存
    uint16_t ir_learn_data_buf_cnt;//ir脉冲间隔缓存个数
    uint32_t ir_learn_Date[LERANDATACNTMAX];//保存将红外学习来的时间间隔保存到该数据中，可以直接发送
    uint8_t ir_learn_data_cnt;//学习间隔个数
    uint32_t ir_carrier_cycle_data[6];//载波频率数据缓存
    uint8_t ir_carrier_cycle_data_cnt;//载波频率数据缓存个数
} TYPEDEFIRLEARN;
//extern TYPEDEFIRLEARN ir_learn;

enum IR_LEARN_STEP
{
    //IR_LEARN_GET_CARRIER,
    IR_WAIT_STOP,
    IR_LEARN_GET_DATA,
};

enum IR_MODE
{
    IR_SEND,
    IR_LEARN,
};

/*
1 refer to IR_test_demo0() to send ir value.
2 In Func IR_start_send() &  IR_task_func() to port IR enable & stop pins
3 In func IR_start_learn() & IR_stop_learn() to port IR learn exti isr.
4 Port IR learn pin for below marco:
    #define IR_LEARN_PIN_INIT() 
    #define IR_LEARN_DISENABLE()
    #define IR_LEARN_ENABLE()
*/
void IR_decode(uint8_t *ir_data,uint8_t data_size,TYPEDEFIRPWMTIM *IR_Send_struct);
void IR_start_send(TYPEDEFIRPWMTIM *IR_Send_struct);
void IR_init(void);
void IR_stop_send(void);

uint8_t IR_start_learn(void);
void IR_stop_learn(void);


/*
1 only send ir value, follow IR_test_demo0();
2 learn a ir value from another controller, follow IR_test_demo1(); After call it, 5s timers later, learn stop
    during learn coures, if  IR_learn_state =ir_data_check() in timer0_isr_ram(), IR_learn_state = true. learn is successful.
3 If you want to resend what has been learn in IR_test_demo1(),  call IR_test_demo2() to resend it.
*/
void IR_test_demo0(void);
void IR_test_demo1(void);
void IR_test_demo2(void);

#endif

