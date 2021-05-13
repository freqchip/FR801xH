/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"


#define SPSC_UUID128_ARR_TX   {0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPSC_UUID128_ARR_RX   {0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}

#if 0
gatt_uuid_t client_att_tb[] =
{
    [0]  =
    { UUID_SIZE_16, SPSC_UUID128_ARR_TX},
    [1]  =
    { UUID_SIZE_16, SPSC_UUID128_ARR_RX},
};
#else
//const uint8_t spss_svc_uuid[16] = {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07};
const uint8_t spss_svc_uuid[2] = {0xF0, 0xFF,};
gatt_uuid_t client_att_tb[2] = {0};
uint16_t handles[2] = {0,0};
#endif

uint8_t client_id;

uint16_t client_msg_handler(gatt_msg_t *p_msg)
{
    co_printf("CCC:%x\r\n",p_msg->msg_evt);
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
            //if(p_msg->att_idx == 0)
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);
            }
        }
        break;
        case GATTC_MSG_SVC_REPORT:
            gatt_svc_report_t *svc_rpt = (gatt_svc_report_t *)(p_msg->param.msg.p_msg_data);
            co_printf("svc:%d,start_hdl:%d,end_hdl:%d\r\n",svc_rpt->uuid_len,svc_rpt->start_hdl,svc_rpt->end_hdl);
#if 0
            if(memcmp(svc_rpt->uuid,spss_svc_uuid,2) == 0)
            {
                for(uint16_t i = 0; i<(svc_rpt->end_hdl - svc_rpt->start_hdl); i++)
                {
                    //co_printf("info[%d].att_type:%d\r\n",i,svc_rpt->info[i].att_type);
                    if(svc_rpt->info[i].att_type == ATT_TYPE_CHAR_DECL)
                    {
                        //co_printf("decl, prop:%x,handle:%d\r\n",svc_rpt->info[i].char_decl.prop,svc_rpt->info[i].char_decl.handle);
                        if((svc_rpt->info[i].char_decl.prop & GATT_PROP_NOTI) && handles[0] == 0)
                            handles[0] = svc_rpt->info[i].char_decl.handle;
                        if((svc_rpt->info[i].char_decl.prop & GATT_PROP_WRITE_CMD) && handles[1] == 0)
                            handles[1] = svc_rpt->info[i].char_decl.handle;
                    }
                    if(svc_rpt->info[i].att_type == ATT_TYPE_VAL)
                    {
                        //co_printf("att_val, uuid_len:%d,uuid:",svc_rpt->info[i].att_value.uuid_len);
                        //show_reg(svc_rpt->info[i].att_value.uuid,svc_rpt->info[i].att_value.uuid_len,1);
                        if( i == (handles[0] - svc_rpt->start_hdl - 1))
                        {
                            client_att_tb[0].size = svc_rpt->info[i].att_value.uuid_len;
                            memcpy(client_att_tb[0].p_uuid,svc_rpt->info[i].att_value.uuid,client_att_tb[0].size);
                        }
                        else if( i == (handles[1] - svc_rpt->start_hdl - 1))
                        {
                            client_att_tb[1].size = svc_rpt->info[i].att_value.uuid_len;
                            memcpy(client_att_tb[1].p_uuid,svc_rpt->info[i].att_value.uuid,client_att_tb[1].size);
                        }
                    }
                    if(svc_rpt->info[i].att_type == ATT_TYPE_DESC)
                    {
                        //co_printf("desc, uuid_len:%d,uuid:",svc_rpt->info[i].att_value.uuid_len);
                        //show_reg(svc_rpt->info[i].att_value.uuid,svc_rpt->info[i].att_value.uuid_len,1);
                    }
                }
            }
#endif
            break;
        case GATTC_MSG_CMP_EVT:
        {
            co_printf("op:%d done\r\n",p_msg->param.op.operation);
            if(p_msg->param.op.operation == GATT_OP_PEER_SVC_DISC_END)
            {
                co_printf("peer svc discovery done\r\n");
#if 0
                gatt_add_client_uuid(client_att_tb,2,p_msg->conn_idx,handles);
#endif
            }
            else if(p_msg->param.op.operation == GATT_OP_PEER_SVC_REGISTERED)
            {
                uint16_t att_handles[3];
                memcpy(att_handles,p_msg->param.op.arg,6);
                show_reg((uint8_t *)att_handles,6,1);

//0x10,0x00,0x14,0x00,
                /*
                                gatt_client_enable_ntf_t ntf_enable;
                                ntf_enable.conidx = p_msg->conn_idx;
                                ntf_enable.client_id = client_id;
                                ntf_enable.att_idx = 0; //TX
                                gatt_client_enable_ntf(ntf_enable);
                */
                /*
                                gatt_client_write_t write;
                                write.conidx = p_msg->conn_idx;
                                write.client_id = client_id;
                                write.att_idx = 1; //RX
                                write.p_data = "\x1\x2\x3\x4\x5\x6\x7";
                                write.data_len = 7;
                                gatt_client_write_cmd(write);
                */
                gatt_client_read_t read;
                read.conidx = p_msg->conn_idx;
                read.client_id = client_id;
                read.att_idx = 0; //TX
                //gatt_client_read(read);
                gatt_client_read_with_handle(read,0x12);
            }
            else if(p_msg->param.op.operation == GATT_OP_READ)
            {
                co_printf("heap:%d\r\n",os_get_free_heap_size());
            }
        }
        break;
        default:
            break;
    }

    return 0;
}
void test_write_cmd(void)
{

    gatt_client_write_t write_att;
    uint8_t value[2] = {0x01,0x00};
    write_att.client_id = client_id;
    write_att.conidx = 0;
    write_att.data_len = 2;
    write_att.p_data = value;
    write_att.att_idx = 0;
    gatt_client_write_req_with_handle(write_att,0x11);

    gatt_client_write_t write;
    write.conidx = 0;
    write.client_id = client_id;
    write.att_idx = 0; //RX
    write.p_data = "\x1\x2\x3\x4\x5\x6\x7";
    write.data_len = 7;
    gatt_client_write_cmd_with_handle(write,0x14);


    gatt_client_read_t read;
    read.conidx = 0;
    read.client_id = client_id;
    read.att_idx = 0; //TX
    //gatt_client_read(read);
    gatt_client_read_with_handle(read,0x10);
}


void prf_client_create(void)
{
    gatt_client_t client;
#if 0
    client.p_att_tb = client_att_tb;
    client.att_nb = 2;
#else
    client.p_att_tb = NULL;
    client.att_nb = 0;
#endif

    client.gatt_msg_handler = client_msg_handler;
    client_id = gatt_add_client(&client);
}

/*
NOTE: call below function after master_connected event.

uint16_t handles[3] = {0x10,0x12,0x14};
gatt_client_direct_register(0,0x10,0x14,3, handles);

*/

