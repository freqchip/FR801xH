/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 * AMS client profile. protocol websit:
 * https://developer.apple.com/library/archive/documentation/CoreBluetooth/Reference/AppleMediaService_Reference/Specification/Specification.html
 *
 */

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"
#include "AMS_client.h"


/*
 * MACROS (宏定义)
 */
#define REMOTE_CMD_UUID     {0xc2, 0x51, 0xca, 0xf7, 0x56, 0x0e, 0xdf, 0xb8, \
                                                0x8a, 0x4a, 0xb1, 0x57, 0xd8,0x81, 0x3c, 0x9b}
#define ENT_UPDATE_UUID       {0x02, 0xc1, 0x96, 0xba, 0x92, 0xbb, 0x0c, 0x9a, \
                                                0x1f, 0x41, 0x8d, 0x80, 0xce,0xab, 0x7c, 0x2f}
#define ENT_ATT_UUID      {0xd7, 0xd5, 0xbb, 0x70, 0xa8, 0xa3, 0xab, 0xa6, \
                                                0xd8, 0x46, 0xab, 0x23, 0x8c,0xf3, 0xb2, 0xc6}

/*
 * CONSTANTS (常量定义)
 */

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */


/*
 * LOCAL VARIABLES (本地变量)
 */
static void AMS_gatt_write_cmd(uint8_t conidx,enum ams_att_idx att_idx,uint8_t *p_data, uint16_t len);
static void AMS_gatt_write_req(uint8_t conidx,enum ams_att_idx att_idx,uint8_t *p_data, uint16_t len);
static void AMS_gatt_read(uint8_t conidx,enum ams_att_idx att_idx);

static uint8_t cmd_id_mask;
static uint8_t AMS_client_conidx;
uint8_t AMS_client_id;


/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个characteristic attribute对应的UUID的定义。
 * 第一个是UUID的长度，
 * 每二个是UUID的值。
 */
const gatt_uuid_t AMS_att_tb[] =
{
    [AMS_ATT_IDX_REMOTE_CMD] = { UUID_SIZE_16, REMOTE_CMD_UUID},    //write_req, ntf
    [AMS_ATT_IDX_ENT_UPDATE] = { UUID_SIZE_16, ENT_UPDATE_UUID},  //write_req, ntf
    [AMS_ATT_IDX_ENT_ATT] = { UUID_SIZE_16, ENT_ATT_UUID},  // read, write_req
};

void AMS_recv_entity_info(uint8_t conidx,uint8_t *p_data, uint16_t len)
{
    /*
        user get subscribed entity info here
    */
    switch(p_data[0])
    {
        case ENTI_ID_PLAYER:
            break;
        case ENTI_ID_QUEUE:
            break;
        case ENTI_ID_TRACK:
            if(p_data[2] & ENTI_UPDATE_FLAG_TRUNCATED)
            {
                co_printf("truncated\r\n");
            }
            if(p_data[1] ==TRACK_ATT_ID_ARTIST)
            {
                //show_utf8_string(&rsp[3],len -3);
            }
            else if(p_data[1] ==TRACK_ATT_ID_ALBUM)
            {
                //show_utf8_string(&rsp[3],len -3);
            }
            else if(p_data[1] ==TRACK_ATT_ID_TITLE)
            {
                //show_utf8_string(&rsp[3],len -3);
                if(p_data[2] & ENTI_UPDATE_FLAG_TRUNCATED)
                    ams_subscribe_entity_att(ENTI_ID_TRACK,BIT(TRACK_ATT_ID_TITLE));
            }
            else if(p_data[1] ==TRACK_ATT_ID_DURATION)
            {
                // 0x32,0x38,0x35,0x2E,0x32,0x33,0x34, ==> 285.234  sec
                //show_ascii_string(&rsp[3],len -3);  //floating value.
            }
            break;
    }
}

/*********************************************************************
 * @fn      AMS_gatt_msg_handler
 *
 * @brief   Device information gatt message handler.
 *
 *
 * @param   p_msg  - messages from GATT layer.
 *
 * @return  none.
 */
uint16_t AMS_gatt_msg_handler(gatt_msg_t *p_msg)
{
    co_printf("CCC:%x,att_idx[%d]\r\n",p_msg->msg_evt,p_msg->att_idx);
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_NTF_REQ:
        {
            if(p_msg->att_idx == AMS_ATT_IDX_REMOTE_CMD)
            {
                //recv which commands is support by current music. one byte respresent one command
                //refer to enum remote_cmd
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
                uint8_t *value = p_msg->param.msg.p_msg_data;
                AMS_client_conidx = p_msg->conn_idx;

                cmd_id_mask = 0;
                for(uint8_t i = 0; i< p_msg->param.msg.msg_len; i++)    //update cmd_id_mask
                    cmd_id_mask |= (1<<(value[i]));

            }
            else if(p_msg->att_idx == AMS_ATT_IDX_ENT_UPDATE)
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
                AMS_recv_entity_info(p_msg->conn_idx,p_msg->param.msg.p_msg_data, p_msg->param.msg.msg_len);
            }
        }
        break;
        case GATTC_MSG_READ_IND:
        {
            ;
        }
        break;
        case GATTC_MSG_CMP_EVT:
        {
            co_printf("op:%d done\r\n",p_msg->param.op.operation);
            if(p_msg->param.op.operation == GATT_OP_PEER_SVC_REGISTERED)
            {
                uint16_t att_handles[3];
                memcpy(att_handles,p_msg->param.op.arg,6);
                show_reg((uint8_t *)att_handles,6,1);

                gatt_client_enable_ntf_t ntf_enable;
                ntf_enable.conidx = p_msg->conn_idx;
                ntf_enable.client_id = AMS_client_id;
                ntf_enable.att_idx = AMS_ATT_IDX_ENT_UPDATE;
                gatt_client_enable_ntf(ntf_enable);

                ntf_enable.att_idx = AMS_ATT_IDX_REMOTE_CMD;
                gatt_client_enable_ntf(ntf_enable);
            }
            else if(p_msg->param.op.operation == GATT_OP_WRITE_REQ && p_msg->att_idx == AMS_ATT_IDX_ENT_ATT)
                AMS_gatt_read(AMS_client_conidx,AMS_ATT_IDX_ENT_ATT);
        }
        break;
        case GATTC_MSG_LINK_LOST:
            if(p_msg->conn_idx == AMS_client_conidx)
                AMS_client_conidx = 0xff;
            break;
        default:
            break;
    }

    return 0;
}



/*********************************************************************
 * @fn      AMS_gatt_write_cmd
 *
 * @brief   Write(without response) data to peer.
 *
 *
 * @param   conidx  - link idx.
 *          att_idx - attribute idx,refer to enum ams_att_idx.
 *          p_data  - pointer to buffer, its value will be sent.
 *          len     - buffer len.
 *
 * @return  none.
 */
void AMS_gatt_write_cmd(uint8_t conidx,enum ams_att_idx att_idx,uint8_t *p_data, uint16_t len)
{
    gatt_client_write_t write;
    write.conidx = conidx;
    write.client_id = AMS_client_id;
    write.att_idx = att_idx;
    write.p_data = p_data;
    write.data_len = len;
    gatt_client_write_cmd(write);
}
/*********************************************************************
 * @fn      AMS_gatt_write_req
 *
 * @brief   Write(with response) data to peer.
 *
 *
 * @param   conidx  - link idx.
 *          att_idx - attribute idx,refer to enum ams_att_idx.
 *          p_data  - pointer to buffer, its value will be sent.
 *          len     - buffer len.
 *
 * @return  none.
 */
void AMS_gatt_write_req(uint8_t conidx,enum ams_att_idx att_idx,uint8_t *p_data, uint16_t len)
{
    gatt_client_write_t write;
    write.conidx = conidx;
    write.client_id = AMS_client_id;
    write.att_idx = att_idx;
    write.p_data = p_data;
    write.data_len = len;
    gatt_client_write_req(write);
}
/*********************************************************************
 * @fn      AMS_gatt_read
 *
 * @brief   Read data from peer. read result will be update from GATT_EVENT: GATTC_MSG_READ_IND
 *
 *
 * @param   conidx  - link idx.
 *          att_idx - attribute idx,refer to enum ams_att_idx.
 *
 * @return  none.
 */
void AMS_gatt_read(uint8_t conidx,enum ams_att_idx att_idx)
{
    gatt_client_read_t read;
    read.conidx = conidx;
    read.client_id = AMS_client_id;
    read.att_idx = att_idx;
    gatt_client_read(read);
}

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
void AMS_gatt_add_client(void)
{
    gatt_client_t client;

    client.p_att_tb = AMS_att_tb;
    client.att_nb = AMS_ATT_IDX_MAX;
    client.gatt_msg_handler = AMS_gatt_msg_handler;
    AMS_client_id = gatt_add_client(&client);
}



/***********************************Usage Example****************************************/

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
void ams_subscribe_entity_update(enum entity_id_values id,uint8_t att_id_bits)
{
//you can subscribe one or more entity ids. and one or more att_ids.
    uint8_t value[5];
    uint8_t idx = 0;
    value[idx++] = id;
    for(uint8_t i=0; i<4; i++)
    {
        if(att_id_bits & BIT(i))
        {
            value[idx++] = i;
        }
    }
    AMS_gatt_write_req(AMS_client_conidx,AMS_ATT_IDX_ENT_UPDATE,value,idx);
}


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
void ams_subscribe_entity_att(enum entity_id_values id,uint8_t att_id_bit)
{
    uint8_t value[2];
    value[0] = id;
    for(uint8_t i=0; i<4; i++)
    {
        if(att_id_bit & BIT(i))
            value[1] = i;
    }
    AMS_gatt_write_req(AMS_client_conidx,AMS_ATT_IDX_ENT_ATT,value,2);
}

/*********************************************************************
 * @fn      ams_control
 *
 * @brief   send control cmd to apple, and control current music .
 *          After att_idx: AMS_ATT_IDX_REMOTE_CMD is ntf enabled, call this func
 *
 * @param   cmd  - command id, refet to enum remote_cmd.
 *
 * @return  none.
 */
void ams_crtl_cmd(enum remote_cmd cmd)
{
    uint8_t value = cmd;
    if(cmd_id_mask & (1<<cmd))
        AMS_gatt_write_req(AMS_client_conidx,AMS_ATT_IDX_REMOTE_CMD,&value,1);
}

/***********/

