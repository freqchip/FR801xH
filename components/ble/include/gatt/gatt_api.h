/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef GATT_API_H
#define GATT_API_H

/*
* INCLUDES 
*/
#include <stdint.h>
#include <stdbool.h>
#include "os_msg_q.h"

/*
 * MACROS 
 */

/*
 * CONSTANTS 
 */
#define MAX_PEER_SVC_NUM    (10)
#define MAX_CLEINT_ATT_NUM  (50)

#define UUID_SIZE_2         2                   //!< 2 bytes UUID, usually SIG assigned UUID.
#define UUID_SIZE_16        16                  //!< 16 bytes UUID, usually assigned by users. 

/** @defgroup GATT_PROP_BITMAPS_DEFINES GATT Attribute Access Permissions Bit Fields
 * @{
 */
#define GATT_PROP_BROADCAST             (1<<0)  //!< Attribute is able to broadcast
#define GATT_PROP_READ                  (1<<1)  //!< Attribute is Readable
#define GATT_PROP_WRITE_CMD             (1<<2)  //!< Attribute supports write with no response
#define GATT_PROP_WRITE_REQ             (1<<3)  //!< Attribute supports write request
#define GATT_PROP_NOTI                  (1<<4)  //!< Attribute is able to send notification
#define GATT_PROP_INDI                  (1<<5)  //!< Attribute is able to send indication
#define GATT_PROP_AUTH_SIG_WRTIE        (1<<6)  //!< Attribute supports authenticated signed write
#define GATT_PROP_EXTEND_PROP           (1<<7)  //!< Attribute supports extended properities
#define GATT_PROP_WRITE                 (1<<8)  //!< Attribute is Writable (support both write_req and write_cmd)
#define GATT_PROP_AUTHEN                (1<<9)  //!< Attribute requires Authentication

/** @} End GATT_PERMIT_BITMAPS_DEFINES */

/** @defgroup GATT_OPERATION_NAME GATT operation name define, used with GATTS/C_MSG_CMP_EVT
 * @{
 */
#define GATT_OP_NOTIFY                  0x01    //!< GATT notification operation
#define GATT_OP_INDICA                  0x02    //!< GATT indication operation
#define GATT_OP_PEER_SVC_REGISTERED     0x03    //!< Used with GATTC_CMP_EVT, GATT peer device service registered
#define GATT_OP_WRITE_REQ               0x05    //!< GATT wirte request operation
#define GATT_OP_WRITE_CMD               0x06    //!< GATT wirte command operation, write without response
#define GATT_OP_READ                    0x07    //!< GATT read operation
#define GATT_OP_PEER_SVC_DISC_END       0x08    //!< Used with GATTC_CMP_EVT, GATT peer device service discovery is ended  

/** @} End GATT_OPERATION_NAME */


#define GATT_SVC_ID_FAIL                0xff



/*
 * TYPEDEFS 
 */
/// Service Discovery Attribute type
enum svc_att_type
{
    ATT_TYPE_NONE,         /// No Attribute Information
    ATT_TYPE_INC_SVC,      /// Included Service
    ATT_TYPE_CHAR_DECL,     /// Characteristic Declaration
    ATT_TYPE_VAL,      /// Characteristic Attribute Value
    ATT_TYPE_DESC,     /// Characteristic Attribute Descriptor
};

enum svc_change_type
{
    SVC_CHANGE_UUID,   /// change uuid 
    SVC_CHANGE_PERM,   /// change property 
};

typedef struct
{
    uint8_t att_type;           /// should be ATT_TYPE_INC_SVC ,Included Service Information
    uint8_t uuid_len;           /// Included Service UUID
    uint8_t  uuid[UUID_SIZE_16];    /// Included Service UUID
    uint16_t start_hdl;         /// Included service Start Handle
    uint16_t end_hdl;           /// Included service End Handle
} inc_svc_t;

/// Information about attribute characteristic
typedef struct
{
    uint8_t att_type;       /// should be ATT_TYPE_CHAR_DECL: Characteristic Declaration
    uint8_t prop;           /// Value property, see @defgroup GATT_PROP_BITMAPS_DEFINES
    uint16_t handle;        /// Value Handle
} char_decl_t;

/// Information about attribute
typedef struct
{
    uint8_t  att_type;      /// should be ATT_TYPE_VAL: Attribute Value OR  ATT_TYPE_DESC: Attribute Descriptor
    uint8_t  uuid_len;      /// Attribute UUID Length
    uint8_t  uuid[UUID_SIZE_16];    /// Attribute UUID
} att_value_t;

/// Attribute information
union attr_info
{
    uint8_t att_type;       /// Attribute Type
    char_decl_t char_decl;    /// Information about attribute characteristic,valid when att_type == ATT_TYPE_CHAR_DECL
    inc_svc_t inc_svc;  /// Information about included service, valid when att_type == ATT_TYPE_INC_SVC
    att_value_t att_value;              /// Information about attribute, valid when att_type == ATT_TYPE_VAL or att_type == ATT_TYPE_DESC
};

/**
* Service Discovery indicate that a service has been found.
*/
typedef struct
{
    uint8_t  uuid_len;      /// Service UUID Length
    uint8_t  uuid[UUID_SIZE_16];    /// Service UUID
    uint16_t start_hdl;     /// Service start handle
    uint16_t end_hdl;       /// Service end handle
    union attr_info info[];  /// attribute information present in the service. (Nnumber of info = end_hdl - start_hdl)
} gatt_svc_report_t;


/**
* GATT message events type define
*/
typedef enum
{
    GATTC_MSG_READ_REQ,         //!< GATT client read request
    GATTC_MSG_WRITE_REQ,        //!< GATT client write request
    GATTC_MSG_ATT_INFO_REQ,     //!< GATT client att information request
    GATTC_MSG_NTF_REQ,          //!< GATT client notification received
    GATTC_MSG_IND_REQ,          //!< GATT client indication received
    GATTC_MSG_READ_IND,         //!< GATT client read response received
    GATTC_MSG_CMP_EVT,          //!< GATT client message complete event
    GATTC_MSG_LINK_CREATE,      //!< A Link is established
    GATTC_MSG_LINK_LOST,        //!< A Link is lost
    GATTC_MSG_SVC_REPORT,       //!< GATT client received peer service report
} gatt_msg_evt_t;

/**
* GATT UUID format.
*/
typedef struct
{
    uint8_t size;               //!< Length of UUID (2 or 16 bytes UUIDs: UUID_SIZE_2 or UUID_SIZE_16). 
    uint8_t p_uuid[16];         //!< Pointer to uuid, could be 2 or 16 bytes array. 
} gatt_uuid_t;

/**
* BLE attribute define format.
*/
typedef struct
{
    gatt_uuid_t     uuid;       //!< Attribute UUID
    uint16_t        prop;       //!< Attribute properties, see @GATT_PROP_BITMAPS_DEFINES
    uint16_t        max_size;   //!< Attribute data maximum size
    uint8_t         *p_data;    //!< Attribute data pointer
} gatt_attribute_t;

/**
* GATT message event operation done structer
*/
typedef struct
{
    uint8_t operation;          //!< GATT request type
    uint8_t status;             //!< Status of the request
    void * arg;                 //!< Parameter pointer
} gatt_op_cmp_t;

/**
* GATT message event data structer.
*/
typedef struct
{
    uint16_t    msg_len;        //!< GATT message length
    uint8_t        *p_msg_data;    //!< GATT message data pointer
} gatt_msg_hdl_t;

/**
* GATT message event structure.
*/
typedef struct
{
    gatt_msg_evt_t  msg_evt;    //!< The event that message come with
    uint8_t         conn_idx;   //!< Connection index
    uint8_t         svc_id;     //!< service id of this message
    uint16_t        att_idx;    //!< Attribute index of in the service table
    uint16_t        handle;     //!< Attribute handle number in peer service
    union
    {
        gatt_msg_hdl_t  msg;    //!< GATT message, length, data pointer
        gatt_op_cmp_t   op;     //!< GATT operation, read, write, notification, indication
    } param;
} gatt_msg_t;

/**
* attributor and uuid change structure for created service.
*/
typedef struct
{
    uint8_t svc_id;     //!< service id of this svc changes
    uint8_t att_idx;    //!< Attribute index of this svc changes
    enum svc_change_type type;  //!< Type of this svc changes
    union
    {
        gatt_uuid_t new_uuid;       //!< New Attribute UUID
        uint16_t    new_prop;       //!< New Attribute properties, see @GATT_PROP_BITMAPS_DEFINES
    } param;
} svc_change_t;


/// service handles
struct svc_hdl
{
    /// start handle
    uint16_t shdl;
    /// end handle
    uint16_t ehdl;
};

/**
* For client, peer service group info, which will be stored to or loaded from flash. This structor is for func: gatt_client_direct_register
*/
typedef struct
{
    uint8_t svc_grp_idx;                            //!< Peer service group number. 
    uint8_t client_info_att_nb;                     //!< Actual attributor number the client profile will use. 
    struct svc_hdl svc[MAX_PEER_SVC_NUM];           //!< The start and end handles of each peer service group 
    uint16_t link_chars_hdl[MAX_CLEINT_ATT_NUM];    //!< The handles of actual attributor, which the client profile will use. 
    uint16_t link_descs_hdl[MAX_CLEINT_ATT_NUM];    //!< The handles of actual attributor configuration, which the client profile will use. 
} gatt_stored_peer_svc_info_t;

/*********************************************************************
 * @fn      gatt_msg_handler
 *
 * @brief   User application handles read request in this callback.
 *
 * @param   p_read  - the pointer to read buffer. NOTE: It's just a pointer from lower layer, please create the buffer in application layer.
 *          len     - the pointer to the length of read buffer. Application to assign it.
 *          att_idx - offset of the attribute value.
 *
 * @return  Data length.
 */
typedef uint16_t (*gatt_msg_handler_t)(gatt_msg_t *p_msg);

/**
* Profile service define format.
*/
typedef struct
{
    const gatt_attribute_t  *p_att_tb;          //!< Service's attributes table to add to system attribute database. 
    uint8_t                 att_nb;             //!< Service's attributes number. 
    gatt_msg_handler_t      gatt_msg_handler;   //!< Read request callback function. 
} gatt_service_t;

/**
* Profile client define format.
*/
typedef struct
{
    const gatt_uuid_t       *p_att_tb;          //!< Service UUID
    uint8_t                 att_nb;             //!< Service's attributes number. 
    uint8_t                 att_tb_initialized; //!< Service's attributes is initialized by gatt_add_client(). 
    gatt_msg_handler_t      gatt_msg_handler;   //!< Read request callback function. 
} gatt_client_t;

/**
* BLE client write format.
*/
typedef struct
{
    uint8_t     conidx;         //!< Connection index
    uint8_t     client_id;      //!< Service ID among all services in current system. 
    uint8_t     att_idx;        //!< Attribute id number in its service attribute table. 
    uint8_t     *p_data;        //!< Data pointer to be written
    uint16_t    data_len;       //!< Data length to be written
} gatt_client_write_t;

/**
* BLE client read format.
*/
typedef struct
{
    uint8_t     conidx;         //!< Connection index
    uint8_t     client_id;      //!< Service ID among all services in current system. 
    uint8_t     att_idx;        //!< Attribute id number in its service attribute table. 
} gatt_client_read_t;

/**
* BLE client read by uuid format.
*/
typedef struct
{
    uint8_t     conidx;         //!< Connection index
    uint8_t     client_id;      //!< Service ID among all services in current system. 
    gatt_uuid_t uuid;        //!< Attribute id number in its service attribute table. 
} gatt_client_read_by_uuid_t;

/**
* BLE client enable ntf format.
*/
typedef struct
{
    uint8_t     conidx;         //!< Connection index
    uint8_t     client_id;      //!< Service ID among all services in current system. 
    uint8_t     att_idx;        //!< Attribute id number in its service attribute table. 
} gatt_client_enable_ntf_t;


/**
* BLE notification format.
*/
typedef struct
{
    uint8_t     conidx;         //!< Connection index
    uint8_t     svc_id;         //!< Service ID among all services in current system. 
    uint8_t     att_idx;        //!< Attribute id number in its service attribute table. 
    uint8_t     *p_data;        //!< Data to be sent as notification.
    uint16_t    data_len;       //!< Data length
} gatt_ntf_t;

/**
* BLE indication format.
*/
typedef struct
{
    uint8_t     conidx;         //!< Connection index
    uint8_t     svc_id;         //!< Service ID among all services in current system. 
    uint8_t     att_idx;        //!< Attribute id number in its service attribute table. 
    uint8_t     *p_data;        //!< Data to be sent as indication.
    uint16_t    data_len;       //!< Data length
} gatt_ind_t;


/*
 * GLOBAL VARIABLES 
 */

/*
 * LOCAL VARIABLES 
 */

/*
 * LOCAL FUNCTIONS 
 */

/*
 * EXTERN FUNCTIONS 
 */

/*
 * PUBLIC FUNCTIONS 
 */

/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */

/*********************************************************************
 * @fn      gatt_add_service
 *
 * @brief   Addding a services & characteristics into gatt database.
 *
 * @param   p_service - service data to be added.
 *
 * @return  Assigned svc_id.
 */
uint8_t gatt_add_service(gatt_service_t *p_service);

/*********************************************************************
 * @fn      gatt_add_client
 *
 * @brief   Addding a GATT client in the system.
 *
 * @param   p_client - client information.
 *
 * @return  Assigned client_id.
 *
 example1 :
    const gatt_uuid_t client_att_tb[] =
    {
        [0]  =
        { UUID_SIZE_16, SPSC_UUID128_ARR_TX},
        [1]  =
        { UUID_SIZE_16, SPSC_UUID128_ARR_RX},
    };
    func()
    {
        gatt_client_t client;
        client.p_att_tb = client_att_tb;
        client.att_nb = 2;
        client.gatt_msg_handler = client_msg_handler;
    }
  example2 :   //in this case, there will no be uuids being registed.
    func()
    {
        gatt_client_t client;
        client.p_att_tb = NULL;
        client.att_nb = 0;
        client.gatt_msg_handler = client_msg_handler;
    }
 */
uint8_t gatt_add_client(gatt_client_t *p_client);

/*********************************************************************
 * @fn      gatt_add_client_uuid
 *
 * @brief   Addding a GATT client uuid in the system if no uuid array is given when gatt_add_client() is called 
            Normally this function is called after gatt event: GATTC_MSG_CMP_EVT and op: GATT_OP_PEER_SVC_DISC_END
 *
 * @param   att_tb - uuid array which will be added into client profile
 *          att_nb - size of uuid array.
 *          conn_idx - current link conidx.
 *          handles - handle array corresponding to att_tb[] array .
            
 * @return  None.
 */
void gatt_add_client_uuid(gatt_uuid_t att_tb[], uint8_t att_nb, uint8_t conn_idx, uint16_t handles[]);

/*********************************************************************
 * @fn      gatt_delete_svc
 *
 * @brief   Delete services which is created already
 *          svc_uuid - pointer to services uuid buffer.
 *          uuid_size - services uuid size.

 * @param   None
 *
 * @return  None.

   example :   //in this case, delete gap & gatt services.
   func()
   {
       uint16_t svc_uuid = 0x1801;
       gatt_delete_svc((uint8_t *)&svc_uuid,2);
       svc_uuid = 0x1800;
       gatt_delete_svc((uint8_t *)&svc_uuid,2);
   }
 */
void gatt_delete_svc(uint8_t *svc_uuid,uint8_t uuid_size);

/*********************************************************************
 * @fn      gatt_get_svc_hdl
 *
 * @brief   Gat att start handle & end handle of services, which is created already
 *
 * @param   svc_uuid - pointer to services uuid buffer.
 *          uuid_size - services uuid size.
 *          svc_shdl - service first att handle. 
 *          svc_ehdl - service last att handle. 
 *
 * @return  0, successful.  -1, fail to find service

   example :   //in this case, get start handle and end handle of gap services.
   func()
   {
       uint16_t svc_uuid = 0x1801;
       uint16_t gap_svc_shdl = 0;
       uint16_t gap_svc_ehdl = 0;
       gatt_get_svc_hdl((uint8_t *)&svc_uuid,2,&gap_svc_shdl,,&gap_svc_ehdl);
   }
 */
int gatt_get_svc_hdl(uint8_t *svc_uuid,uint8_t uuid_size,uint16_t *svc_shdl,uint16_t *svc_ehdl);

/*********************************************************************
 * @fn      gatt_change_svc
 *
 * @brief   Change service UUID Or Property after service is added.
 *
 * @param   svc_change - svc attributor new settings. detail setting refer to type svc_change_t
 *
 * @return  None.
 */
void gatt_change_svc(svc_change_t svc_change);

/*********************************************************************
 * @fn      gatt_change_client_uuid
 *
 * @brief   Change client UUID.
 *
 * @param   client_id   - client information.
 *          att_idx     -
 *          new_uuid    -
 *          uuid_len    -
 *
 * @return  None.
 */
void gatt_change_client_uuid(uint8_t client_id,uint8_t att_idx,uint8_t *new_uuid,uint8_t uuid_len);

/*********************************************************************
 * @fn      gatt_discovery_all_peer_svc
 *
 * @brief   Discover peer device service.
 *
 * @param   client_id  - client information.
 *          conidx     -
 *
 * @return  None.
 */
void gatt_discovery_all_peer_svc(uint8_t client_id,uint8_t conidx);

/*********************************************************************
 * @fn      gatt_discovery_peer_svc
 *
 * @brief   Discover peer device service.
 *
 * @param   client_id   - client information.
 *          conidx      -
 *          uuid_len    -
 *          group_uuid  -
 *
 * @return  None.
 */
void gatt_discovery_peer_svc(uint8_t client_id, uint8_t conidx, uint8_t uuid_len, uint8_t *group_uuid);


/*********************************************************************
 * @fn      gatt_client_direct_register
 *
 * @brief   No need to discover peer device service, if peer svc info is know, use this func directly regist peer servcie group.
 *
 * @param   client_id   - client information.
 *          conidx      -
 *          uuid_len    -
 *          group_uuid  -
 *
 * @return  None.
 example:
 
    case GAP_EVT_MASTER_CONNECT:
    {
        gatt_stored_peer_svc_info_t peer_svc
        peer_svc.svc_grp_idx = 2;
        peer_svc.svc[0].shdl = 0x10;
        peer_svc.svc[0].ehdl = 0x14;
        peer_svc.svc[1].shdl = 0x16;
        peer_svc.svc[1].ehdl = 0x1A;
        peer_svc.client_info_att_nb = 3;
        peer_svc.link_chars_hdl[0] = 0x10;
        peer_svc.link_chars_hdl[1] = 0x13;
        peer_svc.link_chars_hdl[2] = 0x14;
        gatt_client_direct_register(event->param.master_connect.conidx,peer_svc);
    }
    break;
 */
void gatt_client_direct_register(uint8_t conidx,gatt_stored_peer_svc_info_t peer_svc);

/*********************************************************************
 * @fn      gatt_client_write_req
 *
 * @brief   Write request, with response.
 *
 * @param   write_att   - write operation parameter.
 *
 * @return  None.
 */
void gatt_client_write_req(gatt_client_write_t write_att);

/*********************************************************************
 * @fn      gatt_client_write_cmd
 *
 * @brief   Write request, without response.
 *
 * @param   gatt_client_write_t   - client information.
 *
 * @return  None.
 */
void gatt_client_write_cmd(gatt_client_write_t write_att);
void gatt_client_write_cmd_with_handle(gatt_client_write_t write_att,uint16_t handle);
/*********************************************************************
 * @fn      gatt_client_enable_ntf
 *
 * @brief   Enable notification.
 *
 * @param   gatt_client_enable_ntf_t   - client information.
 *
 * @return  None.
 */
void gatt_client_enable_ntf(gatt_client_enable_ntf_t ntf_enable_att);

/*********************************************************************
 * @fn      gatt_client_enable_ntf_ind
 *
 * @brief   Enable notification / indication. 
 *
 * @param   gatt_client_enable_ntf_t   - client information.
 *          ntf - true, notification is enalbe;     false,  notification is disalbe;         
 *          ind - true, indication is enalbe;   false,  indication is disalbe;    
 *
 * @return  None.
 */
void gatt_client_enable_ntf_ind(gatt_client_enable_ntf_t ntf_enable_att,bool ntf, bool ind);

/*********************************************************************
 * @fn      gatt_client_read
 *
 * @brief   Read request.
 *
 * @param   gatt_client_read_t   - client information.
 *
 * @return  None.
 */
void gatt_client_read(gatt_client_read_t read_att);

/*********************************************************************
 * @fn      gatt_client_read_by_uuid
 *
 * @brief   Read request by uuid, you can read att value with this function directly after link is established
 *          , no need to discovery peer service to get handles.
 *
 * @param   gatt_client_read_by_uuid_t   - client read info.
 *
 * @return  None.
 example
     case GAP_EVT_SLAVE_CONNECT:
     {
         gatt_client_read_by_uuid_t read_att =
        {
            .conidx = 0,
            .client_id = client_id,
            .uuid.size = 2,
            .uuid.p_uuid = "\x00\x2A",
        };
        gatt_client_read_by_uuid(read_att);
    }
    break;
 */
void gatt_client_read_by_uuid(gatt_client_read_by_uuid_t read_att);

/*********************************************************************
 * @fn      gatt_notification
 *
 * @brief   Sending notification.
 *          发送notification.
 *
 * @param   ntf_att - in which service and which attribute the notification will be sent
 *                    This parameter contains servcie ID and attribute ID
 *                    to indicate exact which attribute will be used to send notification.
 *          p_data - notification data to be sent.
 *          data_len - length of notification data.
 *
 * @return  None.
 */
void gatt_notification(gatt_ntf_t ntf_att);

/*********************************************************************
 * @fn      gatt_indication
 *
 * @brief   Sending indication.
 *
 * @param   ind_att - in which service and which attribute the indication will be sent
 *                    This parameter contains servcie ID and attribute ID
 *                    to indicate exact which attribute will be used to send indication.
 *          p_data - indication data to be sent.
 *          data_len - length of indication data.
 *
 * @return  None.
 */
void gatt_indication(gatt_ind_t ind_att);

/*********************************************************************
 * @fn      gatt_mtu_exchange_req
 *
 * @brief   Change maximum MTU.
 *
 * @param   conidx - Connection index.
 *
 * @return  None.
 */
void gatt_mtu_exchange_req(uint8_t conidx);

/****************************************************************************************
 * @fn      gatt_get_mtu
 *
 * @brief   Gets the negotiated MTU. This function gets the negotiated MTU.
 *
 * @param   conidx  - Link idx.
 *
 * @return  MTU negotiated
 */
uint16_t gatt_get_mtu(uint8_t conidx);

/****************************************************************************************
 * @fn      gatt_svc_changed_req
 *
 * @brief   Send svc changed indication pkt through svc_change attributor in GATT profile.
 *          Normaly, master device will discovery services again after receiving this pkt.
 *
 * @param   conidx  - Link idx.
 *          shdl - start handle of changed svc.
 *          ehdl - end handle of changed svc.
 *
 * @return  MTU negotiated
 */
void gatt_svc_changed_req(uint8_t conidx,uint16_t shdl,uint16_t ehdl);

/****************************************************************************************
 * @fn      gatt_msg_default
 *
 * @brief   Used to deal gatt message received from host layer when RTOS is enabled.
 *          When RTOS is enabled, user has to create a independent task used to deal GATT
 *          messages. The message is generated by HOST layer, and consumed by this task.
 *          And this function is entity of the task to deal incoming message.
 *
 * @param   msg - message to be dealed.
 *
 * @return  None
 */
void gatt_msg_default(os_event_t *msg);

/****************************************************************************************
 * @fn      gatt_get_task_no_from_prf_id
 *
 * @brief   Used to get task number from profile index
 *
 * @param   prf_idx - profile index.
 *
 * @return  task number which is corresponding to the profile
 */
uint16_t gatt_get_task_no_from_prf_id(uint8_t prf_idx);
typedef void (*gatt_rx_with_handle)(uint8_t conidx,uint16_t handle,uint8_t *rec_data,uint16_t len);
extern gatt_rx_with_handle gatt_recv_with_handle ;
#endif // end of #ifndef GATT_API_H
