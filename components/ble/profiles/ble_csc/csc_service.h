#ifndef CSC_SERVICE_H
#define CSC_SERVICE_H

#include <stdio.h>
#include <stdint.h>



#define SERVICE_DATA_VAL_LEN	20
#define SERVICE_DATA_DESC_LEN	20

//characteristic description
#define CSC_FEATURE					"CSC Feature"
#define CSC_MEASUREMENT				"CSC Measurement"
#define CSC_SENSOR_LOCATION			"Sensor Location"
#define SC_CONTROL_POINT			"SC Control Point"


enum 
{
	CSC_SVC_ATTRIBUTE,
	
	CSC_FEATURE_ATT_CHAR,
	CSC_FEATURE_ATT_VAL,
	CSC_FEATURE_ATT_DESC,
	
	CSC_MEASURE_ATT_CHAR,
	CSC_MEASURE_ATT_VAL,
	CSC_MEASURE_ATT_CFG,
	CSC_MEASURE_ATT_DESC,
	
	CSC_SENSOR_LOCATION_CHAR,
	CSC_SENSOR_LOCATION_VAL,
	CSC_SENSOR_LOCATION_DESC,
	
	SC_CONTROL_POINT_CHAR,
	SC_CONTROL_POINT_VAL,
	SC_CONTROL_POINT_CFG,
	SC_CONTROL_POINT_DESC,

	CSC_ATTRIBUTE_NB,
};

void csc_measurement_send_data(uint8_t conidx,uint8_t * data, uint8_t len);
void sc_control_point_send_data(uint8_t conidx,uint8_t * data, uint8_t len);
void csc_add_service(void);

#endif


