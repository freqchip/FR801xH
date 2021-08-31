/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef GAP_SERVICE_H
#define GAP_SERVICE_H

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
// GAP Server Profile attributes index.
enum
{

    IDX_GAP_SERVICE,            // GAP_IDX_PRIM_SVC - GAP service

    IDX_GAP_CHAR_DEVNAME,       // GAP_IDX_CHAR_DEVNAME - device name declaration
    IDX_GAP_DEVNAME,            // GAP_IDX_DEVNAME - device name definition
    IDX_GAP_CHAR_ICON,          // GAP_IDX_CHAR_ICON - appearance declaration
    IDX_GAP_ICON,               // GAP_IDX_ICON -appearance
    IDX_GAP_CHAR_SLAVE_PREF_PARAM,     // GAP_IDX_CHAR_SLAVE_PREF_PARAM - Peripheral parameters declaration
    IDX_GAP_SLAVE_PREF_PARAM,       // GAP_IDX_SLAVE_PREF_PARAM - Peripheral parameters definition
    IDX_GAP_CHAR_CNT_ADDR_RESOL,     // GAP_IDX_CHAR_ADDR_RESOL - Central Address Resolution declaration
    IDX_GAP_CNT_ADDR_RESOL,        // GAP_IDX_ADDR_RESOL_SUPP - Central Address Resolution supported
    IDX_GAP_CHAR_RSLV_PRIV_ADDR_ONLY,    // GAP_IDX_CHAR_RSLV_PRIV_ADDR_ONLY - Resolvable Private Address Only declaration
    IDX_GAP_RSLV_PRIV_ADDR_ONLY,     // GAP_IDX_RSLV_PRIV_ADDR_ONLY - If Central Address is Resolvable Private Address Only 

    IDX_GAP_NB,
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
 * @fn      gap_gatt_add_service
 *
 * @brief   Create gap server.
 *
 * @param   None.
 * 
 * @return  None.
 */
void gap_gatt_add_service(void);

#endif







