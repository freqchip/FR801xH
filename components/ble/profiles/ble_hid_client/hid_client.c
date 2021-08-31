/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "hid_client.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"


gatt_uuid_t hid_client_att_tb[HID_CLT_ATT_NB] =
{
    [HID_CLT_INFO_IDX]          = { UUID_SIZE_2, UUID16_ARR(HID_INFORMATION_UUID) },
    [HID_CLT_CONTROL_POINT_IDX] = { UUID_SIZE_2, UUID16_ARR(HID_CTRL_PT_UUID) },
    [HID_CLT_REPORT_MAP_IDX]    = { UUID_SIZE_2, UUID16_ARR(REPORT_MAP_UUID)},
    [HID_CLT_PROTOCOL_MODE_IDX] = { UUID_SIZE_2, UUID16_ARR(PROTOCOL_MODE_UUID)},
    [HID_CLT_BOOT_KEY_IN_IDX]   = { UUID_SIZE_2, UUID16_ARR(BOOT_KEY_INPUT_UUID)},   //ntf enable
    [HID_CLT_BOOT_KEY_OUT_IDX]  = { UUID_SIZE_2, UUID16_ARR(BOOT_KEY_OUTPUT_UUID)},
    [HID_CLT_BOOT_MOUSE_IN_IDX] = { UUID_SIZE_2, UUID16_ARR(BOOT_MOUSE_INPUT_UUID)},   //ntf enable
    [HID_CLT_FEATURE_IDX]       = { UUID_SIZE_2, UUID16_ARR(REPORT_UUID)},   //hid report
};
static const uint8_t hid_svc_uuid[] = HID_SVC_UUID;
uint16_t hid_client_handles[HID_CLT_ATT_NB] = {0};
uint16_t hid_client_props[HID_CLT_ATT_NB] = {0};
// HID report information table
static hid_report_ref_t  hid_rpt_info[HID_CLT_NUM_REPORTS] = {0};


uint8_t hid_client_id;


uint16_t hid_client_msg_handler(gatt_msg_t *p_msg)
{
    //co_printf("CCC:%x\r\n",p_msg->msg_evt);
    switch(p_msg->msg_evt)
    {
        case GATTC_MSG_NTF_REQ:
        {
            if(p_msg->att_idx == 0)
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
            }
        }
        break;
        case GATTC_MSG_READ_IND:
        {
            co_printf("att_idx:%d\r\n",p_msg->att_idx);
            if(p_msg->att_idx == HID_CLT_INFO_IDX)
            {
                co_printf("read hid info\r\n");
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
            }
            else if(p_msg->att_idx == HID_CLT_REPORT_MAP_IDX)
            {
                co_printf("read map\r\n");
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
            }
            else if(p_msg->att_idx >= HID_CLT_REPORT_REF_FEATURE_IDX)
            {
                if((p_msg->att_idx - HID_CLT_FEATURE_IDX)%3 == 1)       //report_info idx
                {
                    uint16_t rpt_info_idx = (p_msg->att_idx - HID_CLT_REPORT_REF_FEATURE_IDX)/3;
                    co_printf("read rpt info[%d]\r\n",rpt_info_idx);
                    show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
                    memcpy((void *)&hid_rpt_info[rpt_info_idx], p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len);
                }
                else if( (p_msg->att_idx - HID_CLT_FEATURE_IDX)%3 == 0 )    //report idx
                {
                    uint16_t rpt_info_idx = (p_msg->att_idx - HID_CLT_REPORT_REF_FEATURE_IDX)/3;
                    co_printf("report idx[%d]\r\n",rpt_info_idx);
                    show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
                    ////process the received data here.

                    ////process the received data here.
                }
            }
        }
        break;
        case GATTC_MSG_SVC_REPORT:
        {
            gatt_svc_report_t *svc_rpt = (gatt_svc_report_t *)(p_msg->param.msg.p_msg_data);
            co_printf("svc_uuid_len:%d,uuid:0x%x,shdl:%d,ehdl:%d\r\n",svc_rpt->uuid_len,*(uint16_t *)(svc_rpt->uuid),svc_rpt->start_hdl,svc_rpt->end_hdl);
#if 0   //show service attributors
            for(uint16_t i = 0; i<(svc_rpt->end_hdl - svc_rpt->start_hdl); i++)
            {
                if(svc_rpt->info[i].att_type > ATT_TYPE_DESC)
                    break;
                //co_printf("info[%d].att_type:%d\r\n",i,svc_rpt->info[i].att_type);
                if(svc_rpt->info[i].att_type == ATT_TYPE_CHAR_DECL)
                    co_printf("[%d],char_decl, prop:0x%X,handle:%d\r\n",i+svc_rpt->start_hdl+1,svc_rpt->info[i].char_decl.prop,svc_rpt->info[i].char_decl.handle);

                if(svc_rpt->info[i].att_type == ATT_TYPE_VAL)
                {
                    co_printf("[%d],char, uuid_len:%d,uuid:",i+svc_rpt->start_hdl+1,svc_rpt->info[i].att_value.uuid_len);
                    show_reg(svc_rpt->info[i].att_value.uuid,svc_rpt->info[i].att_value.uuid_len,1);
                }
                if(svc_rpt->info[i].att_type == ATT_TYPE_DESC)
                {
                    co_printf("[%d],char_desc&cfg, uuid_len:%d,uuid:",i+svc_rpt->start_hdl+1,svc_rpt->info[i].att_value.uuid_len);
                    show_reg(svc_rpt->info[i].att_value.uuid,svc_rpt->info[i].att_value.uuid_len,1);
                }
            }
#endif

#if 1
            if(memcmp(svc_rpt->uuid,hid_svc_uuid,sizeof(hid_svc_uuid)) == 0)
            {
                for(uint16_t i = 0; i<(svc_rpt->end_hdl - svc_rpt->start_hdl); i++)
                {
                    //co_printf("info[%d].att_type:%d\r\n",i,svc_rpt->info[i].att_type);
                    if(svc_rpt->info[i].att_type == ATT_TYPE_CHAR_DECL)
                    {
                        //co_printf("decl, prop:%x,handle:%d\r\n",svc_rpt->info[i].char_decl.prop,svc_rpt->info[i].char_decl.handle);
                        for(uint8_t att_idx = 0; att_idx < (HID_CLT_ATT_NB); att_idx++)
                        {
                            if( hid_client_handles[att_idx] == 0
                                && memcmp(hid_client_att_tb[att_idx].p_uuid,svc_rpt->info[i+1].att_value.uuid,svc_rpt->info[i+1].att_value.uuid_len) == 0
                              )
                            {
                                hid_client_props[att_idx] = svc_rpt->info[i].char_decl.prop;
                                hid_client_handles[att_idx] = svc_rpt->info[i].char_decl.handle;

                                if( memcmp(hid_client_att_tb[att_idx].p_uuid,"\x4D\x2A",2) == 0)
                                {
                                    hid_client_handles[att_idx+1] = hid_client_handles[att_idx] + 1 ;
                                    hid_client_handles[att_idx+2] = hid_client_handles[att_idx] + 2 ;
                                }
                                break;
                            }
                        }
                    }
                    else if(svc_rpt->info[i].att_type == ATT_TYPE_VAL)
                    {
                        //co_printf("att_val, uuid_len:%d,uuid:",svc_rpt->info[i].att_value.uuid_len);
                        //show_reg(svc_rpt->info[i].att_value.uuid,svc_rpt->info[i].att_value.uuid_len,1);
                    }
                    else if(svc_rpt->info[i].att_type == ATT_TYPE_DESC)
                    {
                        //co_printf("desc, uuid_len:%d,uuid:",svc_rpt->info[i].att_value.uuid_len);
                        //show_reg(svc_rpt->info[i].att_value.uuid,svc_rpt->info[i].att_value.uuid_len,1);
                    }
                }
            }
#endif
        }
        break;
        case GATTC_MSG_CMP_EVT:
        {
            //co_printf("op:%d done\r\n",p_msg->param.op.operation);
            if(p_msg->param.op.operation == GATT_OP_PEER_SVC_DISC_END)
            {
                co_printf("peer svc discovery done\r\n");
                gatt_add_client_uuid(hid_client_att_tb,HID_CLT_ATT_NB,p_msg->conn_idx,hid_client_handles);
            }
            else if(p_msg->param.op.operation == GATT_OP_PEER_SVC_REGISTERED)
            {
                //uint16_t att_handles[HID_CLT_ATT_NB];
                memcpy(hid_client_handles,p_msg->param.op.arg,HID_CLT_ATT_NB*2);
                show_reg((uint8_t *)hid_client_handles,HID_CLT_ATT_NB*2,1);

                hid_client_read(p_msg->conn_idx,HID_CLT_INFO_IDX);
                hid_client_read(p_msg->conn_idx,HID_CLT_REPORT_MAP_IDX);
                for(uint8_t report_idx = 0; report_idx<(HID_CLT_NUM_REPORTS*3); report_idx+=3)
                    hid_client_read(p_msg->conn_idx,report_idx + HID_CLT_REPORT_REF_FEATURE_IDX);

                for(uint8_t att_idx = 0; att_idx<(HID_CLT_FEATURE_IDX); att_idx++)        //enable common att ntf
                {
                    if(hid_client_props[att_idx] & GATT_PROP_NOTI && hid_client_handles[att_idx] != 0)
                    {
#if 0
                        gatt_client_enable_ntf_t ntf_enable;
                        ntf_enable.conidx = p_msg->conn_idx;
                        ntf_enable.client_id = hid_client_id;
                        ntf_enable.att_idx = att_idx;
                        gatt_client_enable_ntf(ntf_enable);
#else      //another way
                        gatt_client_write_t write_att;
                        uint8_t value[2] = {0x01,0x00};
                        write_att.client_id = hid_client_id;
                        write_att.conidx = p_msg->conn_idx;
                        write_att.data_len = 2;
                        write_att.p_data = value;
                        write_att.att_idx = 0;
                        gatt_client_write_req_with_handle(write_att,hid_client_handles[att_idx]+1);
#endif
                    }
                }
                for(uint8_t report_idx = 0; report_idx<(HID_CLT_NUM_REPORTS*3); report_idx+=3)      //enable report idx att ntf
                {
                    if(hid_client_props[report_idx + HID_CLT_FEATURE_IDX] & GATT_PROP_NOTI
                       && hid_client_handles[report_idx + HID_CLT_FEATURE_IDX] != 0)
                    {
                        //hid_client_write_cmd(p_msg->conn_idx,report_idx + HID_CLT_FEATURE_CCCD_IDX,"\x01\x00",2);
                        gatt_client_write_t write_att;
                        uint8_t value[2] = {0x01,0x00};
                        write_att.client_id = hid_client_id;
                        write_att.conidx = p_msg->conn_idx;
                        write_att.data_len = 2;
                        write_att.p_data = value;
                        write_att.att_idx = 0;
                        gatt_client_write_req_with_handle(write_att,hid_client_handles[report_idx + HID_CLT_FEATURE_IDX]+2);
                    }
                }
            }
        }
        break;
        default:
            break;
    }

    return 0;
}

void hid_client_read(uint8_t conidx,uint8_t att_idx)
{
    if(hid_client_handles[att_idx] != 0)
    {
        gatt_client_read_t read;
        read.conidx = conidx;
        read.client_id = hid_client_id;
        read.att_idx = 0;
        gatt_client_read_with_handle(read,hid_client_handles[att_idx]);
    }
}
void hid_client_write_cmd(uint8_t conidx, uint8_t att_idx, uint8_t *p_data, uint16_t len)
{
    if(hid_client_handles[att_idx] != 0)
    {
        gatt_client_write_t wrt;
        wrt.conidx = conidx;
        wrt.client_id = hid_client_id;
        wrt.att_idx = 0 ;
        wrt.data_len = len;
        wrt.p_data = p_data;
        gatt_client_write_cmd_with_handle(wrt,hid_client_handles[att_idx]);
    }
}
void hid_client_write_req(uint8_t conidx, uint8_t att_idx, uint8_t *p_data, uint16_t len)
{
    if(hid_client_handles[att_idx] != 0)
    {
        gatt_client_write_t wrt;
        wrt.conidx = conidx;
        wrt.client_id = hid_client_id;
        wrt.att_idx = 0 ;
        wrt.data_len = len;
        wrt.p_data = p_data;
        gatt_client_write_req_with_handle(wrt,hid_client_handles[att_idx]);
    }
}
extern uint8_t client_idx;
void hid_client_create(void)
{
    gatt_client_t client;
    for(uint8_t report_idx = 0; report_idx<(HID_CLT_NUM_REPORTS*3); report_idx+=3)
    {
        memcpy((void *)&hid_client_att_tb[report_idx+HID_CLT_FEATURE_IDX],"\x2\x4D\x2A",sizeof(gatt_uuid_t));
        memcpy((void *)&hid_client_att_tb[report_idx+HID_CLT_REPORT_REF_FEATURE_IDX],"\x2\x08\x29",sizeof(gatt_uuid_t));
        memcpy((void *)&hid_client_att_tb[report_idx+HID_CLT_FEATURE_CCCD_IDX],"\x2\x02\x29",sizeof(gatt_uuid_t));
    }
    client.p_att_tb = NULL;
    client.att_nb = 0;
    client.gatt_msg_handler = hid_client_msg_handler;
    hid_client_id = gatt_add_client(&client);
}

