#include <rtthread.h>
#include <board.h>
#include "sys_conf.h"
#include "kfifo.h"
#include "uart_com.h"
#include "app_can.h"

#define DISPLAY_APP_THREAD_STACK_SIZE        4096
#define PIXEL_CHAN_MAX  48
#define PIXEL_DIM       3

#define __BYTE_ORDER        0
#define __LITTLE_ENDIAN     0
#define __BIG_ENDIAN        1

#define PIXEL_RX_EVENT (1 << 0)
static struct rt_event pixel_event;

static uint64_t pix_mat[PIXEL_DIM];

static uint8_t 	kf_buf_raw[512*4];
kfifo_t 		kf_raw;

void kf_init(void)
{
    rt_event_init(&pixel_event, "pixel_rx_event", RT_IPC_FLAG_FIFO);
    rt_memset(&kf_raw, 0, sizeof(kf_raw));
    kfifo_init(&kf_raw, (void *)kf_buf_raw, sizeof(kf_buf_raw));
}

int16_t is_fifo_empty(void)
{
    if(kfifo_len(&kf_raw) > 0)
        return 0;
    else
        return 1;
}

uint16_t push_raw(void *data, uint16_t cnt)
{
    uint16_t ret = 0;
    ret = kfifo_in(&kf_raw,data,cnt);
    return ret;
}

uint16_t pop_raw(void *data_ptr, uint16_t cnt)
{
    uint16_t ret;
    ret = kfifo_out(&kf_raw, data_ptr,cnt);
    return ret;
}

static void reset_kfifo_raw(void)
{
    kfifo_reset(&kf_raw);
}

static void reset_px_mat(void)
{
    rt_memset(pix_mat, 0, 24);
//    for(int i=0;i<PIXEL_DIM;i++)
//    {
//        pix_mat[i] = 0;
//    }
}

uint16_t pixel_in(uint16_t chan, uint16_t pixel)
{    
    extern sys_reg_st  g_sys;
    static uint8_t      index=0;
    uint16_t i;
    if(chan == PIXEL_CHAN_MAX-1)
    {   
        for(i=0;i<PIXEL_DIM;i++)
        {
            pix_mat[i] |= ((uint64_t)pixel>>i)&0x00000001;
            pix_mat[i] &= 0x0000ffffffffffff;
            pix_mat[i] |= (uint64_t)i<<48;
            pix_mat[i] |= (uint64_t)index<<56;    
//            if(g_sys.stat.dbg == 1)
//            {
//                rt_kprintf("i:%d ",i);
//                for(int j=0;j<8;j++)
//                    rt_kprintf("%x ",*((uint8_t *)&pix_mat[i]+j));
//                rt_kprintf("\n");
//            }
        }
//        if(g_sys.stat.dbg == 1)
//            rt_kprintf("\n");
        index++;        
                
        if(is_fifo_empty())
            push_raw(pix_mat,3*sizeof(uint64_t));
        if(g_sys.stat.dbg == 1)
        {
            for(int j=0;j<24;j++)
                rt_kprintf("%x ",*((uint8_t *)&pix_mat+j));
            rt_kprintf("\n");
            g_sys.stat.dbg = 0;
        }        
        reset_px_mat();

        rt_event_send(&pixel_event, PIXEL_RX_EVENT);
    }
    else
    {
        for(int k=0;k<PIXEL_DIM;k++)
            pix_mat[k] = (pix_mat[k]|(((uint64_t)pixel>>k)&0x00000001))<<1;
    }
        
    return 0;
}

static void display_app_entry(void *parameter)
{
    extern sys_reg_st  g_sys;
    rt_err_t ret = 0;
    rt_uint32_t e;
    uint16_t i;
    uint64_t temp[3];
    
    kf_init();
    
    for(i=0;i<PIXEL_DIM;i++)
    {
        pix_mat[i] = 0;
    }
    
    while (1)
    {
        ret = rt_event_recv(&pixel_event, PIXEL_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
        if (RT_EOK == ret)
        {
            if(kfifo_len(&kf_raw) > 0)
            {
                pop_raw(temp,3*sizeof(uint64_t));
                reset_kfifo_raw();                
                for(i=0;i<3;i++)
                {              
                    can_sendmsg((uint8_t *)&temp[i],8);
                }
//                if(g_sys.stat.dbg == 1)
//                {
//                    for(int k=0;k<24;k++)
//                        rt_kprintf("%x ",*((uint8_t *)&temp+k));
//                    g_sys.stat.dbg = 0;
//                }
            }
        }
//        }
    }
}

static int display_app(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("display", 
                            display_app_entry, 
                            RT_NULL,
                            DISPLAY_APP_THREAD_STACK_SIZE, 
                            12, 
                            20
                          );
    
    RT_ASSERT(tid != RT_NULL);

    rt_thread_startup(tid);
    return 0;
}

INIT_APP_EXPORT(display_app);
