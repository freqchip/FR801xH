/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef GATTM_SERVICE_H
#define GATTM_SERVICE_H

/*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"


/*
 * MACROS (宏定义)
 */

/*
 * CONSTANTS (常量定义)
 */
// GATTM Server Profile attributes index.
enum
{
    IDX_GATTM_SERVICE,            // GATT_IDX_PRIM_SVC - GATT service

    IDX_GATTM_CHAR_SVC_CHANGED,       // GATT_IDX_CHAR_SVC_CHANGED - Service Changed declaration
    IDX_GATTM_SVC_CHANGED,            // GATT_IDX_SVC_CHANGED - Service Changed definition
    IDX_GATTM_SVC_CHANGED_CFG,        // GATT_IDX_SVC_CHANGED_CFG - Service Changed Client Characteristic Configuration Descriptor
    
    IDX_GATTM_NB,
};

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */

/*
 * LOCAL VARIABLES (本地变量)
 */


/*
 * PUBLIC FUNCTIONS (全局函数)
 */
 /*********************************************************************
 * @fn      gattm_gatt_add_service
 *
 * @brief   Create gatt server.
 *
 * @param   None.
 * 
 * @return  None.
 */
void gattm_gatt_add_service(void);

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
void gattm_svc_changed_indicaiton(uint8_t conidx,uint16_t svc_shdl,uint16_t svc_ehdl);



#endif







