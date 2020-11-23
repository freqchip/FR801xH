/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef AIRSYNC_SERVICE_H
#define AIRSYNC_SERVICE_H

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>
#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"


/*
 * MACROS (宏定义)
 */


/*
 * CONSTANTS (常量定义)
 */

enum
{
    AIRSYNC_SVC_IDX,            // AirSync Service

    AIRSYNC_WRITE_DEL_IDX,      // write character declaration
    AIRSYNC_WRITE_IDX,          // write character value

    AIRSYNC_IND_DEL_IDX,      // indication character declaration
    AIRSYNC_IND_IDX,          // indication character value
    AIRSYNC_IND_CCCD_IDX,     //client characteristic configuration

    AIRSYNC_READ_DEL_IDX,      // read character declaration
    AIRSYNC_READ_IDX,          // read character value

    AIRSYNC_ATT_NB,
};
/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */

/*********************************************************************
 * @fn      airsync_gatt_add_service
 *
 * @brief   airsync Profile add GATT service function.
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void airsync_gatt_add_service(void);
/*********************************************************************
 * @fn      airsync_send_req
 *
 * @brief   Send indication req to wetchat server.
 *
 *
 * @param   conidx      - link conidx.
 *          cmd_id      - requuest id.      refer to @grp: WEXIN_CMD_ID at airsync_service.c
 *          req_value   - data to be sent.
 *          len         - data len
 *
 * @return  none.
 */
void airsync_send_req(uint8_t conidx, uint16_t cmd_id,uint8_t * req_value, uint16_t len);
/*********************************************************************
 * @fn      airsync_start_advertising
 *
 * @brief   Start airsync advertising.
 *
 *
 * @param   duration      - link conidx. Advertising duration (in unit of 10ms). 0 means that advertising continues
 *
 * @return  none.
 */
void airsync_start_advertising(uint16_t duration);

#endif







