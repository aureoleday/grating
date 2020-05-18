#include "dac_drv.h"
#include "sr_drv.h"

uint16_t ref_set_opt(uint16_t pram)
{
    dac_set(pram);
    
    return 1;
}


uint16_t sys_reset_opt(uint16_t pram)
{  
    if(pram == 9527)
        NVIC_SystemReset();
    return 1;
}

