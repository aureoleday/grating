#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "sys_conf.h"
#include "amux_drv.h"
#include "addr_drv.h"
#include "sr_drv.h"

#define SR_D        GET_PIN(A, 6)
#define SR_CLK      GET_PIN(A, 3)
#define SR_CLR_N    GET_PIN(A, 5)

#define SR_SYNC_TE    GET_PIN(B, 12)
#define SR_RST_TE     GET_PIN(B, 10)

#define HWTIMER_DEV_NAME   "timer2"     /* 定时器名称 */

static rt_device_t hw_dev = RT_NULL;   /* 定时器设备句柄 */

typedef struct
{
    uint16_t	hwt_cnt;
    uint16_t	sr_reg[8];		
}sr_st;
static sr_st sr_inst;


/* 定时器超时回调函数 */
static rt_err_t hwt_cb(rt_device_t dev, rt_size_t size)
{
    extern sys_reg_st  g_sys;
    uint16_t chan;
    
    chan = sr_inst.hwt_cnt>>1;
    
    if(chan >= g_sys.conf.hwt_thl)
    {        
        if(chan >= g_sys.conf.hwt_thh)
        {            
            sr_inst.hwt_cnt = 0;
            rt_pin_write(SR_D,PIN_HIGH);
        }
        else
        {
            rt_pin_write(SR_D,PIN_LOW);
            if(chan == (g_sys.conf.hwt_thl+2))
            {
                rt_pin_write(SR_CLR_N,PIN_LOW);
            }
            else
                rt_pin_write(SR_CLR_N,PIN_HIGH);
            sr_inst.hwt_cnt++;
        }        
        rt_pin_write(SR_CLK,PIN_LOW);
        
    }
    else
    {
        if((sr_inst.hwt_cnt&0x0001) == 0)
        {
            rt_pin_write(SR_CLK,PIN_HIGH);
        }
        else
        {
            if(chan >= (g_sys.conf.on_bits-1))
                rt_pin_write(SR_D,PIN_LOW);
            rt_pin_write(SR_CLK,PIN_LOW);            
        }            
        sr_inst.hwt_cnt++;
    }
    return 0;
}

static int hwt_init(void)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimer_mode_t mode;         /* 定时器模式 */
    
    /* 查找定时器设备 */
    hw_dev = rt_device_find(HWTIMER_DEV_NAME);
    if (hw_dev == RT_NULL)
    {
        rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
        return RT_ERROR;
    } 
    
    /* 以读写方式打开设备 */
    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
        return ret;
    }

    /* 设置超时回调函数 */ 
    rt_device_set_rx_indicate(hw_dev, hwt_cb);
    
    /* 设置模式为周期性定时器 */
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        rt_kprintf("set mode failed! ret is :%d\n", ret);       
    }     
    return ret;
}

int16_t sr_hwt_start(int16_t sec,int16_t u_sec)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
    /* 设置定时器超时值为u_sec us并启动定时器 */
    timeout_s.sec = sec;      /* 秒 */
    timeout_s.usec = u_sec;     /* 微秒 */

    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        rt_kprintf("set timeout value failed\n");
        return RT_ERROR;
    } 
    return ret;
}

int16_t sr_hwt_stop(void)
{
    rt_err_t ret = RT_EOK;
    rt_hwtimer_mode_t mode;         /* 定时器模式 */
    rt_pin_write(SR_CLR_N,PIN_LOW);
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_STOP, &mode);
    return ret;
}

static void sr_drv_init(uint16_t addr)
{
    extern sys_reg_st  g_sys;
    rt_pin_mode(SR_D,PIN_MODE_OUTPUT);
    rt_pin_mode(SR_CLK,PIN_MODE_OUTPUT);
    rt_pin_mode(SR_CLR_N,PIN_MODE_OUTPUT);
    
    rt_pin_mode(SR_SYNC_TE,PIN_MODE_OUTPUT);
    rt_pin_mode(SR_RST_TE,PIN_MODE_OUTPUT);
    
    rt_pin_write(SR_D,PIN_LOW);
    rt_pin_write(SR_CLK,PIN_LOW);
    rt_pin_write(SR_CLR_N,PIN_HIGH);
    
    if(addr == 0)
    {
        rt_pin_write(SR_CLR_N,PIN_LOW);
        rt_pin_write(SR_SYNC_TE,PIN_HIGH);
        rt_pin_write(SR_RST_TE,PIN_HIGH);
        hwt_init();
        if(g_sys.conf.power_en == 1)
            sr_hwt_start(0,g_sys.conf.sync_period);
    }
    else
    {
        rt_pin_write(SR_SYNC_TE,PIN_LOW);
        rt_pin_write(SR_RST_TE,PIN_LOW);    
    }    
    rt_kprintf("sr_drv initialized!\n");
}


static void sr_ds_init(void)
{
    extern sys_reg_st  g_sys;
    uint16_t i;
    sr_inst.hwt_cnt = 0;    
    
    for(i=0;i<8;i++)
        sr_inst.sr_reg[i] = 0;
    sr_inst.sr_reg[0] = 1;
}

static void hwt_set_lim(int argc, char**argv)
{
    extern sys_reg_st  g_sys;
    if (argc < 3)
    {
        rt_kprintf("Please input'hwt_start <low_lim> <high_lim>'\n");
        return;
    }
    g_sys.conf.hwt_thl = atoi(argv[1]); 
    g_sys.conf.hwt_thh = atoi(argv[2]);
}

static void hwt_start(int argc, char**argv)
{
    if (argc < 3)
    {
        rt_kprintf("Please input'hwt_start <sec> <usec>'\n");
        return;
    }
    sr_hwt_start(atoi(argv[1]),atoi(argv[2]));
}

static void hwt_stop(int argc, char**argv)
{
    sr_hwt_stop();
}

int sr_init(void)
{
    extern sys_reg_st  g_sys;
    
    sr_drv_init(g_sys.stat.lc_addr);
    if(g_sys.stat.lc_addr == 0)
    {
//        sr_drv_init();        
        sr_ds_init();
        rt_kprintf("master, start sr logic\n");
        return 0;
    }
    else
    {
        rt_kprintf("slave: %d, sr logic halted\n",g_sys.stat.lc_addr);
        return 2;    
    }
}

MSH_CMD_EXPORT(hwt_start, start hwtimer);
MSH_CMD_EXPORT(hwt_stop, stop hwtimer);
MSH_CMD_EXPORT(hwt_set_lim, set hwtimer limits);

