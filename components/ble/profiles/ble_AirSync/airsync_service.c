/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
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
#include "os_task.h"
#include "os_mem.h"
#include "airsync_service.h"
#include "driver_plf.h"


/*
 * MACROS (宏定义)
 */
#define CFG_CON                     20

#define AIRSYNC_SVC_UUID     (0xFEE7)
#define AIRSYNC_WRITE_UUID   (0xFEC7)
#define AIRSYNC_IND_UUID     (0xFEC8)
#define AIRSYNC_READ_UUID    (0xFEC9)


#define WEXIN_PRF_FIX_MAGIC 0xFE
#define WEXIN_PRF_FIX_VER 0x01

//@ grp, WEXIN_CMD_ID
//req: ble device  --> wechat
#define WEXIN_CMD_ID_REQ_AUTH       10001
#define WEXIN_CMD_ID_REQ_SENDDATA   10002
#define WEXIN_CMD_ID_REQ_INIT       10003
//resp: wetchat --> ble device
#define WEXIN_CMD_ID_RSP_AUTH       20001
#define WEXIN_CMD_ID_RSP_SENDDATA   20002
#define WEXIN_CMD_ID_RSP_INIT       20003
//resp: wetchat --> ble device
#define WEXIN_CMD_ID_PUSH_RECV_DATA         30001       //push data to ble device
#define WEXIN_CMD_ID_PUSH_SWITCH_VIEW       30002       //enter/exit UI
#define WEXIN_CMD_ID_PUSH_SWITCH_BACK_GRROUND       30003   //switch background
#define WEXIN_CMD_ID_ERR_DECODE       29999     //decode fail code

/*
 * CONSTANTS (常量定义)
 */
const uint8_t airsync_svc_uuid[] = UUID16_ARR(AIRSYNC_SVC_UUID);
/*
 * TYPEDEFS (类型定义)
 */
enum EmErrorCode
{
    EEC_system = -1,        //normal err
    EEC_needAuth = -2,      //device don't log in. need log in
    EEC_sessionTimeout = -3,    //session key time out, need re-log in
    EEC_decode = -4,        //server prise protocol fail. because of bug of device data
    EEC_deviceIsBlock = -5, //server reject device request in a short time
    EEC_serviceUnAvalibleInBackground = -6, //ios is at backgroud mode, can't work
    EEC_deviceProtoVersionNeedUpdate = -7,  //device protocol is old version
    EEC_phoneProtoVersionNeedUpdate = -8,   //server protocol is old version
    EEC_maxReqInQueue = -9,         //server can't response in time
    EEC_userExitWxAccount = -10,    //user exit Wetchat
};



/*
 * EXTERNAL VARIABLES (外部变量)
 */__PACKED typedef struct
{
    bool need_assemble;
    uint16_t tot_len;
    uint16_t cur_recv_len;
    uint8_t *buf;
}airsync_recv_data_t GCC_PACKED;

__PACKED typedef struct fixhead
{
    uint8_t magic_number;
    uint8_t ver;
    uint16_t len;
    uint16_t cmd_id;
    uint16_t seq;
    uint8_t value[];
}airsync_pkt_t GCC_PACKED;

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */
static uint8_t airsync_svc_id = 0;
static bool airsync_link_ind_enable[CFG_CON] = {0};
static airsync_recv_data_t airsync_recv_data = {0};     //global value to recv whole airsync pkt
static uint16_t rsp_seq;
static void airsync_gatt_indicate(uint8_t conidx, uint8_t *p_data, uint16_t len);


/*********************************************************************
 * Profile Attributes - Table
 * 每一项都是一个attribute的定义。
 * 第一个attribute为Service 的的定义。
 * 每一个特征值(characteristic)的定义，都至少包含三个attribute的定义；
 * 1. 特征值声明(Characteristic Declaration)
 * 2. 特征值的值(Characteristic value)
 * 3. 特征值描述符(Characteristic description)
 * 如果有notification 或者indication 的功能，则会包含四个attribute的定义，除了前面定义的三个，还会有一个特征值客户端配置(client characteristic configuration)。
 *
 */
const gatt_attribute_t airsync_profile_att_table[] =
{
    [AIRSYNC_SVC_IDX]               =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID) },
        GATT_PROP_READ,
        UUID_SIZE_2,
        (uint8_t *)airsync_svc_uuid,
    },
// write character declaration
    [AIRSYNC_WRITE_DEL_IDX]          =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },
        GATT_PROP_READ,
        0,
        NULL,
    },

    [AIRSYNC_WRITE_IDX]               =   {
        { UUID_SIZE_2, UUID16_ARR(AIRSYNC_WRITE_UUID) },
        GATT_PROP_WRITE_REQ,
        600,
        NULL,
    },
// indication character declaration
    [AIRSYNC_IND_DEL_IDX]    =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },
        GATT_PROP_READ,
        0,
        NULL,
    },

    [AIRSYNC_IND_IDX]          =   {
        { UUID_SIZE_2, UUID16_ARR(AIRSYNC_IND_UUID) },
        GATT_PROP_INDI,
        300,
        NULL,
    },

    [AIRSYNC_IND_CCCD_IDX]               =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID) },
        GATT_PROP_READ | GATT_PROP_WRITE,
        2,
        NULL,
    },
// read character declaration
    [AIRSYNC_READ_DEL_IDX]    =   {
        { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID) },
        GATT_PROP_READ,
        0,
        NULL,
    },

    [AIRSYNC_READ_IDX]          =   {
        { UUID_SIZE_2, UUID16_ARR(AIRSYNC_READ_UUID) },
        GATT_PROP_READ,
        600,
        NULL,
    },
};


void airsync_send_req(uint8_t conidx, uint16_t cmd_id,uint8_t * req_value, uint16_t len)
{
    airsync_pkt_t *rsp = (airsync_pkt_t *)os_malloc(sizeof(airsync_pkt_t) + len);
    rsp->magic_number = WEXIN_PRF_FIX_MAGIC;
    rsp->ver = WEXIN_PRF_FIX_VER;
    rsp->len = TO_BIG_EDN_16(sizeof(airsync_pkt_t) + len);
    rsp->cmd_id = TO_BIG_EDN_16(cmd_id);
    rsp->seq = TO_BIG_EDN_16(rsp_seq++);
    memcpy(rsp->value,req_value,len);
    airsync_gatt_indicate(conidx,(uint8_t *)rsp,sizeof(airsync_pkt_t) + len);
    os_free(rsp);
}

void airsync_pkt_hdl(uint8_t conidx, uint8_t *buf, uint16_t len)
{
    airsync_pkt_t *rsp_pkt = (airsync_pkt_t *)buf;

    rsp_pkt->len = TO_LITTLE_EDN_16(rsp_pkt->len);
    rsp_pkt->cmd_id = TO_LITTLE_EDN_16(rsp_pkt->cmd_id);
    rsp_pkt->seq = TO_LITTLE_EDN_16(rsp_pkt->seq);
    co_printf("len:%d,id:%d,seq:%d\r\n",rsp_pkt->len,rsp_pkt->cmd_id,rsp_pkt->seq);

    if( (int16_t)(rsp_pkt->cmd_id) < 0 )
    {
        co_printf("server err :%d\r\n",(int16_t)(rsp_pkt->cmd_id));
    }
    else if(rsp_pkt->cmd_id == WEXIN_CMD_ID_RSP_AUTH)
    {
        uint8_t req_value[] = {0x0a,0x00,0x1a,0x04,0x30,0x37,0x31,0x35,0x12,0x01,0x41};
        airsync_send_req(conidx,WEXIN_CMD_ID_REQ_INIT,req_value,sizeof(req_value));
    }
    else if(rsp_pkt->cmd_id == WEXIN_CMD_ID_RSP_INIT)
    {
        uint8_t req_value[] = {0x0a,0x00,0x12,0x06,0x0a,0x04,0x08,0xbb,0x80,0x00,0x18,0x01};
        airsync_send_req(conidx,WEXIN_CMD_ID_REQ_SENDDATA,req_value,sizeof(req_value));
    }
    else if(rsp_pkt->cmd_id == WEXIN_CMD_ID_RSP_SENDDATA)
    {
        show_reg(rsp_pkt->value,rsp_pkt->len,1);
        //recv data from Server;
    }
    else if(rsp_pkt->cmd_id == WEXIN_CMD_ID_PUSH_RECV_DATA)
    {
        show_reg(rsp_pkt->value,rsp_pkt->len,1);
        //recv data from Server;
    }
    else if(rsp_pkt->cmd_id == WEXIN_CMD_ID_PUSH_SWITCH_VIEW)
    {
        //information that Wetchat exit UI ;
    }
    else if(rsp_pkt->cmd_id == WEXIN_CMD_ID_PUSH_SWITCH_BACK_GRROUND)
    {
        //information that Wetchat exit background ;
    }
}

static void airsync_recv_data_handler(uint8_t conidx, uint8_t *value, uint16_t length)
{
    if(airsync_recv_data.need_assemble == false)
    {
        if(value[0] == WEXIN_PRF_FIX_MAGIC && value[1] == WEXIN_PRF_FIX_VER)
        {
            airsync_pkt_t *req = (airsync_pkt_t *)value;
            uint16_t req_len = TO_LITTLE_EDN_16(req->len);
            if(req_len > length )
            {
                airsync_recv_data.need_assemble = true;
                airsync_recv_data.tot_len = req_len;
                airsync_recv_data.buf = (uint8_t *)os_malloc(req_len);
                memcpy(airsync_recv_data.buf,value,length);
                airsync_recv_data.cur_recv_len += length;
            }
            else
            {
                airsync_recv_data.need_assemble = false;
                airsync_recv_data.buf = value;
                airsync_recv_data.tot_len = length;
            }
        }
    }
    else
    {
        memcpy(airsync_recv_data.buf + airsync_recv_data.cur_recv_len,value,length);
        airsync_recv_data.cur_recv_len += length;
        if(airsync_recv_data.cur_recv_len == airsync_recv_data.tot_len)
        {
            airsync_recv_data.need_assemble = false;
        }
    }

    if(airsync_recv_data.need_assemble == false)
    {
        airsync_pkt_hdl(conidx,airsync_recv_data.buf,airsync_recv_data.tot_len);
        if(airsync_recv_data.buf != NULL)
        {
            os_free(airsync_recv_data.buf);
            memset((void *)&airsync_recv_data,0,sizeof(airsync_recv_data));
        }
    }
}

/*********************************************************************
 * @fn      airsync_svc_msg_handler
 *
 * @brief   AirSync gatt message handler.
 *
 *
 * @param   p_msg  - messages from GATT layer.
 *
 * @return  none.
 */
static uint16_t airsync_svc_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_READ_REQ:
            if(p_msg->att_idx == AIRSYNC_READ_IDX)
            {
                memcpy(p_msg->param.msg.p_msg_data, "\x11\x18\x19", 3);
                return 3;
            }
            else if(p_msg->att_idx == AIRSYNC_IND_CCCD_IDX)
            {
                if(airsync_link_ind_enable[p_msg->conn_idx])
                    memcpy(p_msg->param.msg.p_msg_data, "\x01\x00", 2);
                else
                    memcpy(p_msg->param.msg.p_msg_data, "\x00\x00", 2);
                return 2;
            }
            break;

        case GATTC_MSG_WRITE_REQ:
            if(p_msg->att_idx == AIRSYNC_IND_CCCD_IDX)
            {
                airsync_link_ind_enable[p_msg->conn_idx] = true;
                rsp_seq = 1;
                uint8_t req_value[] = {0x0a,0x00,0x18,0x80,0x80,0x04,0x20,0x01,0x28,0x02,0x3a,0x06,0x18,0x7a,0x93,0x9c,0x3a,0x47};
                airsync_send_req(p_msg->conn_idx,WEXIN_CMD_ID_REQ_AUTH,req_value,sizeof(req_value));
            }
            else if(p_msg->att_idx == AIRSYNC_WRITE_IDX)
            {
                airsync_recv_data_handler(p_msg->conn_idx,p_msg->param.msg.p_msg_data, p_msg->param.msg.msg_len);
            }
            break;

        case GATTC_MSG_CMP_EVT:

            break;
        case GATTC_MSG_LINK_CREATE:
            break;
        case GATTC_MSG_LINK_LOST:
            airsync_link_ind_enable[p_msg->conn_idx] = false;
            break;
        default:
            break;
    }
    return 0;
}


/*********************************************************************
 * @fn      airsync_gatt_indicate
 *
 * @brief   Send indication value, etc.
 *
 *
 * @param   conidx      - link conidx.
 *          len         - length of data.
 *          p_data      - data to be sent.
 *
 * @return  none.
 */
void airsync_gatt_indicate(uint8_t conidx, uint8_t *p_data, uint16_t len)
{
    if (airsync_link_ind_enable[conidx])
    {
        gatt_ind_t ind;
        ind.conidx = conidx;
        ind.svc_id = airsync_svc_id;
        ind.att_idx = AIRSYNC_IND_IDX ;
        ind.data_len = len;
        ind.p_data = p_data;
        gatt_indication(ind);
    }
}

void airsync_start_advertising(uint16_t duration)
{
    uint8_t *pos;
    uint8_t adv_data[0x1C];
    uint8_t adv_data_len = 0;

    uint8_t scan_rsp_data[0x1F];
    uint8_t scan_rsp_data_len = 0;

    gap_adv_param_t adv_param;
    adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
    adv_param.disc_mode = GAP_ADV_DISC_MODE_GEN_DISC;
    adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
    adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
    adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_param.adv_intv_min = 0x28;
    adv_param.adv_intv_max = 0x28;
    gap_set_advertising_param(&adv_param);

    pos = &adv_data[0];

#if 1   //normal adv
    uint8_t manufacturer_value[] = {0x02,0xfe,0x1,0x2,0x3,0x4,0x5,0x6};
    gap_address_get((mac_addr_t *)&manufacturer_value[2]);
#else   //ack adv
    uint8_t manufacturer_value[] = {0x02,0xfe,0xfe,0x01,0x01,0x1,0x2,0x3,0x4,0x5,0x6};
    gap_address_get((mac_addr_t *)&manufacturer_value[5]);
#endif
    *pos++ = sizeof(manufacturer_value) + 1;
    *pos++  = '\xff';
    memcpy(pos, manufacturer_value, sizeof(manufacturer_value));
    pos += sizeof(manufacturer_value);

    uint16_t uuid_value = AIRSYNC_SVC_UUID;
    *pos++ = sizeof(uuid_value) + 1;
    *pos++  = '\x03';
    memcpy(pos, (uint8_t *)&uuid_value, sizeof(uuid_value));
    pos += sizeof(uuid_value);
    adv_data_len = ((uint32_t)pos - (uint32_t)(&adv_data[0]));
    gap_set_advertising_data(adv_data,adv_data_len);

    uint8_t local_name[LOCAL_NAME_MAX_LEN];
    uint8_t local_name_len = gap_get_dev_name(local_name);

    pos = &scan_rsp_data[0];
    *pos++ = local_name_len + 1;  //pos len;  (payload + type)
    *pos++  = '\x09';   //pos: type
    memcpy(pos, local_name, local_name_len);
    pos += local_name_len;
    scan_rsp_data_len = ((uint32_t)pos - (uint32_t)(&scan_rsp_data[0]));
    gap_set_advertising_rsp_data(scan_rsp_data,scan_rsp_data_len );
    gap_start_advertising(duration);
}




/*********************************************************************
 * @fn      airsync_gatt_add_service
 *
 * @brief   airsync Profile add GATT service function.
 *          添加GATT service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void airsync_gatt_add_service(void)
{
    gatt_service_t airsync_profie_svc;

    airsync_profie_svc.p_att_tb = airsync_profile_att_table;
    airsync_profie_svc.att_nb = AIRSYNC_ATT_NB;
    airsync_profie_svc.gatt_msg_handler = airsync_svc_msg_handler;

    airsync_svc_id = gatt_add_service(&airsync_profie_svc);
}





