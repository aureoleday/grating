#include "sys_conf.h"
#include "dac_drv.h"
#include "sr_drv.h"

uint16_t power_opt(uint16_t pram)
{
    extern sys_reg_st  g_sys;
    if(g_sys.stat.ms_status == 1)
    {
        if(pram == 0)
            sr_hwt_stop();
        else
            sr_hwt_start(0,g_sys.conf.sync_period);
    }
    else
    {
        
    }
    return 1;
}

uint16_t ref_set_opt(uint16_t pram)
{
    dac_set(pram);
    
    return 1;
}

uint16_t sync_period_opt(uint16_t pram)
{
    sr_hwt_stop();
    rt_thread_delay(1);
    sr_hwt_start(0,pram);    
    return 1;
}


uint16_t sys_reset_opt(uint16_t pram)
{  
    extern sys_reg_st  g_sys; 
    if(g_sys.stat.ms_status == 1)
    {
        sr_hwt_stop();
        rt_thread_delay(1);
    }
    if(pram == 9527)
        NVIC_SystemReset();
    return 1;
}

