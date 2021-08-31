#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "co_printf.h"
#include "os_timer.h"
#include "driver_system.h"
#include "driver_pmu_regs.h"
#include "driver_rtc.h"
#include "driver_pmu.h"

//#define USE_BB_BASE_CNT       //use baseband basic cnt as clock counter
#define USE_PMU_RTC_CNT         //use pmu rtc cnt as clock counter

typedef struct clock_param
{
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t week;
    uint16_t hour;
    uint16_t min;
    uint16_t sec;
} clock_param_t;

clock_param_t clock_env =
{
    .year = 2010,
    .month = 1,
    .day = 1,
    .week = 5,
    .hour = 0,
    .min = 0,
    .sec = 0,
};

static uint32_t last_ke_time = 0;
#define RTC_10MS_MAX_CNT ((1L<<23) - 1)
#define PMU_RTC_MAX_CNT (0xffffffff)

uint32_t ke_time(void);
int get_days(int, int, int);   //返回从公元元年算起，某年某月某日是第几天， 用这个数字算星期几
int days_of_year(int, int, int);  //返回某年某月某日是当前年份第几天
int days_of_month(int, int);  //返还某年某月有几天
int day_of_week(int, int, int); //返回某年某月某日是星期几，星期天返回0
bool is_leap_year(int);    //返回当前年份是否为闰年

/*************day calculate**************/
#define DAYS_PER_WEEK 7
#define MONTHS 12
#define DATE_HEADER "   Sun   Mon   Tues  Wed   Thur  Fri   Sat"

//return the day idx from  year0, month 0 and day 0
int get_days(int year, int month, int day)
{
    int days = days_of_year(year, month, day);
    int temp = year-1;
    return (temp * 365 + temp / 4 - temp / 100 + temp / 400 +  days -1);        //dyc changed ,from 0 day begins
}

int get_sec_from_2010_1_1(uint16_t year, uint16_t month, uint16_t day, uint16_t hour, uint16_t min, uint16_t sec)
{
    return (get_days(year, month, day) - 733772) * 86400 + hour * 3600 + min * 60 + sec;
}

//return the day idx in this year
int days_of_year(int year, int month, int day)
{
    int i;
    int days = 0;
    for(i = 1; i < month; i++)
    {
        days += days_of_month(year, i);

    }
    return days + day;
}

// return how may days in this month
int days_of_month(int year, int month)
{
    //store day number in no_leap year
    const int  month_days[MONTHS] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if(2 == month && is_leap_year(year))
        return 29; // if it is leap year,return 29
    else
        return month_days[month-1];  //normal return
}

//judge if the year is leap
bool is_leap_year(int year)
{
    return (year % 400 == 0 ) || (year % 4 == 0 && year % 100 != 0);
}

int day_of_week(int year, int month, int day)
{
    return get_days(year, month, day) % DAYS_PER_WEEK;
}
/*************day calculate**************/

/*************clock handle***************/
void clock_hdl(void)
{
#ifdef USE_BB_BASE_CNT
    uint32_t cur_base_time = ke_time();
    uint32_t diff;
    if(cur_base_time >= last_ke_time)
        diff = cur_base_time - last_ke_time;
    else
        diff = cur_base_time + RTC_10MS_MAX_CNT + 1 - last_ke_time;
    //co_printf("base:%d,diff:%d\r\n",ke_time(),diff);
    if( diff > 100 )
    {
        last_ke_time += 100;
        if( last_ke_time > RTC_10MS_MAX_CNT)
            last_ke_time -= (RTC_10MS_MAX_CNT+1);
#endif
#ifdef USE_PMU_RTC_CNT
    uint32_t cur_base_time = rtc_get_value();
    uint32_t diff;
    if(cur_base_time >= last_ke_time)
        diff = cur_base_time - last_ke_time;
    else
        diff = cur_base_time + 1 + (PMU_RTC_MAX_CNT - last_ke_time);

    uint32_t count_in_1s = pmu_get_rc_clk(false);
    //co_printf("cur_base:%d,last_base:%d,diff:%d,cnt_1s:%d\r\n",cur_base_time,last_ke_time,diff,count_in_1s);
    
    if( diff > count_in_1s )
    {
        last_ke_time += count_in_1s;
#endif
        clock_env.sec++;

        if(clock_env.sec>=60)
        {
            clock_env.sec = 0;
            clock_env.min++;
            if(clock_env.min>=60)
            {
                clock_env.min = 0;
                clock_env.hour++;
                if(clock_env.hour>=24)
                {
                    clock_env.hour = 0;
                    clock_env.week++;
                    if(clock_env.week>=8)
                        clock_env.week=1;

                    clock_env.day++;
                    if(clock_env.day >= (days_of_month(clock_env.year,clock_env.month)+1) )
                    {
                        clock_env.day = 1;
                        clock_env.month++;
                        if(clock_env.month>=13)
                        {
                            clock_env.month = 1;
                            clock_env.year++;
                        }
                    }
                }
            }
        }//min hdl
        //co_printf("h:%d-m:%d-s:%d\r\n",clock_env.hour,clock_env.min,clock_env.sec);
    }
}

void show_clock_func()
{
    co_printf("h:%d-m:%d-s:%d\r\n",clock_env.hour,clock_env.min,clock_env.sec);
}
/*************clock handle***************/

os_timer_t demo_rtc_timer;
void demo_rtc_timer_func(void * arg)
{
    clock_hdl();
    //show_clock_func();
}

void demo_rtc_start_timer(void)
{
#ifdef USE_PMU_RTC_CNT
    if(ool_read(PMU_REG_SYSTEM_STATUS) == 0)    //firset power_on
    {
        co_printf("1st powr on\r\n");
        rtc_init();
        last_ke_time = rtc_get_value();
    }
	co_printf("rtc:%d\r\n",rtc_get_value());
#endif

    os_timer_init(&demo_rtc_timer,demo_rtc_timer_func,NULL);
    os_timer_start(&demo_rtc_timer,500,1);
}







