#ifndef FITNESS_MACHINE_SERVICE_H
#define FITNESS_MACHINE_SERVICE_H


#include <stdio.h.>
#include <string.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"



//使用宏定义去确定需要使用的fitness machine

#define TREADMILL                   //跑步机
#define CROSS_TRAINER				//交叉训练机
#define STEP_CLIMBER				//攀岩机
#define STAIR_CLIMBER               //爬楼梯机
#define ROWER						//划船机
#define INDOOR_BIKE					//单车



#define SERVICE_DATA_VAL_LEN		20
#define	SERVICE_DATA_DESC_LEN	20

//characteristic description
#define  FITNESS_MACHINE_FEATURE								"Fitness Machine Feature"
#define	 FITNESS_MACHINE_TREADMIL_DATA					        "Treadmill Data"
#define  FITNESS_MACHINE_TRAINER_DATA						    "Cross Trainer Data"
#define  FITNESS_MACHINE_STEP_CLIMBER_DATA			            "Step Climber Data"
#define  FITNESS_MACHINE_STAIR_CLIMBER_DATA			            "Stair Climber Data"
#define  FITNESS_MACHINE_ROWER_DATA							    "Rower Data"
#define  FITNESS_MACHINE_BIKE_DATA							    "Indoor Bike Data"
#define	 FITNESS_MACHINE_TRAINING_STATUS				        "Training Status"					  //训练状态
#define  FITNESS_MACHINE_SPEED_RANGE						    "Supported Speed Range"				  //速度范围
#define  FITNESS_MACHINE_INCLINATION						    "Supported Inclination Range"		  //倾角范围
#define	 FITNESS_MACHINE_RESISTANCE_LEVEL                       "Supported Resistance Range"          //水平阻力范围
#define  FITNESS_MACHINE_POWER_RANGE						    "Supported Power Range"				  //力量范围
#define	 FITNESS_MACHINE_HEART_RATE							    "Support Heart Rate Range"			  //心率范围
#define	 FITNESS_MACHINE_CNTROL_POINT						    "Fitness Machine Control Point"       //健身器控制点
#define	 FITNESS_MACHINE_STATUS									"Fitness Machine Status"              //健身器状态

//枚举属性句柄
enum
{
    FITNESS_MACHINE_SVC_ATTRIBUTE,

    SERVICE_ATTRIBUTE_FEATURE_CHAR,
    SERVICE_ATTRIBUTE_FEATURE_VAL,
    SERVICE_ATTRIBUTE_FEATURE_DESC,

#ifdef  TREADMILL
    SERVICE_ATTRIBUTE_TREADMIL_DATA_CHAR,
    SERVICE_ATTRIBUTE_TREADMIL_DATA_VAL,
    SERVICE_ATTRIBUTE_TREADMIL_DATA_CFG,
    SERVICE_ATTRIBUTE_TREADMIL_DATA_DESC,
#endif	

#ifdef  CROSS_TRAINER
    SERVICE_ATTRIBUTE_TRAINER_DATA_CHAR,
    SERVICE_ATTRIBUTE_TRAINER_DATA_VAL,
    SERVICE_ATTRIBUTE_TRAINER_DATA_CFG,
    SERVICE_ATTRIBUTE_TRAINER_DATA_DESC,
#endif

#ifdef  STEP_CLIMBER
    SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_CHAR,
    SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_VAL,
    SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_CFG,
    SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_DESC,
#endif		

#ifdef  STAIR_CLIMBER
    SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_CHAR,
    SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_VAL,
    SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_CFG,
    SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_DESC,
#endif

#ifdef  ROWER
    SERVICE_ATTRIBUTE_ROWER_DATA_CHAR,
    SERVICE_ATTRIBUTE_ROWER_DATA_VAL,
    SERVICE_ATTRIBUTE_ROWER_DATA_CFG,
    SERVICE_ATTRIBUTE_ROWER_DATA_DESC,
#endif

#ifdef  INDOOR_BIKE	
    SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_CHAR,
    SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_VAL,
    SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_CFG,
    SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_DESC,
#endif

    SERVICE_ATTRIBUTE_TRAINING_STATUS_CHAR,
    SERVICE_ATTRIBUTE_TRAINING_STATUS_VAL,
    SERVICE_ATTRIBUTE_TRAINING_STATUS_CFG,
    SERVICE_ATTRIBUTE_TRAINING_STATUS_DESC,

    SERVICE_ATTRIBUTE_SPEED_RANGE_CHAR,
    SERVICE_ATTRIBUTE_SPEED_RANGE_VAL,
    SERVICE_ATTRIBUTE_SPEED_RANGE_DESC,

    SERVICE_ATTRIBUTE_INCLINATION_CHAR,
    SERVICE_ATTRIBUTE_INCLINATION_VAL,
    SERVICE_ATTRIBUTE_INCLINATION_DESC,

    SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_CHAR,
    SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_VAL,
    SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_DESC,

    SERVICE_ATTRIBUTE_POWER_RANGE_CHAR,
    SERVICE_ATTRIBUTE_POWER_RANGE_VAL,
    SERVICE_ATTRIBUTE_POWER_RANGE_DESC,

    SERVICE_ATTRIBUTE_HEART_RATE_CHAR,
    SERVICE_ATTRIBUTE_HEART_RATE_VAL,
    SERVICE_ATTRIBUTE_HEART_RATE_DESC,

    SERVICE_ATTRIBUTE_CONTROL_POINT_CHAR,
    SERVICE_ATTRIBUTE_CONTROL_POINT_VAL,
    SERVICE_ATTRIBUTE_CONTROL_POINT_CFG,
    SERVICE_ATTRIBUTE_CONTROL_POINT_DESC,

    SERVICE_ATTRIBUTE_STATUS_CHAR,
    SERVICE_ATTRIBUTE_STATUS_VAL,
    SERVICE_ATTRIBUTE_STATUS_CFG,
    SERVICE_ATTRIBUTE_STATUS_DESC,

    SERVICE_ATTRIBUTE_NUM,
};

void treadmil_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void cross_trainer_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void step_climber_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void stair_climber_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void rower_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void indoor_bike_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void fitness_training_status_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void fitness_machinestatus_send_data(uint8_t conidx, uint8_t *data, uint8_t len);
void control_point_send_data(uint8_t conidx, uint8_t *data, uint8_t len);

void  Fitness_Machine_add_service(void);

#endif
