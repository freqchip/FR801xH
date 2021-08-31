/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef AMS_CLIENT_H
#define AMS_CLIENT_H

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <stdint.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"


/*
 * MACROS (宏定义)
 */
#define AMS_SVC_UUID "\xdc\xf8\x55\xad\x02\xc5\xf4\x8e\x3a\x43\x36\x0f\x2b\x50\xd3\x89"
/*
 * CONSTANTS (常量定义)
 */


/*
 * TYPEDEFS (类型定义)
 */
enum ams_att_idx
{
    AMS_ATT_IDX_REMOTE_CMD,      //47
    AMS_ATT_IDX_ENT_UPDATE,      //51
    AMS_ATT_IDX_ENT_ATT,      //55
    AMS_ATT_IDX_MAX,
};

enum remote_cmd
{
    CMD_ID_PLAY,
    CMD_ID_PAUSE,
    CMD_ID_TOGGLE_PLAY_PAUSE,
    CMD_ID_NEXT_TRACK,
    CMD_ID_PRE_TRACK,
    CMD_ID_VOL_UP,
    CMD_ID_VOL_DOWN,
    CMD_ID_ADV_REPEAT_MODE,
    CMD_ID_ADV_SHUFFLE_MODE,
    CMD_ID_SKIP_FORWARD,
    CMD_ID_SKIP_BACKWARD,
    CMD_ID_LIKE_TRACK,
    CMD_ID_DISLIKE_TRACK,
    CMD_ID_BOOKMARK_TRACK,
};

enum entity_id_values
{
    ENTI_ID_PLAYER,
    ENTI_ID_QUEUE,
    ENTI_ID_TRACK,
};
#define ENTI_UPDATE_FLAG_TRUNCATED (1<<0)

enum player_att_id_values
{
    PLAYER_ATT_ID_NAME,
    PLAYER_ATT_ID_PLAYBACK_INFO,
    PLAYER_ATT_ID_VOL,
};

enum queue_att_id_values
{
    QUEUE_ATT_ID_INDEX,
    QUEUE_ATT_ID_COUNT,
    QUEUE_ATT_ID_SHUFFLE_MODE,
    QUEUE_ATT_ID_REPEAT_MODE,
};
enum track_att_id_values
{
    TRACK_ATT_ID_ARTIST,
    TRACK_ATT_ID_ALBUM,
    TRACK_ATT_ID_TITLE,
    TRACK_ATT_ID_DURATION,
};

enum shuffle_mode
{
    SHUFFLE_MODE_OFF,
    SHUFFLE_MODE_ONE,
    SHUFFLE_MODE_ALL,
};

enum repeat_mode
{
    REPEAT_MODE_OFF,
    REPEAT_MODE_ONE,
    REPEAT_MODE_ALL,
};
/*
 * GLOBAL VARIABLES (全局变量)
 */
extern uint8_t AMS_client_id;

/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */


/*********************************************************************
 * @fn      AMS_gatt_add_client
 *
 * @brief   AMS Profile add GATT client function.
 *          添加AMS GATT client到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void AMS_gatt_add_client(void);


/*********************************************************************
 * @fn      ams_subscribe_entity_update
 *
 * @brief   subscribe some entity info.
 *          call this function to subscribe what you are intrested.
 *          After att_idx: AMS_ATT_IDX_ENT_UPDATE is ntf enabled, call this func
 *
 * @param   id  - entity id. refer to enum entity_id_values
 *          att_id_bits - each bit represent each att_id, multi bits can be set to represent multi att_id is selected
 *
 * @return  None.
 * example: subscribe TRACK informations.        
 *          ams_subscribe_entity_update(ENTI_ID_TRACK
 *          ,BIT(TRACK_ATT_ID_ARTIST)|BIT(TRACK_ATT_ID_ALBUM)|BIT(TRACK_ATT_ID_TITLE)|BIT(TRACK_ATT_ID_DURATION));
 *
 */
void ams_subscribe_entity_update(enum entity_id_values id,uint8_t att_id_bits);

/*********************************************************************
 * @fn      ams_subscribe_entity_update
 *
 * @brief   subscribe some entity info.
 *          call this function only when entity update response is truncated.
 *
 * @param   id  - entity id. refer to enum entity_id_values
 *          att_id_bit - each bit represent each att_id. but only one bit is allowed to be set
 *
 * @return  None.
 * example: subscribe TRACK & TRACK_ATT_ID_TITLE informations.        
 *          ams_subscribe_entity_att(ENTI_ID_TRACK,BIT(TRACK_ATT_ID_TITLE));
 *
 */
void ams_subscribe_entity_att(enum entity_id_values id,uint8_t att_id_bit);

/*********************************************************************
 * @fn      ams_control
 *
 * @brief   send control cmd to apple, and control current music .
 *          After att_idx: AMS_ATT_IDX_REMOTE_CMD is ntf enabled, call this func
 *
 * @param   cmd  - command id, refet to enum remote_cmd.
 *
 * @return  none.
 * example: control iphone to play next track.        
 *          ams_crtl_cmd(CMD_ID_NEXT_TRACK);
 *
 */
void ams_crtl_cmd(enum remote_cmd cmd);


#endif







