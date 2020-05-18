#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "sys_conf.h"

#define AUTOADDR_APP_THREAD_STACK_SIZE        1024

#define CUP0    GET_PIN(B, 3)
#define CUP1    GET_PIN(B, 4)
#define CDN0    GET_PIN(B, 6)
#define CDN1    GET_PIN(B, 5)

#define RUP0    GET_PIN(B, 7)
#define RUP1    GET_PIN(B, 8)
#define RDN0    GET_PIN(B, 13)
#define RDN1    GET_PIN(B, 9)

#define FSM_INIT  0x0
#define FSM_PREP  0x1
#define FSM_SYNC  0x2
#define FSM_PUSH  0x3
#define FSM_SUCC  0x4

typedef struct
{
    uint8_t sr;
    uint8_t rxd;
    uint8_t txd;
    uint8_t cnt;
    uint8_t fsm;
    uint8_t auto_addr;
}autoaddr_st;

autoaddr_st autoaddr_row_inst;
autoaddr_st autoaddr_col_inst;

static struct rt_timer last_timer_row;
static struct rt_timer last_timer_col;

static void last_timer_row_cb(void *parameter)
{
    rt_pin_write(RUP0,1);
    rt_pin_mode(RUP0, PIN_MODE_INPUT_PULLUP);
}

static void last_timer_col_cb(void *parameter)
{
    rt_pin_write(CUP0,1);
    rt_pin_mode(CUP0, PIN_MODE_INPUT_PULLUP);
}

static void autoaddr_row_rx_cb(void *args)
{    
    rt_pin_mode(RUP0, PIN_MODE_INPUT_PULLUP);
  
    if(rt_pin_read(RUP0) == 0)
        autoaddr_row_inst.sr = autoaddr_row_inst.sr<<1;
    else
        autoaddr_row_inst.sr = (autoaddr_row_inst.sr<<1) + 1;    
    autoaddr_row_inst.cnt++;

    if(autoaddr_row_inst.cnt>=8)
    {
        autoaddr_row_inst.rxd = autoaddr_row_inst.sr>>2;
        autoaddr_row_inst.sr = 0;
        autoaddr_row_inst.cnt = 0;
        rt_pin_mode(RUP0, PIN_MODE_OUTPUT);
        rt_pin_write(RUP0,1);
        rt_timer_start(&last_timer_row);
    }
    else if(autoaddr_row_inst.cnt==7)
    {
        rt_pin_mode(RUP0, PIN_MODE_OUTPUT);
        rt_pin_write(RUP0,0);
        rt_timer_start(&last_timer_row);
    }
}

static void autoaddr_col_rx_cb(void *args)
{    
    rt_pin_mode(CUP0, PIN_MODE_INPUT_PULLUP);
  
    if(rt_pin_read(CUP0) == 0)
        autoaddr_col_inst.sr = autoaddr_col_inst.sr<<1;
    else
        autoaddr_col_inst.sr = (autoaddr_col_inst.sr<<1) + 1;    
    autoaddr_col_inst.cnt++;

    if(autoaddr_col_inst.cnt>=8)
    {
        autoaddr_col_inst.rxd = autoaddr_col_inst.sr>>2;
        autoaddr_col_inst.sr = 0;
        autoaddr_col_inst.cnt = 0;
        rt_pin_mode(CUP0, PIN_MODE_OUTPUT);
        rt_pin_write(CUP0,1);
        rt_timer_start(&last_timer_col);
    }
    else if(autoaddr_col_inst.cnt==7)
    {
        rt_pin_mode(CUP0, PIN_MODE_OUTPUT);
        rt_pin_write(CUP0,0);
        rt_timer_start(&last_timer_col);
    }
}

static void inaddr_init(void)
{
    rt_pin_attach_irq(RUP1, PIN_IRQ_MODE_FALLING, autoaddr_row_rx_cb, RT_NULL);
    rt_pin_attach_irq(CUP1, PIN_IRQ_MODE_FALLING, autoaddr_col_rx_cb, RT_NULL);
    rt_pin_irq_enable(RUP1, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(CUP1, PIN_IRQ_ENABLE);
}

static uint8_t autoaddr_row_tx(uint8_t data)
{
    uint8_t i;
    uint8_t temp = (data<<2)|0x03;
    uint8_t ret=0;
    uint8_t tx_buf = 0;
    
    for(i=0;i<8;i++)
    {
        rt_pin_write(RDN1, (temp>>(7-i))&0x1);
        rt_thread_mdelay(1);
        rt_pin_write(RDN0, 0);
        rt_thread_mdelay(1);
        ret = rt_pin_read(RDN1);
        tx_buf = (tx_buf<<1)|ret;
        rt_pin_write(RDN0, 1);        
    }
    autoaddr_row_inst.txd = tx_buf;
    return ret;
}

static uint8_t autoaddr_col_tx(uint8_t data)
{
    uint8_t i;
    uint8_t temp = (data<<2)|0x03;
    uint8_t ret=0;
    uint8_t tx_buf = 0;
    
    for(i=0;i<8;i++)
    {
        rt_pin_write(CDN1, (temp>>(7-i))&0x1);
        rt_thread_mdelay(1);
        rt_pin_write(CDN0, 0);
        rt_thread_mdelay(1);
        ret = rt_pin_read(CDN1);
        tx_buf = (tx_buf<<1)|ret;
        rt_pin_write(CDN0, 1);        
    }
    autoaddr_col_inst.txd = tx_buf;
    return ret;
}


uint8_t drv_autoaddr_get_row_addr(void)
{
    return autoaddr_row_inst.auto_addr;
}

uint8_t drv_autoaddr_get_col_addr(void)
{
    return autoaddr_col_inst.auto_addr;
}

uint8_t drv_autoaddr_get_row_fsm(void)
{
    return autoaddr_row_inst.fsm;
}

uint8_t drv_autoaddr_get_col_fsm(void)
{
    return autoaddr_col_inst.fsm;
}

static void addr_out_test(int argc, char**argv)
{
    uint8_t ret;
    if (argc < 2)
    {
        rt_kprintf("Please input'addr_out_test <data>'\n");
        return;
    }
    
    ret = autoaddr_row_tx(atoi(argv[1]));
    if((autoaddr_col_inst.txd&0x03) == 0x01)
        rt_kprintf("col wr ok\n",ret);
    else
        rt_kprintf("col wr fail\n",ret);
}

static void addr_init(void)
{
    rt_pin_mode(RDN0, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(RDN1, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(RUP0, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(RUP1, PIN_MODE_INPUT_PULLUP);
    
    rt_pin_mode(CDN0, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(CDN1, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(CUP0, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(CUP1, PIN_MODE_INPUT_PULLUP);    
    
    rt_pin_write(RDN0, 1);
    rt_pin_write(RDN1, 1);
    rt_pin_write(CDN0, 1);
    rt_pin_write(CDN1, 1);    
  
    inaddr_init();
  
    autoaddr_row_inst.cnt = 0;
    autoaddr_row_inst.sr = 0;
    autoaddr_row_inst.rxd = 0;
    autoaddr_row_inst.txd = 0;
    autoaddr_row_inst.fsm = FSM_INIT;
    autoaddr_row_inst.auto_addr = 0;
    
    autoaddr_col_inst.cnt = 0;
    autoaddr_col_inst.sr = 0;
    autoaddr_col_inst.rxd = 0;
    autoaddr_col_inst.txd = 0;
    autoaddr_col_inst.fsm = FSM_INIT;
    autoaddr_col_inst.auto_addr = 0;    
  
    rt_timer_init(&last_timer_row, "last_time_row",
                  last_timer_row_cb, 
                  RT_NULL, 
                  2, 
                  RT_TIMER_FLAG_ONE_SHOT); 
                  
    rt_timer_init(&last_timer_col, "last_time_col",
                  last_timer_col_cb, 
                  RT_NULL, 
                  2, 
                  RT_TIMER_FLAG_ONE_SHOT);                   
}

static void autoaddr_info(void)
{
    uint8_t ri_sts,ro_sts,ci_sts,co_sts;
    ri_sts = rt_pin_read(RUP0);
    ro_sts = rt_pin_read(RUP1);
    ci_sts = rt_pin_read(CUP0);
    co_sts = rt_pin_read(CUP1);
    rt_kprintf("local addr(row,col):(%d,%d)\n",autoaddr_row_inst.auto_addr,autoaddr_col_inst.auto_addr);
    rt_kprintf("row info:\n");
    rt_kprintf("sr\trxd\ttxd\tcnt\tfsm\tauto_addr\tUP_I\tUP_O\t\n");
    rt_kprintf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t\t0x%x\t0x%x\t\n",autoaddr_row_inst.sr,autoaddr_row_inst.rxd,autoaddr_row_inst.txd,autoaddr_row_inst.cnt,autoaddr_row_inst.fsm,autoaddr_row_inst.auto_addr,ri_sts,ro_sts);
    
    rt_kprintf("col info:\n");
    rt_kprintf("sr\trxd\ttxd\tcnt\tfsm\tauto_addr\tUP_I\tUP_O\t\n");
    rt_kprintf("0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t0x%x\t\t0x%x\t0x%x\t\n",autoaddr_col_inst.sr,autoaddr_col_inst.rxd,autoaddr_col_inst.txd,autoaddr_col_inst.cnt,autoaddr_col_inst.fsm,autoaddr_col_inst.auto_addr,ci_sts,co_sts);    
}

//void drv_led_toggle(uint8_t period)
//{
//    static uint8_t timeout=0;
//    if(timeout >= (period<<1))
//    {
//        rt_pin_write(LED_PIN, PIN_HIGH);
//        timeout=0;
//    }
//    else if(timeout == period)
//    {
//        rt_pin_write(LED_PIN, PIN_LOW);
//        timeout++;
//    }
//    else
//        timeout++;
//}

void autoaddr_row_thread_entry(void* parameter)
{    
	rt_thread_delay(200);    
    autoaddr_row_inst.fsm = FSM_INIT;
    while(1)
    {
        switch(autoaddr_row_inst.fsm)
        {
            case FSM_INIT:
            {                
                autoaddr_row_inst.fsm = FSM_PREP;
                rt_thread_mdelay(20);
                break;
            }
            case FSM_PREP:
            {
                if((rt_pin_read(RUP0) == 0)&&(rt_pin_read(RUP1) == 0))
                {
                    autoaddr_row_inst.auto_addr = 1;
                    rt_pin_irq_enable(RUP1, PIN_IRQ_DISABLE);
                    autoaddr_row_inst.fsm = FSM_PUSH;
                }
                else
                {
                    autoaddr_row_inst.fsm = FSM_SYNC;
                }
                rt_thread_mdelay(20);
                break;
            }
            case FSM_SYNC:
            {
                if(autoaddr_row_inst.rxd != 0)
                {
                    autoaddr_row_inst.auto_addr = autoaddr_row_inst.rxd;
                    rt_pin_irq_enable(RUP1, PIN_IRQ_DISABLE);
                    autoaddr_row_inst.fsm = FSM_PUSH;
                }
                else
                {
                    autoaddr_row_inst.cnt = 0;
                    autoaddr_row_inst.sr = 0;
                    autoaddr_row_inst.rxd = 0;
                    autoaddr_row_inst.fsm = FSM_SYNC;   
                }
                rt_thread_mdelay(100);
                break;
            }
            case FSM_PUSH:
            {
                autoaddr_row_tx(autoaddr_row_inst.auto_addr+8);
                if((autoaddr_row_inst.txd&0x03) != 0x01)
                    autoaddr_row_inst.fsm = FSM_PUSH;
                else
                    autoaddr_row_inst.fsm = FSM_SUCC;  
                rt_thread_mdelay(100);
                break;
            }
            case FSM_SUCC:
            {
                autoaddr_row_tx(autoaddr_row_inst.auto_addr+8);
                rt_thread_mdelay(1000);
                break;
            }
            default:
            {
                autoaddr_row_inst.auto_addr = 0;
                autoaddr_row_inst.fsm = FSM_INIT;
                rt_thread_mdelay(10);
                break;
            }
        }
        rt_thread_mdelay(100);
    }
}

void autoaddr_col_thread_entry(void* parameter)
{    
	rt_thread_delay(200);    
    autoaddr_col_inst.fsm = FSM_INIT;
    while(1)
    {
        switch(autoaddr_col_inst.fsm)
        {
            case FSM_INIT:
            {                
                autoaddr_col_inst.fsm = FSM_PREP;
                rt_thread_mdelay(20);
                break;
            }
            case FSM_PREP:
            {
                if((rt_pin_read(CUP0) == 0)&&(rt_pin_read(CUP1) == 0))
                {
                    autoaddr_col_inst.auto_addr = 1;
                    rt_pin_irq_enable(CUP1, PIN_IRQ_DISABLE);
                    autoaddr_col_inst.fsm = FSM_PUSH;
                }
                else
                {
                    autoaddr_col_inst.fsm = FSM_SYNC;
                }
                rt_thread_mdelay(20);
                break;
            }
            case FSM_SYNC:
            {
                if(autoaddr_col_inst.rxd != 0)
                {
                    autoaddr_col_inst.auto_addr = autoaddr_col_inst.rxd;
                    rt_pin_irq_enable(CUP1, PIN_IRQ_DISABLE);
                    autoaddr_col_inst.fsm = FSM_PUSH;
                }
                else
                {
                    autoaddr_col_inst.cnt = 0;
                    autoaddr_col_inst.sr = 0;
                    autoaddr_col_inst.rxd = 0;
                    autoaddr_col_inst.fsm = FSM_SYNC;   
                }
                rt_thread_mdelay(100);
                break;
            }
            case FSM_PUSH:
            {
                autoaddr_col_tx(autoaddr_col_inst.auto_addr+1);
                if((autoaddr_col_inst.txd&0x03) != 0x01)
                    autoaddr_col_inst.fsm = FSM_PUSH;
                else
                    autoaddr_col_inst.fsm = FSM_SUCC;  
                rt_thread_mdelay(100);
                break;
            }
            case FSM_SUCC:
            {
                autoaddr_col_tx(autoaddr_col_inst.auto_addr+1);
                rt_thread_mdelay(1000);
                break;
            }
            default:
            {
                autoaddr_col_inst.auto_addr = 0;
                autoaddr_col_inst.fsm = FSM_INIT;
                rt_thread_mdelay(10);
                break;
            }
        }
        rt_thread_mdelay(100); 
    }
}


static int auto_addr_app(void)
{
    rt_thread_t tid;
    addr_init();
    
//    tid = rt_thread_create("auto_addr_row", 
//                            autoaddr_row_thread_entry, 
//                            RT_NULL,
//                            AUTOADDR_APP_THREAD_STACK_SIZE, 
//                            24, 
//                            20
//                          );
//    
//    RT_ASSERT(tid != RT_NULL);

//    rt_thread_startup(tid);
    
    tid = rt_thread_create("auto_addr_col", 
                            autoaddr_col_thread_entry, 
                            RT_NULL,
                            AUTOADDR_APP_THREAD_STACK_SIZE, 
                            25, 
                            20
                          );
    
    RT_ASSERT(tid != RT_NULL);

    rt_thread_startup(tid);    
    return 0;
}

INIT_APP_EXPORT(auto_addr_app);
MSH_CMD_EXPORT(autoaddr_info, show autoaddr info)
MSH_CMD_EXPORT(addr_out_test, auto addr test)
