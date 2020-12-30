/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
#ifndef HID_CLIENT_H
#define HID_CLIENT_H

#include <stdint.h>
 /*
 * INCLUDES (包含头文件)
 */

/*
 * MACROS (宏定义)
 */
#define HID_CLT_NUM_REPORTS (6)
#define HID_SVC_UUID "\x12\x18"

/*
 * CONSTANTS (常量定义)
 */
#define HID_REPORT_TYPE_INPUT       0x01
#define HID_REPORT_TYPE_OUTPUT      0x02
#define HID_REPORT_TYPE_FEATURE     0x03
#define HID_REPORT_TYPE_WR          0x10    /// Input report with Write capabilities

/*
 * TYPEDEFS (类型定义)
 */
enum
{
    HID_CLT_INFO_IDX,                   // HID Information characteristic
    HID_CLT_CONTROL_POINT_IDX,          // HID Control Point characteristic
    HID_CLT_REPORT_MAP_IDX,             // HID Report Map characteristic
    HID_CLT_PROTOCOL_MODE_IDX,          // HID Protocol Mode characteristic
    HID_CLT_BOOT_KEY_IN_IDX,            // HID Boot Keyboard Input Report
    HID_CLT_BOOT_KEY_OUT_IDX,           // HID Boot Keyboard Output Report
    HID_CLT_BOOT_MOUSE_IN_IDX,          // HID Boot Mouse Input Report
    HID_CLT_FEATURE_IDX,                // Feature Report
    HID_CLT_REPORT_REF_FEATURE_IDX,
    HID_CLT_FEATURE_CCCD_IDX,
    HID_CLT_ATT_NB = (HID_CLT_FEATURE_IDX + HID_CLT_NUM_REPORTS*3),
};

// HID report mapping table
typedef struct
{
    ///Report ID, refer to report map.
    uint8_t report_id;
    ///Report Type, define report id type, see HID Report type
    uint8_t report_type;
} hid_report_ref_t;

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */

/*
 * LOCAL FUNCTIONS (本地函数)
 */

/*
 * EXTERN FUNCTIONS (外部函数)
 */
extern uint8_t hid_client_id;
/*
 * PUBLIC FUNCTIONS (全局函数)
 */
void hid_client_create(void);
void hid_client_read(uint8_t conidx,uint8_t att_idx);
void hid_client_write_cmd(uint8_t conidx, uint8_t att_idx, uint8_t *p_data, uint16_t len);
void hid_client_write_req(uint8_t conidx, uint8_t att_idx, uint8_t *p_data, uint16_t len);

#endif // end of #ifndef HID_CLIENT_H

