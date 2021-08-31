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
#include "gap_service.h"


/*
 * MACROS (宏定义)
 */
#define CFG_CON                     20

/*
 * CONSTANTS (常量定义)
 */


// GAP Service UUID: 0x1800
static const uint8_t gap_svc_uuid[UUID_SIZE_2] = UUID16_ARR(GAP_SERVICE_UUID);

/*
 * TYPEDEFS (类型定义)
 */
/// Slave preferred connection parameters
struct gap_slv_pref
{
    /// Connection interval minimum
    uint16_t con_intv_min;
    /// Connection interval maximum
    uint16_t con_intv_max;
    /// Slave latency
    uint16_t slave_latency;
    /// Connection supervision timeout multiplier
    uint16_t conn_timeout;
};

/*
 * GLOBAL VARIABLES (全局变量)
 */


/*
 * LOCAL VARIABLES (本地变量)
 */
static uint8_t gap_svc_id;


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

static const gatt_attribute_t gap_att_table[] =
{
    // GAP_IDX_PRIM_SVC - GAP service
    [IDX_GAP_SERVICE] = { { UUID_SIZE_2, UUID16_ARR(GATT_PRIMARY_SERVICE_UUID)},
        GATT_PROP_READ,  UUID_SIZE_2, (uint8_t *)gap_svc_uuid,
    },

    // GAP_IDX_CHAR_DEVNAME - device name declaration
    [IDX_GAP_CHAR_DEVNAME] = { { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},
        GATT_PROP_READ,0, NULL,
    },
    // GAP_IDX_DEVNAME - device name definition
    [IDX_GAP_DEVNAME] = { { UUID_SIZE_2, UUID16_ARR(GATT_DEVICE_NAME_UUID)},
        GATT_PROP_READ | GATT_PROP_WRITE_REQ, 0x20,NULL,
    },

    // GAP_IDX_CHAR_ICON - appearance declaration
    [IDX_GAP_CHAR_ICON] = { {UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},
        GATT_PROP_READ,0,NULL,
    },
    // GAP_IDX_ICON -appearance
    [IDX_GAP_ICON] = { { UUID_SIZE_2, UUID16_ARR(GATT_APPEARANCE_UUID)},
        GATT_PROP_READ | GATT_PROP_WRITE_REQ,sizeof(uint16_t), NULL,
    },

    // GAP_IDX_CHAR_SLAVE_PREF_PARAM - Peripheral parameters declaration
    [IDX_GAP_CHAR_SLAVE_PREF_PARAM] = { { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},
        GATT_PROP_READ, 0,NULL,
    },
    // GAP_IDX_SLAVE_PREF_PARAM - Peripheral parameters definition
    [IDX_GAP_SLAVE_PREF_PARAM] = { {UUID_SIZE_2, UUID16_ARR(GATT_PERI_CONN_PARAM_UUID)},
        GATT_PROP_READ,sizeof(struct gap_slv_pref),NULL,
    },

    // GAP_IDX_CHAR_ADDR_RESOL - Central Address Resolution declaration
    [IDX_GAP_CHAR_CNT_ADDR_RESOL] = { { UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},
        GATT_PROP_READ,0, NULL,
    },
    // GAP_IDX_ADDR_RESOL_SUPP - Central Address Resolution supported
    [IDX_GAP_CNT_ADDR_RESOL] = { { UUID_SIZE_2, UUID16_ARR(GATT_ADDR_RESOL_SUPP)},
        GATT_PROP_READ, sizeof(uint8_t),NULL,
    },

    // GAP_IDX_CHAR_RSLV_PRIV_ADDR_ONLY - Resolvable Private Address Only declaration
    [IDX_GAP_CHAR_RSLV_PRIV_ADDR_ONLY] = { {UUID_SIZE_2, UUID16_ARR(GATT_CHARACTER_UUID)},
        GATT_PROP_READ,0,NULL,
    },
    // GAP_IDX_ADDR_RESOL_SUPP - Central Address Resolution supported
    [IDX_GAP_RSLV_PRIV_ADDR_ONLY] = { {UUID_SIZE_2, UUID16_ARR(GATT_RSLV_PRIV_ADDR_ONLY)},
        GATT_PROP_READ,sizeof(uint8_t),NULL,
    },
};

/*********************************************************************
 * @fn      gap_gatt_op_cmp_handler
 *
 * @brief   Gatt operation complete handler.
 *
 *
 * @param   p_operation  - operation that has compeleted
 *
 * @return  none.
 */
void gap_gatt_op_cmp_handler(gatt_op_cmp_t *p_operation)
{
    if (p_operation->status == 0)
    {}
}


/*********************************************************************
 * @fn      gap_gatt_msg_handler
 *
 * @brief   GAP gatt message handler.
 *
 *
 * @param   p_msg  - messages from GATT layer.
 *
 * @return  none.
 */
static uint16_t gap_gatt_msg_handler(gatt_msg_t *p_msg)
{
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_ATT_INFO_REQ:
            if(p_msg->att_idx == IDX_GAP_DEVNAME)
                return 0;
            else if(p_msg->att_idx == IDX_GAP_ICON)
                return sizeof(uint16_t);
            else if(p_msg->att_idx == IDX_GAP_CNT_ADDR_RESOL)
                return sizeof(uint8_t);
            break;
        case GATTC_MSG_READ_REQ:
            if(p_msg->att_idx == IDX_GAP_DEVNAME)
            {
                return gap_get_dev_name(p_msg->param.msg.p_msg_data);
            }
            else if(p_msg->att_idx == IDX_GAP_ICON)
            {
                *(uint16_t *)(p_msg->param.msg.p_msg_data) = gap_get_dev_appearance();
                return sizeof(uint16_t);
            }
            else if(p_msg->att_idx == IDX_GAP_SLAVE_PREF_PARAM)
            {
                struct gap_slv_pref slv_pref;
                // Slave preferred Connection interval Min
                slv_pref.con_intv_min = 8;
                // Slave preferred Connection interval Max
                slv_pref.con_intv_max = 10;
                // Slave preferred Connection latency
                slv_pref.slave_latency  = 0;
                // Slave preferred Link supervision timeout
                slv_pref.conn_timeout    = 400;  // 2s (500*10ms)

                memcpy((uint8_t *)(p_msg->param.msg.p_msg_data),(uint8_t *)&slv_pref,sizeof(struct gap_slv_pref));
                return sizeof(struct gap_slv_pref);
            }
            else if(p_msg->att_idx == IDX_GAP_CNT_ADDR_RESOL)
            {
                *(uint8_t *)(p_msg->param.msg.p_msg_data) =  0;
                return sizeof(uint8_t);
            }
            else if(p_msg->att_idx == IDX_GAP_RSLV_PRIV_ADDR_ONLY)
            {
                *(uint8_t *)(p_msg->param.msg.p_msg_data) =  0;
                return sizeof(uint8_t);
            }

            break;

        case GATTC_MSG_WRITE_REQ:
            if(p_msg->att_idx == IDX_GAP_DEVNAME)
            {
                gap_set_dev_name((uint8_t *)(p_msg->param.msg.p_msg_data),(uint8_t)(p_msg->param.msg.msg_len));
            }
            else if(p_msg->att_idx == IDX_GAP_ICON)
            {
                gap_set_dev_appearance((uint16_t)(p_msg->param.msg.p_msg_data));
            }
            break;

        case GATTC_MSG_CMP_EVT:
            gap_gatt_op_cmp_handler((gatt_op_cmp_t*)&(p_msg->param.op));
            break;
        case GATTC_MSG_LINK_CREATE:
            break;
        case GATTC_MSG_LINK_LOST:
            break;

        default:
            break;
    }
    return 0;
}

/*********************************************************************
 * @fn      gap_gatt_add_service
 *
 * @brief   Add GAP service to inner database.
 *          添加GAP service到ATT的数据库里面。
 *
 * @param   None.
 *
 *
 * @return  None.
 */
void gap_gatt_add_service(void)
{
    gatt_service_t gap_profie_svc;

    gap_profie_svc.p_att_tb = gap_att_table;
    gap_profie_svc.att_nb = IDX_GAP_NB;
    gap_profie_svc.gatt_msg_handler = gap_gatt_msg_handler;

    gap_svc_id = gatt_add_service(&gap_profie_svc);
}




