/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef ANCS_CLIENT_H
#define ANCS_CLIENT_H

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
#define ANCS_SVC_UUID "\xd0\x00\x2d\x12\x1e\x4b\x0f\xa4\x99\x4e\xce\xb5\x31\xf4\x05\x79"
/*
 * CONSTANTS (常量定义)
 */


/*
 * TYPEDEFS (类型定义)
 */
enum ancs_att_idx
{
    ANCS_ATT_IDX_CTL_POINT,      //18
    ANCS_ATT_IDX_NTF_SRC,      //27
    ANCS_ATT_IDX_DATA_SRC,      //33
    ANCS_ATT_IDX_MAX,
};

/** @brief ANCS client control command id*/
enum ancs_cmd_id
{
    ANCS_CMD_ID_GET_NOTIFICATION_ATTR = 0,
    ANCS_CMD_ID_GET_APP_ATTR = 1,
    ANCS_CMD_ID_PERFORM_NOTIFICATION_ACTION = 2,
    ANCS_CMD_ID_RESERVED = 255
};

/**  @brief  ANCS action id state */
enum ancs_act_id
{
    ANCS_ACT_ID_POSITIVE = 0,
    ANCS_ACT_ID_NEGATIVE = 1,
    ANCS_ACT_ID_RESERVED  = 255
};

/*
 * GLOBAL VARIABLES (全局变量)
 */
//current call msg uid
extern uint32_t call_notification_uid ;
//current call social msg uid
extern uint32_t social_notification_uid ;
/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */
extern uint8_t ANCS_client_id;
/*********************************************************************
* @fn      ANCS_gatt_add_client
*
* @brief   Create battery server.
*
* @param   None.
*
* @return  None.
*/
void ANCS_gatt_add_client(void);
/*********************************************************************
 * @fn      ANCS_perform_ntf_act
 *
 * @brief   execute positive or negative action for current ancs msg, ex: call or social msg. 
 *
 *
 * @param   conidx  - link idx.
 *          notification_uid  - current ancs msg uid. call_notification_uid, or social_notification_uid.
 *          action_id  - postive or negative.
 *
 * @return  none.
 */
void ANCS_perform_ntf_act(uint8_t conidx, uint32_t notification_uid, enum ancs_act_id action_id);


/*********************************************************************
 * @fn      ANCS_gatt_write_cmd
 *
 * @brief   Send batt level notification to peer, and update batt level
 *
 *
 * @param   conidx  - link idx.
 *          att_idx  - index in client profile uuid table .
 *          p_data  - pointer to the buff which will be sent.
 *          len  - buff len.
 *
 * @return  none.
 */
void ANCS_gatt_write_cmd(uint8_t conidx,enum ancs_att_idx att_idx,uint8_t *p_data, uint16_t len);
void ANCS_gatt_write_req(uint8_t conidx,enum ancs_att_idx att_idx,uint8_t *p_data, uint16_t len);
void ANCS_gatt_read(uint8_t conidx,enum ancs_att_idx att_idx);




#endif







