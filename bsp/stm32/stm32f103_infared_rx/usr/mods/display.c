#include <rtthread.h>
#include <board.h>
#include "kfifo.h"

#define DISPLAY_APP_THREAD_STACK_SIZE        4096
#define PIXEL_DIM       3
#define PIXEL_NUM       4

static uint32_t pix_mat[PIXEL_DIM][PIXEL_NUM];

//static uint8_t 	kf_buf_raw[128*4];
//kfifo_t 		kf_raw;

//void kf_init(void)
//{
//    rt_memset(&kf_raw, 0, sizeof(kf_raw));
//    kfifo_init(&kf_raw, (void *)kf_buf_raw, sizeof(kf_buf_raw));
//}

//uint16_t push_raw(uint16_t data)
//{
//    uint16_t temp = data;
//    uint16_t ret = 0;
//    ret = kfifo_in(&kf_raw,&temp,sizeof(uint8_t));
//    return ret;
//}

//uint16_t pop_raw(uint16_t *data_ptr, uint16_t cnt)
//{
//    uint16_t ret;
//    ret = kfifo_out(&kf_raw, data_ptr,cnt*sizeof(uint16_t));
//    return ret;
//}

uint16_t pixel_in(uint16_t chan, uint16_t pixel)
{    
    uint16_t i,quo,rem;
    rem = chan%32;
    quo = chan/32;
    for(i=0;i<PIXEL_DIM;i++)
    {
        pix_mat[i][quo] |= ((pixel>>i)&0x0001)<<rem;
    }
    if(chan >= 15)
    {
//        rt_kprintf("pix_mat %x\n",pix_mat[0][0]);
    }
}

static void display_app_entry(void *parameter)
{
    uint16_t i,j;
//    uint16_t rx_buf[32],len,i;
//    kf_init();
    for(i=0;i<PIXEL_DIM;i++)
    {
        for(j=0;j<PIXEL_NUM;j++)
            pix_mat[i][j] = 0;
    }    
    while (1)
    {

//        len = pop_raw(rx_buf,4*sizeof(uint16_t))/sizeof(uint16_t);
//        rt_kprintf("len:%d,data:\n",len);
//        for(i=0;i<len;i++)
//            rt_kprintf(" %x ",rx_buf[i]);
//        rt_kprintf("\n");
//        
        rt_thread_delay(100);
    }
}

static int display_app(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("display", 
                            display_app_entry, 
                            RT_NULL,
                            DISPLAY_APP_THREAD_STACK_SIZE, 
                            RT_MAIN_THREAD_PRIORITY, 
                            20
                          );
    
    RT_ASSERT(tid != RT_NULL);

    rt_thread_startup(tid);
    return 0;
}

INIT_APP_EXPORT(display_app);
