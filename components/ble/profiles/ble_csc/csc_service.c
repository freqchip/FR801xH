#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"

#include "driver_system.h"
#include "csc_service.h"

//Cycling Spend and Cadence gatt profile uuid : 0x1816
const uint8_t csc_svc_uuid[] = UUID16_ARR(CSC_SERV_UUID);


#define CSC_FEATURE_LEN			10
uint8_t csc_feature_buf[CSC_FEATURE_LEN] = {0};

#define CSC_SENSOR_LEN			6
uint8_t csc_sensor_buf[CSC_SENSOR_LEN] = {0};

#define CONTROLPOINT_REC_LEN	2
uint8_t sc_controlpoint_rec_buf[CONTROLPOINT_REC_LEN] = {0};

#define MENSUREMENT_LEN			10
uint8_t sc_measurement_rec_buf[MENSUREMENT_LEN] = {0};

#define CONTROLPOINT_LEN		10
uint8_t sc_control_buf[CONTROLPOINT_LEN] = {0};


const gatt_attribute_t csc_att_table[CSC_ATTRIBUTE_NB] = {
	[CSC_SVC_ATTRIBUTE] = {
							{UUID_SIZE_2,UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},
							GATT_PROP_READ,
							UUID_SIZE_2,
							(uint8_t*)csc_svc_uuid,
							},
	
	//Characteristic 1 Declaration
	[CSC_FEATURE_ATT_CHAR] = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
								GATT_PROP_READ,
								0,
								NULL,
								},
	//Characteristic 1 Value
	[CSC_FEATURE_ATT_VAL]  = {
								{UUID_SIZE_2,UUID16_ARR(CSC_FEATURE_UUID)},
								GATT_PROP_READ,
								SERVICE_DATA_VAL_LEN,
								NULL,
								},
	//Characteristic 1 Description
	[CSC_FEATURE_ATT_DESC] = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
								GATT_PROP_READ,
								SERVICE_DATA_DESC_LEN,
								(uint8_t*)CSC_FEATURE,
								},
	
	//Characteristic 2 Declaration
	[CSC_MEASURE_ATT_CHAR]  = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
								GATT_PROP_READ,
								0,
								NULL,
								},
	//Characteristic 2 Value
	[CSC_MEASURE_ATT_VAL]   = {
								{UUID_SIZE_2,UUID16_ARR(CSC_MEAS_UUID)},
								GATT_PROP_NOTI,
								SERVICE_DATA_VAL_LEN,
								NULL,
								},
	//Characteristic 2 Client config
	[CSC_MEASURE_ATT_CFG]   = { 
								{UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
								GATT_PROP_READ |GATT_PROP_WRITE,
								0x02,
								NULL,
								},
	//Characteristic 2 Description
	[CSC_MEASURE_ATT_DESC]  = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
								GATT_PROP_READ,
								SERVICE_DATA_DESC_LEN,
								(uint8_t*)CSC_MEASUREMENT,
								},
	
	//Charactristic 3 Declaration
	[CSC_SENSOR_LOCATION_CHAR] = {
									{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
									GATT_PROP_READ,
									0,
									NULL,
									},
	//Characteristic 3 value
    [CSC_SENSOR_LOCATION_VAL]  = {
                                    {UUID_SIZE_2,UUID16_ARR(SENSOR_LOC_UUID)},
                                    GATT_PROP_READ,
                                    SERVICE_DATA_VAL_LEN,
                                    NULL,
                                    },
	//Characteristic 3 Description
	[CSC_SENSOR_LOCATION_DESC] = {
                                    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
                                    GATT_PROP_READ,
                                    SERVICE_DATA_DESC_LEN,
                                    (uint8_t*)CSC_SENSOR_LOCATION,
                                    },
	
	//Characteristic 4 Declaration
	[SC_CONTROL_POINT_CHAR] = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
								GATT_PROP_READ,
								0,
								NULL,
								},
	//Characteristic 4 value
	[SC_CONTROL_POINT_VAL]  = {
								{UUID_SIZE_2,UUID16_ARR(SC_CTRL_PT_UUID)},
								GATT_PROP_WRITE | GATT_PROP_INDI,
								SERVICE_DATA_VAL_LEN,
								NULL,
								},
	//Characteristic 4 Client config
	[SC_CONTROL_POINT_CFG]  = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
								GATT_PROP_READ |GATT_PROP_WRITE,
								0x02,
								NULL,
								},
	//Characteristic 4 Description
	[SC_CONTROL_POINT_DESC] = {
								{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
								GATT_PROP_READ,
								SERVICE_DATA_DESC_LEN,
								(uint8_t*)SC_CONTROL_POINT,
								},
	
};

static uint8_t csc_svc_id = 0xff;

void csc_measurement_send_data(uint8_t conidx,uint8_t * data, uint8_t len)
{
	gatt_ntf_t ntf_att;
	ntf_att.att_idx = CSC_MEASURE_ATT_VAL;
	ntf_att.conidx = conidx;
	ntf_att.svc_id = csc_svc_id;
	ntf_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
	ntf_att.p_data = data;
	
	gatt_notification(ntf_att);
	
}

void sc_control_point_send_data(uint8_t conidx,uint8_t * data, uint8_t len)
{
	gatt_ind_t ind_att;
	ind_att.att_idx = SC_CONTROL_POINT_VAL;
	ind_att.conidx = conidx;
	ind_att.svc_id = csc_svc_id;
	ind_att.data_len = MIN(len,gatt_get_mtu(conidx) - 7);
	ind_att.p_data = data;
	
	gatt_indication(ind_att);
}
static void csc_gatt_read_cb(uint8_t *p_read,uint16_t *len,uint16_t att_idx)
{
	switch(att_idx)
	{
		case CSC_FEATURE_ATT_VAL:
			for(int i =0; i < CSC_FEATURE_LEN;i++)
				csc_feature_buf[i] = csc_feature_buf[0]+ i + 1;
			memcpy(p_read,csc_feature_buf,CSC_FEATURE_LEN);
		    *len = CSC_FEATURE_LEN;
			break;
		case CSC_SENSOR_LOCATION_VAL:
			for(int i =0; i < CSC_SENSOR_LEN;i++)
				csc_sensor_buf[i] = csc_sensor_buf[0]+ i + 1;
			memcpy(p_read,csc_sensor_buf,CSC_SENSOR_LEN);
		    *len = CSC_SENSOR_LEN;
			break;
		
		default:
			break;
	}
}
static void csc_gatt_write_cb(uint8_t *write_buf,uint16_t len,uint16_t att_idx)
{
	for(int i = 0;i < len; i++){
        co_printf("CSC Write request: len: %d, 0x%x \r\n", len, write_buf[i]);
    }
    
    if(att_idx == SC_CONTROL_POINT_VAL)	{
        memcpy(sc_control_buf,write_buf,len);
    }
		
	uint16_t uuid = BUILD_UINT16(csc_att_table[att_idx].uuid.p_uuid[0],csc_att_table[att_idx].uuid.p_uuid[1]);
	if(uuid == GATT_CLIENT_CHAR_CFG_UUID){
		if(att_idx == CSC_MEASURE_ATT_CFG){
			sc_measurement_rec_buf[0] = write_buf[0];
			sc_measurement_rec_buf[1] = write_buf[1];
			co_printf("csc_measurement_rec_buf: 0x%x  0x%x  \r\n",sc_measurement_rec_buf[0],sc_measurement_rec_buf[1]);
		}
		if(uuid == SC_CONTROL_POINT_CFG){
			sc_controlpoint_rec_buf[0] = write_buf[0];
			sc_controlpoint_rec_buf[1] = write_buf[1];
			co_printf("cntrolpoint_rec_buf: 0x%x  0x%x  \r\n",sc_controlpoint_rec_buf[0],sc_controlpoint_rec_buf[1]);
		}
	}
}
static uint16_t csc_svc_msg_handler(gatt_msg_t* p_msg)
{
	switch(p_msg->msg_evt){
		case GATTC_MSG_READ_REQ:
			csc_gatt_read_cb((uint8_t *)(p_msg->param.msg.p_msg_data),&(p_msg->param.msg.msg_len),p_msg->att_idx);
			break;
		case GATTC_MSG_WRITE_REQ:
            csc_gatt_write_cb((uint8_t *)(p_msg->param.msg.p_msg_data),(p_msg->param.msg.msg_len),p_msg->att_idx);
			break;
		
		default:
			break;
	}
	
	return p_msg->param.msg.msg_len;
}
void csc_add_service(void)
{
	static gatt_service_t service;
	service.p_att_tb = csc_att_table;
	service.att_nb = CSC_ATTRIBUTE_NB;
	service.gatt_msg_handler = csc_svc_msg_handler;
	csc_svc_id = gatt_add_service(&service);
}

