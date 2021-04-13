#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"
#include "driver_system.h"

#include "HeartRate_service.h"

//heart rate gatt profileuuid :
const uint8_t HtRt_svc_uuid[] = UUID16_ARR(HEARTRATE_SERV_UUID);
#define HR_SERVICE_DATA_DESC_LEN		20

#define HR_SERVICE_DATA_VAL_LEN			20
uint8_t hr_bodysensor_vlaue[HR_SERVICE_DATA_VAL_LEN] = {0};

#define CONTROL_LEN			10
uint8_t hr_control_val[CONTROL_LEN] = {0};

#define HR_MEASUREMENT_LEN	10
uint8_t hr_measurement_buf[HR_MEASUREMENT_LEN]= {0};

const gatt_attribute_t heart_rate_att_table[HEART_RATE_NB] = {
	//Simple gatt Service Declaration
	[HEART_RATE_SERVICE] = {
							{UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},
							GATT_PROP_READ,
							UUID_SIZE_2,
							(uint8_t*)HtRt_svc_uuid,
							},
	//Characteristic 1 Declaration
	[HEART_RATE_MEASUREMENT_CHAR] = {
										{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
										GATT_PROP_READ,
										0,
										NULL,
										},
	//Characteristic 1 value
	[HEART_RATE_MEASUREMENT_VAL]  = {
										{UUID_SIZE_2,UUID16_ARR(HEARTRATE_MEAS_UUID)},
										GATT_PROP_NOTI,
										HR_SERVICE_DATA_VAL_LEN,
										NULL,
										},
	//Characteristic 1 client configration
	[HEART_RATE_MENSUREMENT_CFG] =  {
										{UUID_SIZE_2,UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
										GATT_PROP_READ | GATT_PROP_WRITE,
										0x02,
										NULL,
										},
	//Characteristic 1 Description
	[HEART_RATE_MENSUREMENT_DESC] = {
									    {UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
										GATT_PROP_READ,
										HR_SERVICE_DATA_DESC_LEN,
										(uint8_t*)HEART_RATE_MENSUREMENT,
										},
										
	//Characteristic 2 Declaration
	[BODY_SENSOR_LOCATION_CHAR] = {
									{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
									GATT_PROP_READ,
									0,
									NULL,
									},
	//Characteristic 2 value
	[BODY_SENSOR_LOCATION_VAL]  = {
									{UUID_SIZE_2,UUID16_ARR(BODY_SENSOR_LOC_UUID)},
									GATT_PROP_READ,
									HR_SERVICE_DATA_VAL_LEN,
									NULL,
									},
	//Characteristic 2 Desciption
	[BODY_SENSOR_LOCATION_DESC] = {
									{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
									GATT_PROP_READ,
									HR_SERVICE_DATA_DESC_LEN,
									(uint8_t*)BODY_SENSOR_LOCATION,
									},
	
	//Characteristic 3 Declation
	[HEARTRATE_CONTROL_POINT_CHAR] = {
										{UUID_SIZE_2,UUID16_ARR(GATT_CHARACTER_UUID)},
										GATT_PROP_READ,
										0,
										NULL,
										},
	//Characteristic 3 value
	[HEARTRATE_CONTROL_POINT_VAL] = {
										{UUID_SIZE_2,UUID16_ARR(HEARTRATE_CTRL_PT_UUID)},
										GATT_PROP_WRITE,
										HR_SERVICE_DATA_VAL_LEN,
										NULL,
										},
	//Characteristic 3 Description 
	[HEARTRATE_CONTROL_POINT_DESC] = {
										{UUID_SIZE_2,UUID16_ARR(GATT_CHAR_USER_DESC_UUID)},
										GATT_PROP_READ,
										HR_SERVICE_DATA_DESC_LEN,
										(uint8_t*)HEARTRATE_CONTROL_POINT,
										},
	
};

static uint8_t hr_svc_id = 0xff;

static void heartrate_measurement_send_data(uint8_t conidx, uint8_t *data,uint8_t len)
{
	gatt_ntf_t ntf_att;
	ntf_att.att_idx = HEART_RATE_MEASUREMENT_VAL;
	ntf_att.conidx = conidx;
	ntf_att.svc_id = hr_svc_id;
	ntf_att.data_len  = MIN(len,gatt_get_mtu(conidx) - 7);
	ntf_att.p_data = data;
	
	gatt_notification(ntf_att);
}
static void hr_gatt_read_cb(uint8_t* p_read,uint16_t * len,uint16_t att_idx)
{
	switch(att_idx)
	{
		case BODY_SENSOR_LOCATION_VAL:
			for(int i= 0;i < HR_SERVICE_DATA_VAL_LEN;i++)
				hr_bodysensor_vlaue[i] = hr_bodysensor_vlaue[0] + i + 1;
			memcpy(p_read,hr_bodysensor_vlaue,HR_SERVICE_DATA_VAL_LEN);
			*len = HR_SERVICE_DATA_VAL_LEN;
		default:
			break;
			
	}
}
static void hr_gatt_write_cb(uint8_t *write_buf,uint16_t len,uint16_t att_idx)
{
	for(int i = 0; i < len; i++)
	{   
		co_printf(" HR Write request: len: %d, 0x%x \r\n", len, write_buf[i]);
	}
	if(att_idx == HEARTRATE_CONTROL_POINT_VAL)
			memcpy(hr_control_val,write_buf,len);
    
    uint16_t uuid = BUILD_UINT16(heart_rate_att_table[att_idx].uuid.p_uuid[0],heart_rate_att_table[att_idx].uuid.p_uuid[1]);
	if(uuid == GATT_CLIENT_CHAR_CFG_UUID)
	{
		hr_measurement_buf[1] = write_buf[1];
	}
}
static uint16_t hr_gatt_msg_handler(gatt_msg_t *p_msg)
{
	switch(p_msg->msg_evt)
	{
		case GATTC_MSG_READ_REQ:
			hr_gatt_read_cb((uint8_t*)(p_msg->param.msg.p_msg_data ),&(p_msg->param.msg.msg_len),p_msg->att_idx);
			break;
		case GATTC_MSG_WRITE_REQ:
			hr_gatt_write_cb((uint8_t*)(p_msg->param.msg.p_msg_data),(p_msg->param.msg.msg_len),p_msg->att_idx);
			break;
		
		default:
			break;
	}
	
	return p_msg->param.msg.msg_len;
	
}

void hr_gatt_add_service(void)
{
	static gatt_service_t service;
	service.p_att_tb = heart_rate_att_table;
	service.att_nb = HEART_RATE_NB;
	service.gatt_msg_handler = hr_gatt_msg_handler;
	hr_svc_id = gatt_add_service(&service);
}


