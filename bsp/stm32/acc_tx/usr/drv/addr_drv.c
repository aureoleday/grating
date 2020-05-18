#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "sys_conf.h"

#define ADDR0        GET_PIN(B,3)
#define ADDR1        GET_PIN(B,4)
#define ADDR2        GET_PIN(B,7)
#define ADDR3        GET_PIN(B,8)

uint8_t get_local_addr(void)
{
    uint8_t addr = 0;
    addr = (rt_pin_read(ADDR3)<<3)|(rt_pin_read(ADDR2)<<2)|(rt_pin_read(ADDR1)<<1)|rt_pin_read(ADDR0);
    return addr;
}

int local_addr_init(void)
{
    extern sys_reg_st  g_sys;
    rt_pin_mode(ADDR0,PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(ADDR1,PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(ADDR2,PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(ADDR3,PIN_MODE_INPUT_PULLUP);    
    return 0;
}



static void addr_info(void)
{
    uint8_t addr = get_local_addr();
    rt_kprintf("addr: %d\n",addr);
}


MSH_CMD_EXPORT(addr_info, show autoaddr info)
