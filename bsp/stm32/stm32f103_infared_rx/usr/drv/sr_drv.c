#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "amux_drv.h"
#include "kfifo.h"
#include "display.h"

#define SR_D        GET_PIN(A, 6)
#define SR_CLK      GET_PIN(A, 3)
#define SR_CLR_N    GET_PIN(A, 5)

#define HWT_THL     16
#define HWT_THH     120

#define HWTIMER_DEV_NAME   "timer2"     /* 定时器名称 */

static rt_device_t hw_dev = RT_NULL;   /* 定时器设备句柄 */

typedef struct
{
    uint16_t    tim_low_lim;
    uint16_t    tim_high_lim;
    uint16_t	hwt_cnt;
    uint16_t	sr_reg[8];		
}sr_st;
static sr_st sr_inst;


/* 定时器超时回调函数 */
static rt_err_t hwt_cb(rt_device_t dev, rt_size_t size)
{
    uint16_t pos,off;
    uint16_t chan;
    
    chan = sr_inst.hwt_cnt>>1;
    
    if(chan > sr_inst.tim_low_lim)
    {
        if(chan >= sr_inst.tim_high_lim)
        {
//            rt_pin_write(SR_CLR_N,PIN_HIGH);
//            rt_pin_write(SR_CLK,PIN_LOW);
//            rt_pin_write(SR_D,PIN_LOW);
            sr_inst.hwt_cnt = 0;
        }
        else
        {
            if((chan == sr_inst.tim_low_lim)&&((sr_inst.hwt_cnt&0x0001) == 1))
                rt_pin_write(SR_CLR_N,PIN_LOW);
            else if(chan > sr_inst.tim_low_lim)
                rt_pin_write(SR_CLR_N,PIN_HIGH);
            else
            {
                rt_pin_write(SR_CLK,PIN_LOW);
                rt_pin_write(SR_D,PIN_LOW);
            }                
            sr_inst.hwt_cnt++;
        }
    }
    else
    {
        if((sr_inst.hwt_cnt&0x0001) == 0)
        {
            rt_pin_write(SR_CLK,PIN_LOW);
        }
        else
        {      
            if(chan > 0)
                pixel_in(chan-1, get_dig_val());
            
            pos = chan>>4;
            off = (chan)&0x000f;
            if((sr_inst.sr_reg[pos]>>off)&0x0001)
                rt_pin_write(SR_D,PIN_HIGH);
            else
                rt_pin_write(SR_D,PIN_LOW);
            
            amux_set_chan(chan);
            rt_pin_write(SR_CLK,PIN_HIGH);            
            
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
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_STOP, &mode);
    return ret;
}

static void sr_drv_init(void)
{
    rt_pin_mode(SR_D,PIN_MODE_OUTPUT);
    rt_pin_mode(SR_CLK,PIN_MODE_OUTPUT);
    rt_pin_mode(SR_CLR_N,PIN_MODE_OUTPUT);
    
    rt_pin_write(SR_D,PIN_LOW);
    rt_pin_write(SR_CLK,PIN_LOW);
    rt_pin_write(SR_CLR_N,PIN_HIGH);
    
    hwt_init();
    sr_hwt_start(0,50);
    rt_kprintf("sr_drv initialized!\n");
}


static void sr_ds_init(void)
{
    uint16_t i;
    sr_inst.hwt_cnt = 0;
    
    sr_inst.tim_low_lim = HWT_THL;
    sr_inst.tim_high_lim = HWT_THH;
    for(i=0;i<8;i++)
        sr_inst.sr_reg[i] = 0;
    sr_inst.sr_reg[0] = 1;
}

static void hwt_set_lim(int argc, char**argv)
{
    if (argc < 3)
    {
        rt_kprintf("Please input'hwt_start <low_lim> <high_lim>'\n");
        return;
    }
    sr_inst.tim_low_lim = atoi(argv[1]); 
    sr_inst.tim_high_lim = atoi(argv[2]);
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

static int hwt_app_init(void)
{
    sr_drv_init();
    sr_ds_init();
    return 0;
}


INIT_APP_EXPORT(hwt_app_init);

MSH_CMD_EXPORT(hwt_start, start hwtimer);
MSH_CMD_EXPORT(hwt_stop, stop hwtimer);
MSH_CMD_EXPORT(hwt_set_lim, set hwtimer limits);
