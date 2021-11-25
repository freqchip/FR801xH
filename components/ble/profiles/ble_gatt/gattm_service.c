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
#include "gattm_service.h"


/*
 * MACROS (宏定义)
 */
#define CFG_CON                     20

/*
 * CONSTANTS (常量定义)
 */
// GATTM Service UUID: 0x1801
static const uint8_t gattm_svc_uuid[UUID_SIZE_2] = UUID16_ARR(GATT_SERVICE_UUID);

/*
 * TYPEDEFS (类型定义)
 */
/// Service Changed type definition
struct gatt_svc_changed
{
    /// Service start handle which changed
    uint16_t start_hdl;
    /// Service end handle which changed
    uint16_t end_hdl;
};

/*
 * GLOBAL VARIABLES (全局变量)
 */


/*
 * LOCAL VARIABLES (本地变量)
 */
static uint8_t gattm_svc_id;
static uint8_t gattm_link_ind_enable[CFG_CON] = {0};
struct gatt_svc_changed svc_chg = {0x0001, 0xFFFF};


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

static const gatt_attribute_t gattm_att_table[] =
{
    // GATT_IDX_PRIM_SVC - GATT service
    [IDX_GATTM_SERVICE] = { { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},
        GATT_PROP_READ,  UUID_SIZE_2, (uint8_t *)gattm_svc_uuid,
    },

    // GATT_IDX_CHAR_SVC_CHANGED - Service Changed declaration
    [IDX_GATTM_CHAR_SVC_CHANGED] = { { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},
        GATT_PROP_READ,0, NULL,
    },
    // GATT_IDX_SVC_CHANGED - Service Changed definition
    [IDX_GATTM_SVC_CHANGED] = { { UUID_SIZE_2, UUID16_ARR(GATT_SERVICE_CHANGED_UUID)},
        GATT_PROP_READ | GATT_PROP_INDI, sizeof(struct gatt_svc_changed),NULL,
    },
    // GATT_IDX_SVC_CHANGED_CFG - Service Changed Client Characteristic Configuration Descriptor
    [IDX_GATTM_SVC_CHANGED_CFG] = { {UUID_SIZE_2, UUID16_ARR(GATT_CLIENT_CHAR_CFG_UUID)},
        GATT_PROP_READ | GATT_PROP_WRITE_REQ, sizeof(uint16_t),NULL,
    },
};

/*********************************************************************
 * @fn      gattm_gatt_op_cmp_handler
 *
 * @brief   Gatt operation complete handler.
 *
 *
 * @param   p_operation  - operation that has compeleted
 *
 * @return  none.
 */
void gattm_gatt_op_cmp_handler(gatt_op_cmp_t *p_operation)
{
    if (p_operation->status == 0)
    {}
}


/*********************************************************************
 * @fn      gattm_gatt_msg_handler
 *
 * @brief   GATTM gatt message handler.
 *
 *
 * @param   p_msg  - messages from GATT layer.
 *
 * @return  none.
 */
static uint16_t gattm_gatt_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_ATT_INFO_REQ:
            if(p_msg->att_idx == IDX_GATTM_SVC_CHANGED_CFG)
                return sizeof(uint16_t);
            break;
        case GATTC_MSG_READ_REQ:
            if(p_msg->att_idx == IDX_GATTM_SVC_CHANGED_CFG)
            {
                *(uint16_t *)(p_msg->param.msg.p_msg_data) = (0<<0)|(1<<1);  // (ntf << 0)|(ind << 1)
                return sizeof(uint16_t);
            }
            else if(p_msg->att_idx == IDX_GATTM_SVC_CHANGED)
            {
                memcpy((uint8_t *)(p_msg->param.msg.p_msg_data),&svc_chg, sizeof(struct gatt_svc_changed));
                return sizeof(struct gatt_svc_changed);
            }

            break;

        case GATTC_MSG_WRITE_REQ:
            if(p_msg->att_idx == IDX_GATTM_SVC_CHANGED_CFG)
            {
                if(*(uint16_t *)(p_msg->param.msg.p_msg_data) & (1<<1))
                {
                    co_printf("svc_changed ind, enable\r\n");
                    gattm_link_ind_enable[p_msg->conn_idx] = true;
                }
            }
            break;

        case GATTC_MSG_CMP_EVT:
            //gap_gatt_op_cmp_handler((gatt_op_cmp_t*)&(p_msg->param.op));
            break;
        case GATTC_MSG_LINK_CREATE:
            break;
        case GATTC_MSG_LINK_LOST:
            gattm_link_ind_enable[p_msg->conn_idx] = false;
            break;

        default:
            break;
    }
    return 0;
}

/*********************************************************************
 * @fn      gattm_svc_changed_indicaiton
 *
 * @brief   Send svc changed indication to peer. if services devices has a change of attributors, service device can 
 *          indicate client devices by sending svc_changed start hdl and svc_changed end hdl. Then client device
 *          will discovery all attributors between this start handle and end handle. 
 *
 * @param   conidx  - link idx.
 *          svc_shdl  - Start of Affected Attribute Handle Range
 *          svc_ehdl  - End of Affected Attribute Handle Range
 *
 * @return  none.
 */
void gattm_svc_changed_indicaiton(uint8_t conidx,uint16_t svc_shdl,uint16_t svc_ehdl)
{
    svc_chg.start_hdl = svc_shdl;
    svc_chg.end_hdl = svc_ehdl;    
    if(gattm_link_ind_enable[conidx])
    {
        gatt_ind_t ind;
        ind.conidx = conidx;
        ind.svc_id = gattm_svc_id;
        ind.att_idx = IDX_GATTM_SVC_CHANGED;
        ind.data_len = sizeof(struct gatt_svc_changed);
        ind.p_data = (uint8_t *)&svc_chg;
        gatt_indication(ind);
    }
}



/*********************************************************************
 * @fn      gattm_gatt_add_service
 *
 * @brief   Add GATTM service to inner database.
 *          添加GATTM service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void gattm_gatt_add_service(void)
{
    gatt_service_t gattm_profie_svc;

    gattm_profie_svc.p_att_tb = gattm_att_table;
    gattm_profie_svc.att_nb = IDX_GATTM_NB;
    gattm_profie_svc.gatt_msg_handler = gattm_gatt_msg_handler;

    gattm_svc_id = gatt_add_service(&gattm_profie_svc);
}




