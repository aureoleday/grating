/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-08     balanceTWK   first version
 */

#ifndef __SYS_CONF__
#define __SYS_CONF__

#include <rtthread.h>
#include "sys_def.h"
//#define CONF_REG_MAP_NUM		16
//#define STAT_REG_MAP_NUM      8


typedef struct
{
    uint16_t    power_en;
    uint16_t    sync_period;
    uint16_t    hwt_thl;
    uint16_t    hwt_thh;
    uint16_t    dac_ref;
    uint16_t    on_bits;
    uint16_t    sample_delay;
    uint16_t    dbg;
}config_st;

typedef struct
{    
    uint16_t    sw_ver;
    uint16_t    hw_ver;
    uint16_t    sn;
    uint16_t    man_date;
    uint16_t    ms_status;      //0:uninit;1:master;2:slave
    uint16_t    lc_addr;        //upper4bit: row; lower4bits:col;
    uint16_t    dbg;
}status_st;

typedef struct
{
    config_st     conf;
    status_st     stat;	
}sys_reg_st;

typedef struct 
{
    uint16_t    id;
    uint16_t*   reg_ptr;
    int16_t     min;
    uint16_t    max;
    uint16_t    dft;
    uint8_t	    type;                       //0:RW, 1:WO
    uint16_t    (*chk_ptr)(uint16_t pram);
}conf_reg_map_st;

typedef struct 
{
    uint16_t 	id;
    uint16_t*	reg_ptr;
    uint16_t    dft;
}sts_reg_map_st;

void gvar_init(void);
uint16 reg_map_write(uint16_t reg_addr, uint16_t *wr_data, uint8_t wr_cnt);
uint16 reg_map_read(uint16_t reg_addr,uint16_t* reg_data,uint8_t read_cnt);
#endif //__SYS_CONF__
