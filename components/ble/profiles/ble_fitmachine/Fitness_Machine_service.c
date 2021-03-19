#include <stdio.h.>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"

#include "driver_system.h"

#include "Fitness_Machine_service.h"

//fitness machine gatt profile  uuid :0x1826
const uint8_t fit_svc_uuid[] = UUID16_ARR(0x1826);

const gatt_attribute_t fitness_machine_att_table[ SERVICE_ATTRIBUTE_NUM ] = {
    [ FITNESS_MACHINE_SVC_ATTRIBUTE ] = { 
                                            {UUID_SIZE_2,UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},
                                             GATT_PROP_READ,
                                             UUID_SIZE_2,
                                             (uint8_t*)fit_svc_uuid,
                                        },

    //Characteristic 1 Declaration														
    [ SERVICE_ATTRIBUTE_FEATURE_CHAR ] = {
                                            {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                            GATT_PROP_READ,
                                            0,
                                            NULL,
                                            },
    //Characteristic 1 Value									
    [ SERVICE_ATTRIBUTE_FEATURE_VAL ] = {
                                            {UUID_SIZE_2,UUID16_ARR(GATT_UUID_FITNESS_MACHINE_FEATURE) },
                                            GATT_PROP_READ,
                                            SERVICE_DATA_VAL_LEN,
                                            NULL,
                                            },
    // Characteristic 1 User Description									
    [ SERVICE_ATTRIBUTE_FEATURE_DESC ] = {
                                            { UUID_SIZE_2,UUID16_ARR( GATT_CHAR_USER_DESC_UUID )},
                                            GATT_PROP_READ,
                                            SERVICE_DATA_DESC_LEN,
                                            (uint8_t *)FITNESS_MACHINE_FEATURE,
                                             },

                
    #ifdef   TREADMILL
    //Characteristic 2 Declaration                                         
    [ SERVICE_ATTRIBUTE_TREADMIL_DATA_CHAR ] = {
                                                  { UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                  GATT_PROP_READ,
                                                  0,
                                                  NULL,
                                                  },
    //Characteristic 2 Value
    [ SERVICE_ATTRIBUTE_TREADMIL_DATA_VAL ]  = {
                                                  { UUID_SIZE_2,UUID16_ARR(GATT_UUID_TREADMILL_DATA)},
                                                  GATT_PROP_NOTI,
                                                  SERVICE_DATA_VAL_LEN,
                                                  NULL,
                                                    },
    // Characteristic 2 client characteristic configuration											  
    [ SERVICE_ATTRIBUTE_TREADMIL_DATA_CFG ]  = {
                                                  { UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                  GATT_PROP_READ |GATT_PROP_WRITE,
                                                  0X02,
                                                  NULL,
                                                  },
    // Characteristic 2 User Description
    [ SERVICE_ATTRIBUTE_TREADMIL_DATA_DESC ] =  {
                                                  { UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                  GATT_PROP_READ,
                                                  SERVICE_DATA_DESC_LEN,
                                                  (uint8_t*)FITNESS_MACHINE_TREADMIL_DATA,
                                                  },
    #endif


    #ifdef   	CROSS_TRAINER																					
    //Characteristic 3 Declaration
    [ SERVICE_ATTRIBUTE_TRAINER_DATA_CHAR ] = { 
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                  GATT_PROP_READ,
                                                  0,
                                                  NULL,
                                                  },
    //Characteristic 3Value
    [ SERVICE_ATTRIBUTE_TRAINER_DATA_VAL ] =  {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_UUID_CROSS_TRAINER_DATA)},
                                                  GATT_PROP_NOTI,
                                                  SERVICE_DATA_VAL_LEN,
                                                  NULL,
                                                  },
    // Characteristic 3 client characteristic configuration
    [ SERVICE_ATTRIBUTE_TRAINER_DATA_CFG ] =  {
                                                   {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                   GATT_PROP_READ |GATT_PROP_WRITE,
                                                   0X02,
                                                   NULL,
                                                   },
    // Characteristic 3 User Description
    [ SERVICE_ATTRIBUTE_TRAINER_DATA_DESC ] =  {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,
                                                    (uint8_t*)FITNESS_MACHINE_TRAINER_DATA,
                                                    },	
    #endif	
                                                                                                
    #ifdef   STEP_CLIMBER
    //Characteristic 4 Declaration
    [SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_CHAR] = {
                                                     {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                     GATT_PROP_READ,
                                                     0,
                                                     NULL,
                                                     },
    //Characteristic 4 Value
    [ SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_VAL ] = {
                                                     {UUID_SIZE_2,UUID16_ARR(GATT_UUID_STEP_CLIMBER_DATA)},
                                                     GATT_PROP_NOTI,
                                                     SERVICE_DATA_VAL_LEN,
                                                     NULL,
                                                     },
    // Characteristic 4 client characteristic configuration
    [ SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_CFG ] =  {
                                                     {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                     GATT_PROP_READ |GATT_PROP_WRITE,
                                                     0X02,
                                                     NULL,
                                                     },
    // Characteristic 4 User Description
    [ SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_DESC ] =  {
                                                     {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                     GATT_PROP_READ,
                                                     SERVICE_DATA_DESC_LEN,
                                                     (uint8_t*)FITNESS_MACHINE_STEP_CLIMBER_DATA,
                                                      },
    #endif

    #ifdef   STAIR_CLIMBER
    //Characteristic 5 Declaration  
    [ SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_CHAR ] = {
                                                      {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                      GATT_PROP_READ,
                                                      0,
                                                      NULL,
                                                      },
    //Characteristic 5 Value
    [ SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_VAL ] = {
                                                      {UUID_SIZE_2,UUID16_ARR(GATT_UUID_STAIR_CLIMBER_DATA)},
                                                      GATT_PROP_NOTI,
                                                      SERVICE_DATA_VAL_LEN,
                                                      NULL,
                                                      },
    // Characteristic 5 client characteristic configuration
    [ SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_CFG ] = { 
                                                      {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                      GATT_PROP_READ |GATT_PROP_WRITE,
                                                      0X02,
                                                      NULL,
                                                      },
    // Characteristic 5 User Description
    [ SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_DESC ] = {
                                                      {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                      GATT_PROP_READ,
                                                      SERVICE_DATA_DESC_LEN,
                                                      (uint8_t*)FITNESS_MACHINE_STAIR_CLIMBER_DATA,
                                                      },
    #endif

                                                                                                    
    #ifdef		ROWER																										
    //Characteristic 6 Declaration
    [ SERVICE_ATTRIBUTE_ROWER_DATA_CHAR ]  = {
                                              {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                              GATT_PROP_READ,
                                              0,
                                              NULL,
                                              },
    //Characteristic 6 Value
    [ SERVICE_ATTRIBUTE_ROWER_DATA_VAL ] = {
                                              {UUID_SIZE_2,UUID16_ARR(GATT_UUID_ROWER_DATA)},
                                              GATT_PROP_NOTI,
                                              SERVICE_DATA_VAL_LEN,
                                              NULL,
                                              },
    // Characteristic 6 client characteristic configuration
    [ SERVICE_ATTRIBUTE_ROWER_DATA_CFG ] = {
                                              {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                              GATT_PROP_READ |GATT_PROP_WRITE,
                                              0X02,
                                              NULL,
                                              },
    // Characteristic 6 User Description
    [ SERVICE_ATTRIBUTE_ROWER_DATA_DESC ] = { 
                                              {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                              GATT_PROP_READ,
                                              SERVICE_DATA_DESC_LEN,
                                              (uint8_t*)FITNESS_MACHINE_ROWER_DATA,
                                              },
    #endif


    #ifdef   INDOOR_BIKE
    //Characteristic 7 Declaration
    [ SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 7 Value
    [ SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_VAL ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_INDOOR_BIKE_DATA)},
                                                    GATT_PROP_NOTI,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 7 client characteristic configuration
    [ SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_CFG ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                    GATT_PROP_READ |GATT_PROP_WRITE,
                                                    0X02,
                                                    NULL,
                                                    },
    // Characteristic 7 User Description
    [ SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,
                                                    (uint8_t*)FITNESS_MACHINE_BIKE_DATA,
                                                     },
    #endif
    //Characteristic 8 Declaration
    [ SERVICE_ATTRIBUTE_TRAINING_STATUS_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 8 Value
    [ SERVICE_ATTRIBUTE_TRAINING_STATUS_VAL ] =  {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_TRAINING_STATUS)},
                                                    GATT_PROP_READ | GATT_PROP_NOTI,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 8 client characteristic configuration
    [ SERVICE_ATTRIBUTE_TRAINING_STATUS_CFG ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                    GATT_PROP_READ |GATT_PROP_WRITE,
                                                    0X02,
                                                    NULL,
                                                    },
    // Characteristic 8 User Description
    [ SERVICE_ATTRIBUTE_TRAINING_STATUS_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,
                                                    (uint8_t*)FITNESS_MACHINE_TRAINING_STATUS,
                                                    },
                                                                                                
    //Characteristic 9 Declaration
    [ SERVICE_ATTRIBUTE_SPEED_RANGE_CHAR ] =  {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                  GATT_PROP_READ,
                                                  0,
                                                  NULL,
                                                  },
    //Characteristic 9 Value
    [ SERVICE_ATTRIBUTE_SPEED_RANGE_VAL ]  =  {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_UUID_SUPPORT_SPEED_RANGE)},
                                                  GATT_PROP_READ,
                                                  SERVICE_DATA_VAL_LEN,
                                                  NULL,
                                                },
    // Characteristic 9 User Description
    [ SERVICE_ATTRIBUTE_SPEED_RANGE_DESC ] =  {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                  GATT_PROP_READ,
                                                  SERVICE_DATA_DESC_LEN,
                                                  (uint8_t*)FITNESS_MACHINE_SPEED_RANGE,
                                                  },
     
                                                  
                                                  
    //Characteristic 10 Declaration
    [ SERVICE_ATTRIBUTE_INCLINATION_CHAR ] = {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                  GATT_PROP_READ,
                                                  0,
                                                  NULL,
                                                  },
    //Characteristic 10 Value
    [ SERVICE_ATTRIBUTE_INCLINATION_VAL ]   = {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_UUID_SUPPORT_INCLINATION_RANGE)},
                                                  GATT_PROP_READ,
                                                  SERVICE_DATA_VAL_LEN,
                                                  NULL,
                                                  },
    // Characteristic 10 User Description
    [ SERVICE_ATTRIBUTE_INCLINATION_DESC ] = {
                                                  {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                  GATT_PROP_READ,
                                                  SERVICE_DATA_DESC_LEN,	
                                                  (uint8_t*)FITNESS_MACHINE_INCLINATION,
                                                  },			

                                                  
    //Characteristic 11 Declaration
    [ SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 11 Value
    [ SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_VAL ]   = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_SUPPORT_RESISTANCE_LEVEL_RANGE)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 11 User Description
    [ SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,	
                                                    (uint8_t*)FITNESS_MACHINE_RESISTANCE_LEVEL,
                                                    },						

    //Characteristic 12 Declaration
    [ SERVICE_ATTRIBUTE_POWER_RANGE_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 12 Value
    [ SERVICE_ATTRIBUTE_POWER_RANGE_VAL ]   = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_SUPPORT_POWER_RANGE)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 12 User Description
    [ SERVICE_ATTRIBUTE_POWER_RANGE_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,	
                                                    (uint8_t*)FITNESS_MACHINE_POWER_RANGE,
                                                    },						

    //Characteristic 13 Declaration
    [ SERVICE_ATTRIBUTE_HEART_RATE_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 13 Value
    [ SERVICE_ATTRIBUTE_HEART_RATE_VAL ]   = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_SUPPORT_HEART_RATE_RANGE)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 13 User Description
    [ SERVICE_ATTRIBUTE_HEART_RATE_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,	
                                                    (uint8_t*)FITNESS_MACHINE_HEART_RATE,
                                                    },							
    //Characteristic 14 Declaration
    [ SERVICE_ATTRIBUTE_CONTROL_POINT_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 14 Value
    [ SERVICE_ATTRIBUTE_CONTROL_POINT_VAL ]   = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_FITNESS_MACHINE_CONTROL_POINT)},
                                                    GATT_PROP_WRITE | GATT_PROP_NOTI,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 14 client characteristic configuration
    [ SERVICE_ATTRIBUTE_CONTROL_POINT_CFG ]  = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                    GATT_PROP_READ | GATT_PROP_WRITE,
                                                    0X02,
                                                    NULL,
                                                    },
    // Characteristic 14 User Description
    [ SERVICE_ATTRIBUTE_CONTROL_POINT_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,	
                                                    (uint8_t*)FITNESS_MACHINE_CNTROL_POINT,
                                                    },							
    //Characteristic 15 Declaration
    [ SERVICE_ATTRIBUTE_STATUS_CHAR ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
                                                    GATT_PROP_READ,
                                                    0,
                                                    NULL,
                                                    },
    //Characteristic 15 Value
    [ SERVICE_ATTRIBUTE_STATUS_VAL ]   = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_UUID_FITNESS_MACHINE_STATUS)},
                                                    GATT_PROP_NOTI,
                                                    SERVICE_DATA_VAL_LEN,
                                                    NULL,
                                                    },
    // Characteristic 15 client characteristic configuration
    [ SERVICE_ATTRIBUTE_STATUS_CFG ]  = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
                                                    GATT_PROP_READ | GATT_PROP_WRITE,
                                                    0X02,
                                                    NULL,
                                                    },
    // Characteristic 15 User Description
    [ SERVICE_ATTRIBUTE_STATUS_DESC ] = {
                                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                                    GATT_PROP_READ,
                                                    SERVICE_DATA_DESC_LEN,	
                                                    (uint8_t*)FITNESS_MACHINE_STATUS,
                                                    },
};

static uint8_t fit_svc_id = 0xff;

/* 模拟数组 ，使用者可以重新自定义 */
#define MACHINEFEATURE_LEN      8
uint8_t machinefeature_buf[MACHINEFEATURE_LEN];

#define TRAININGSTATUS_LEN      2
uint8_t trainingstatus_buf[TRAININGSTATUS_LEN] = {0};

#define SPEEDDRANGE_LEN         6
uint8_t speedrange_buf[SPEEDDRANGE_LEN];

#define INCLINATION_LEN         6
uint8_t inclination_buf[INCLINATION_LEN];

#define POWERRANGE_LEN          6 
uint8_t powerrange_buf[POWERRANGE_LEN];

#define RESISTANCELEVELRANGE_LEN    6
uint8_t resistancelevelrange_buf[RESISTANCELEVELRANGE_LEN];

#define HEARTRATERANGE_LEN      6
uint8_t heartrateRange_buf[HEARTRATERANGE_LEN];

#define CONTROLPOINT_LEN        10
uint8_t control_point_buf[ CONTROLPOINT_LEN ] = {0};
       
#define CONTROLPOINT_RECV_LEN    2
uint8_t controlpoint_rec_buf[ CONTROLPOINT_RECV_LEN ] = {0};



#ifdef TREADMILL	
void treadmil_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_TRAINER_DATA_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id =fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
#endif

#ifdef CROSS_TRAINER	
void cross_trainer_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_TRAINER_DATA_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id =fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
#endif

#ifdef STEP_CLIMBER	
void step_climber_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_STEP_CLIMBER_DATA_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id = fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
#endif

#ifdef STAIR_CLIMBER
void stair_climber_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_STAIR_CLIMBER_DATA_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id = fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
#endif

#ifdef ROWER	
void rower_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_ROWER_DATA_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id = fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
#endif
	
#ifdef INDOOR_BIKE	
void indoor_bike_data_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_INDOOR_BIKE_DATA_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id = fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}
#endif

void fitness_training_status_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_TRAINING_STATUS_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id = fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}

void fitness_machinestatus_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ntf_t ntf_att;
    ntf_att.att_idx = SERVICE_ATTRIBUTE_STATUS_VAL;
    ntf_att.conidx = conidx;
    ntf_att.svc_id = fit_svc_id;
    ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ntf_att.p_data = data;
    gatt_notification(ntf_att);
}

void control_point_send_data(uint8_t conidx, uint8_t *data, uint8_t len)
{
    gatt_ind_t ind_att;
    ind_att.att_idx = SERVICE_ATTRIBUTE_CONTROL_POINT_VAL;
    ind_att.conidx = conidx;
    ind_att.svc_id = fit_svc_id;
    ind_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
    ind_att.p_data = data;
    gatt_indication(ind_att);
}

static void fit_gatt_read_cb(uint8_t *p_read, uint16_t *len, uint16_t att_idx )
{
    switch (att_idx)
    {
        case SERVICE_ATTRIBUTE_FEATURE_VAL:
            for (int i = 0; i < MACHINEFEATURE_LEN; i++)
                machinefeature_buf[i] = machinefeature_buf[0] + i + 1;
            memcpy(p_read, machinefeature_buf, MACHINEFEATURE_LEN);
            *len = MACHINEFEATURE_LEN;
            break;
        
        case SERVICE_ATTRIBUTE_TRAINING_STATUS_VAL:
            for (int i = 0; i < TRAININGSTATUS_LEN; i++)
                trainingstatus_buf[i] = trainingstatus_buf[0] + i + 1;
            memcpy(p_read, trainingstatus_buf, TRAININGSTATUS_LEN);
            *len = TRAININGSTATUS_LEN;
            break;

        case SERVICE_ATTRIBUTE_SPEED_RANGE_VAL:
            for (int i = 0; i < SPEEDDRANGE_LEN; i++)
                speedrange_buf[i] = speedrange_buf[0] + i + 1;
            memcpy(p_read, speedrange_buf, SPEEDDRANGE_LEN);
            *len = SPEEDDRANGE_LEN;
            break;
        
        case SERVICE_ATTRIBUTE_INCLINATION_VAL:
            for (int i = 0; i < INCLINATION_LEN; i++)
                inclination_buf[i] = inclination_buf[0] + i + 1;
            memcpy(p_read, speedrange_buf, SPEEDDRANGE_LEN);
            *len = INCLINATION_LEN;
            break;
        
        case SERVICE_ATTRIBUTE_RESISTANCE_LEVEL_VAL:
            for (int i = 0; i < RESISTANCELEVELRANGE_LEN; i++)
                resistancelevelrange_buf[i] = resistancelevelrange_buf[0] + i + 1;
            memcpy(p_read, resistancelevelrange_buf, RESISTANCELEVELRANGE_LEN);
            *len = RESISTANCELEVELRANGE_LEN;
            break;
        
        case SERVICE_ATTRIBUTE_POWER_RANGE_VAL:
            for (int i = 0; i < POWERRANGE_LEN; i++)
                powerrange_buf[i] = powerrange_buf[0] + i + 1;
            memcpy(p_read, resistancelevelrange_buf, POWERRANGE_LEN);
            *len = POWERRANGE_LEN;
            break;
        
        case SERVICE_ATTRIBUTE_HEART_RATE_VAL:
            for (int i = 0; i < HEARTRATERANGE_LEN; i++)
                heartrateRange_buf[i] = heartrateRange_buf[0] + i + 1;
            memcpy(p_read, heartrateRange_buf, HEARTRATERANGE_LEN);
            *len = HEARTRATERANGE_LEN;
            break;
        
        default:
            break;
    }
    
	co_printf("Read request: len: %d  value: 0x%x 0x%x \r\n", *len, (p_read)[0], (p_read)[*len-1]);
}

static void fit_gatt_write_cb(uint8_t *write_buf, uint16_t len, uint16_t att_idx)
{
	for (int i = 0; i < len; i++) {
		co_printf("Write request: len: %d, 0x%x \r\n", len, write_buf[i]);
        if (att_idx == SERVICE_ATTRIBUTE_CONTROL_POINT_VAL)
            memcpy(control_point_buf, write_buf, len);
    }
	
	uint16_t uuid = BUILD_UINT16( fitness_machine_att_table[att_idx].uuid.p_uuid[0], fitness_machine_att_table[att_idx].uuid.p_uuid[1] );
	if (uuid == GATT_CLIENT_CHAR_CFG_UUID) {
        if (att_idx == SERVICE_ATTRIBUTE_CONTROL_POINT_CFG) {
            controlpoint_rec_buf[0] = write_buf[0];
            controlpoint_rec_buf[1] = write_buf[1];
            co_printf("controlpoint_rec_buf : 0x%x 0x%x \r\n", controlpoint_rec_buf[0], controlpoint_rec_buf[1]);
        }
    }
}

static uint16_t fit_svc_msg_handler(gatt_msg_t* p_msg)
{
    uint16_t length = 0;

    switch(p_msg->msg_evt) {
        case GATTC_MSG_READ_REQ:
            fit_gatt_read_cb((uint8_t *)(p_msg->param.msg.p_msg_data), &(p_msg->param.msg.msg_len), p_msg->att_idx);
            break;
						
        case GATTC_MSG_WRITE_REQ:
            fit_gatt_write_cb((uint8_t*)(p_msg->param.msg.p_msg_data), (p_msg->param.msg.msg_len), p_msg->att_idx);
            break;
            
        default:
            break;
    }
    
    return length;
}

void fitness_machine_add_service(void)
{
    static gatt_service_t service;
    service.p_att_tb = fitness_machine_att_table;
    service.att_nb = SERVICE_ATTRIBUTE_NUM;
    service.gatt_msg_handler = fit_svc_msg_handler;
    fit_svc_id = gatt_add_service(&service);
}
