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

#include "os_timer.h"
#include "driver_pmu.h"


#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "sys_utils.h"
#include "flash_usage_config.h"

/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */

#define SPSC_UUID128_ARR_TX   {0xb8, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}
#define SPSC_UUID128_ARR_RX   {0xba, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07}

/*
 * TYPEDEFS (???????)
 */

/*
 * GLOBAL VARIABLES (??????)
 */
#if 1
const gatt_uuid_t client_att_tb[] =
{
    [0]  =
    { UUID_SIZE_16, SPSC_UUID128_ARR_TX},
    [1]  =
    { UUID_SIZE_16, SPSC_UUID128_ARR_RX},
};
#else
const uint8_t spss_svc_uuid[16] = {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07};
gatt_uuid_t client_att_tb[2] = {0};
uint16_t handles[2] = {0,0};
#endif

uint8_t client_id;
/*
 * LOCAL VARIABLES (本地变量)
 */
static os_timer_t button_anti_shake_timer;		//Timer for button anti-sahke
static uint32_t curr_button_before_anti_shake = 0;		//before anti-sahke timeout, store the pressed button value
 
/*
 * LOCAL FUNCTIONS (本地函数)
 */
static void simple_central_start_scan(void);

/*********************************************************************
 * @fn      button_anti_shake_timeout_handler
 *
 * @brief   Anti-shake-timer execute fuinction.
 *
 * @param   argument for timer function.
 *
 *
 * @return  None.
 */
static void button_anti_shake_timeout_handler(void *param)
{
    uint32_t pmu_int_pin_setting = ool_read32(PMU_REG_PORTA_TRIG_MASK);
    uint32_t gpio_value = ool_read32(PMU_REG_GPIOA_V);
    gpio_value &= pmu_int_pin_setting;
    gpio_value ^= pmu_int_pin_setting;		//get current press key 
	
		//if current press key is same as recorded key before 10ms anti-shake timer
    if(gpio_value == curr_button_before_anti_shake)	
    {
				gpio_value &= ~GPIO_PC4;    //ignore PC4, because of inner usage in lib
        //co_printf("press_key:%08X\r\n",gpio_value);
				mac_addr_t mac_addr1 = {{0x1,0x2,0x3,0x4,0x5,0x6}};
				mac_addr_t mac_addr2 = {{0x2,0x2,0x3,0x4,0x5,0x6}};
				if(gpio_value == GPIO_PD6)
					//gap_start_conn(&mac_addr1,0,12, 12, 0, 300);  //connect device with mac addr1
					gap_bond_manager_delete_all();
				else if(gpio_value == GPIO_PD7)
					gap_start_conn(&mac_addr2,0,12, 12, 0, 300);	//connect device with mac addr2
    }
}

/*********************************************************************
 * @fn      pmu_gpio_isr_ram
 *
 * @brief   pmu gpio interruption.
 *
 * @param   None.
 *
 *
 * @return  None.
 */
__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void) //ISR function for pmu gpio interruption
{
    uint32_t pmu_int_pin_setting = ool_read32(PMU_REG_PORTA_TRIG_MASK);
    uint32_t gpio_value = ool_read32(PMU_REG_GPIOA_V);

    ool_write32(PMU_REG_PORTA_LAST, gpio_value);
    uint32_t tmp = gpio_value & pmu_int_pin_setting;
    curr_button_before_anti_shake = tmp^pmu_int_pin_setting;	//record 1st press key
    os_timer_start(&button_anti_shake_timer, 10, false);	//start 10ms anti-shake timer
}


/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
static void app_gap_evt_cb(gap_event_t *p_event)	//GAP callback function, p_evnet is pointer to content of evnet
{
    switch(p_event->type)		//event type from BLE stack
    {
        case GAP_EVT_SCAN_END:	//scan action is end
            co_printf("scan_end,status:0x%02x\r\n",p_event->param.scan_end_status);
						gap_set_link_rssi_report(false);
            break;
 
        case GAP_EVT_ADV_REPORT:	//got adv report
        {
            uint8_t scan_name[] = "Simple_Peripheral";
            //if(memcmp(event->param.adv_rpt->src_addr.addr.addr,"\x0C\x0C\x0C\x0C\x0C\x0B",6)==0)
            if (p_event->param.adv_rpt->data[0] == 0x12
                && p_event->param.adv_rpt->data[1] == GAP_ADVTYPE_LOCAL_NAME_COMPLETE
                && memcmp(&(p_event->param.adv_rpt->data[2]), scan_name, 0x10) == 0)
            {
                gap_stop_scan();
                
                co_printf("evt_type:0x%02x,rssi:%d\r\n",p_event->param.adv_rpt->evt_type,p_event->param.adv_rpt->rssi);

                co_printf("content:");
                show_reg(p_event->param.adv_rpt->data,p_event->param.adv_rpt->length,1);
                
                gap_start_conn(&(p_event->param.adv_rpt->src_addr.addr),
                                p_event->param.adv_rpt->src_addr.addr_type,
                                12, 12, 0, 300);
            }
        }
        break;
		
		case GAP_EVT_LINK_PARAM_REQ:
		{
		    
			gap_evt_link_param_update_rsp_t rsp;
			if(p_event->param.link_update_req.intv_min > 80
				|| p_event->param.link_update_req.intv_max > 100
				|| p_event->param.link_update_req.latency > 300	)
			{
				rsp.accept=false;
			  
			}
			else
			{
				rsp.accept=true;
			}
			rsp.src_id=p_event->param.link_update_req.dst_id;
			rsp.dst_id=p_event->param.link_update_req.src_id;
			rsp.conidx=p_event->param.link_update_req.conidx;
			rsp.ce_len_max=p_event->param.link_update_req.intv_max;
			rsp.ce_len_min=p_event->param.link_update_req.intv_min;		
			gap_param_update_rsp(&rsp);	
			co_printf("gap_param_update_response \r\n");	
		}
		break;

        case GAP_EVT_MASTER_CONNECT: //As master role, link is established.
        {
						// p_event->param.master_connect.conidx is link_id, link_id is assigned by BLE stack,
						// from 0 to 19. In case of multi-connection, link_id represets different links
            if (gap_security_get_bond_status()) //return if peer device in last link is bond or not
                gap_security_enc_req(p_event->param.master_connect.conidx); // if bond, then start encryption
            else
                gap_security_pairing_req(p_event->param.master_connect.conidx); // if not bond, then start bond
        }
        break;

        case GAP_EVT_DISCONNECT:	//link is disconnect evnet
        {
            co_printf("Link[%d] disconnect,reason:0x%02X\r\n",p_event->param.disconnect.conidx
                      ,p_event->param.disconnect.reason);
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            co_printf("Link[%d]param reject,status:0x%02x\r\n"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            co_printf("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",p_event->param.link_update.conidx
                      ,p_event->param.link_update.con_interval,p_event->param.link_update.con_latency,p_event->param.link_update.sup_to);
            break;

        case GAP_EVT_CONN_END:	//connection action is end
            co_printf("conn_end,reason:0x%02x\r\n",p_event->param.conn_end_reason);
            break;

        case GAP_SEC_EVT_MASTER_ENCRYPT:  //link encryption action is over
				{
						//discovery all service groups on peer device. client_id is profile idx assigned when profile is created
            gatt_discovery_all_peer_svc(client_id,p_event->param.master_encrypt_conidx);	
					
						//if only need to discovery one service group, you can use below function 
            //uint8_t group_uuid[] = {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07};
            //gatt_discovery_peer_svc(client_id,event->param.master_encrypt_conidx,16,group_uuid);
				}
						break;
				
        default:
            break;
    }
}

/*********************************************************************
 * @fn      simple_central_start_scan
 *
 * @brief   Set central role scan parameters and start scanning BLE devices.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
static void simple_central_start_scan(void)
{
    // Start Scanning
    co_printf("Start scanning...\r\n");
    gap_scan_param_t scan_param;
    scan_param.scan_mode = GAP_SCAN_MODE_GEN_DISC;
    scan_param.dup_filt_pol = 0;
    scan_param.scan_intv = 32;  //scan event on-going time
    scan_param.scan_window = 20;	//scan windows open time
    scan_param.duration = 0;	//will not stop by self
		gap_set_link_rssi_report(true);
    gap_start_scan(&scan_param);
}

/*********************************************************************
 * @fn      simple_central_msg_handler
 *
 * @brief   Simple Central GATT message handler, handles messages from GATT layer.
 *          Messages like read/write response, notification/indication values, etc.
 *
 * @param   p_msg       - GATT message structure.
 *
 * @return  uint16_t    - Data length of the GATT message handled.
 */
static uint16_t simple_central_msg_handler(gatt_msg_t *p_msg)		//GATT event callback function, p_msg is pointer to event content
{
    switch(p_msg->msg_evt)	//event type, GATT event type is defined in gatt_msg_evt_t in "gatt_api.h"
    {
        case GATTC_MSG_NTF_REQ:	//receive notification data from peer device
        {
            if(p_msg->att_idx == 0)	//ntf data is for UUID array idx 0 attribution
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1); //printf received ntf data
            }
        }
        break;
        
        case GATTC_MSG_READ_IND:	//receive read response data from peer device.
        {
            if(p_msg->att_idx == 0)	//read reponse data is for UUID array idx 0 attribution
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);	//printf received read reponse data
            }
        }
        break;
        case GATTC_MSG_SVC_REPORT:
            {
                gatt_svc_report_t *svc_rpt = (gatt_svc_report_t *)(p_msg->param.msg.p_msg_data);
                co_printf("svc:%d,start_hdl:%d,end_hdl:%d\r\n",svc_rpt->uuid_len,svc_rpt->start_hdl,svc_rpt->end_hdl);
            }
#if 0
            if(memcmp(svc_rpt->uuid,spss_svc_uuid,16) == 0)
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
        case GATTC_MSG_CMP_EVT:	//A GATTC operation is over
        {
		    if(p_msg->param.op.operation == GATT_OP_PEER_SVC_DISC_END)
            {
                co_printf("peer svc discovery done\r\n");
#if 0
                gatt_add_client_uuid(client_att_tb,2,p_msg->conn_idx,handles);
#endif
            }
            else if(p_msg->param.op.operation == GATT_OP_PEER_SVC_REGISTERED)	//if operation is discovery serivce group on peer device
            {
                uint16_t att_handles[2];
                memcpy(att_handles,p_msg->param.op.arg,4);	//copy discoveried handles on peer device
                show_reg((uint8_t *)att_handles,4,1);	//printf handlers for UUID array idx. if 0 means handler nnumber is not not found
#if 0	
								handles[0] = att_handles[0];
								handles[1] = att_handles[1];
#endif
                gatt_client_enable_ntf_t ntf_enable;	//below code is to inform peer device that notification is enabled
                ntf_enable.conidx = p_msg->conn_idx;	//GATT event from which link_id
                ntf_enable.client_id = client_id;			//profile id assigned by BLE stack when profile is created
                ntf_enable.att_idx = 0; 							//This operation is for UUID array idx 0 attribution
                gatt_client_enable_ntf(ntf_enable);		//Start enable_ntf operation 

                gatt_client_write_t write;
                write.conidx = p_msg->conn_idx;				//GATT event from which link_id
                write.client_id = client_id;					//profile id assigned by BLE stack when profile is created
                write.att_idx = 1;										//This operation is for UUID array idx 1 attribution
                write.p_data = "\x1\x2\x3\x4\x5\x6\x7";	//Buffer to be written
                write.data_len = 7;										//buffer len
                gatt_client_write_cmd(write);					//start write opertaion

                gatt_client_read_t read;
                read.conidx = p_msg->conn_idx;				//GATT event from which link_id
                read.client_id = client_id;						//profile id assigned by BLE stack when profile is created
                read.att_idx = 0; 										//This operation is for UUID array idx 0 attribution	
                gatt_client_read(read);								//start read opertaion
            }
        }
        break;
        
        default:
        break;
    }

    return 0;
}

/*
 * EXTERN FUNCTIONS (外部函数)
 */

/*
 * PUBLIC FUNCTIONS (全局函数)
 */

/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */

/*********************************************************************
 * @fn      simple_central_init
 *
 * @brief   Initialize simple central, BLE related parameters.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void simple_central_init(void)
{
	  os_timer_init(&button_anti_shake_timer, button_anti_shake_timeout_handler, NULL);
    pmu_set_pin_pull(GPIO_PORT_D, BIT(6)|BIT(7), true);
    pmu_port_wakeup_func_set(GPIO_PD6|GPIO_PD7);
	
    // set local device name
    uint8_t local_name[] = "Simple Central";
    gap_set_dev_name(local_name, sizeof(local_name));

    gap_set_cb_func(app_gap_evt_cb);
    
    //enable bond manage module, which will record bond key and peer service info into flash. 
		//and read these info from flash when func: "gap_bond_manager_init" executes.
    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);
	
    mac_addr_t addr;
    gap_address_get(&addr);
    co_printf("Local BDADDR: 0x%2X%2X%2X%2X%2X%2X\r\n", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
    
#if 0
    gap_security_param_t param =
    {
        .mitm = true,		// use PIN code during bonding
        .ble_secure_conn = false,		//not enable security encryption
        .io_cap = GAP_IO_CAP_KEYBOARD_ONLY,		//ble device has input ability, will input pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,	//need bond
        .bond_auth = true,	//need bond auth
    };
#endif
#if 0
    gap_security_param_t param =
    {
        .mitm = true,		// use PIN code during bonding
        .ble_secure_conn = false,		//not enable security encryption
        .io_cap = GAP_IO_CAP_DISPLAY_ONLY,	//ble device has output ability, will show pin code. 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ, //need bond
        .bond_auth = true,	//need bond auth
        .password = 123456,	//set PIN code, it is a dec num between 100000 ~ 999999
    };
#endif
#if 1
    gap_security_param_t param =
    {
        .mitm = false,	// dont use PIN code during bonding
        .ble_secure_conn = false,	//not enable security encryption
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT, //ble device has neither output nor input ability, 
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,		//need bond
        .bond_auth = true,	//need bond auth
        .password = 0,
    };
#endif
		//initialize bonding configurations
    gap_security_param_init(&param);
    
    // Initialize GATT 
    gatt_client_t client;
    
#if 1
    client.p_att_tb = client_att_tb;
    client.att_nb = 2;
#else
    client.p_att_tb = NULL;
    client.att_nb = 0;
#endif

    client.gatt_msg_handler = simple_central_msg_handler;
    client_id = gatt_add_client(&client);
    
    simple_central_start_scan();
}




