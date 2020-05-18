#include "rtthread.h"
#include "rtdevice.h"
#include "sys_conf.h"
#include "addr_drv.h"
#include "uart_com.h"

#define MAT_ROW_MAX 8
#define MAT_COL_MAX 18

static uint8_t com_tx_buf[256];

/* can接收事件标志 */
#define CAN_RX_EVENT (1 << 0)
static rt_device_t can_device = RT_NULL;
static struct rt_event can_event;

typedef struct
{
    uint8_t    bmp_mat0[MAT_ROW_MAX][MAT_COL_MAX];
    uint8_t    bmp_mat1[MAT_ROW_MAX][MAT_COL_MAX];
    uint32_t   mat_bitmap[2];
    uint8_t    stage;
}bmp_st;

bmp_st bmp_inst;

static uint8_t xor_cs(uint8_t *d_ptr,uint16_t cnt)
{
    uint16_t i;
    uint8_t res = 0;
    for(i=0;i<cnt;i++)
        res = res^(*(d_ptr+i));
    return res;
}

static void frame_up(void)
{
    uint8_t *src_ptr = NULL;
    com_tx_buf[0] = 0x9b;
    com_tx_buf[1] = 0xdf;
    com_tx_buf[2] = 144;
    
    if(bmp_inst.stage == 0)
        src_ptr = &bmp_inst.bmp_mat1[0][0];
    else
        src_ptr = &bmp_inst.bmp_mat0[0][0];
    rt_memcpy(&com_tx_buf[3],src_ptr,144);
    
    com_tx_buf[147] = xor_cs(com_tx_buf,147);
    
    com_tx((uint8_t *)com_tx_buf,148);
}

static void bmp_info(void)
{
    uint16_t i,j;
    rt_kprintf("bmat[0,1]:%x,%x\n",bmp_inst.mat_bitmap[0],bmp_inst.mat_bitmap[1]);
    if(bmp_inst.stage == 1)
    {
        rt_kprintf("mat0:\n");
        for(j=0;j<MAT_ROW_MAX;j++)
        {
            for(i=0;i<MAT_COL_MAX;i++)
            {
                rt_kprintf("%2x ",bmp_inst.bmp_mat0[j][i]);
            }
            rt_kprintf("\n");
        }
    }
    else
    {
        rt_kprintf("mat1:\n");
        for(j=0;j<MAT_ROW_MAX;j++)
        {
            for(i=0;i<MAT_COL_MAX;i++)
            {
                rt_kprintf("%2x ",bmp_inst.bmp_mat1[j][i]);
            }
            rt_kprintf("\n");
        }
    }
}

static int16_t bmp_data_in(struct rt_can_msg *can_din)
{
    int16_t ret = 0;
    uint16_t base,offset,i;
    uint8_t *dptr = NULL;
    base = can_din->id;
    offset = can_din->data[6];
    
    if(bmp_inst.stage == 0)
    {
        if((bmp_inst.mat_bitmap[0] & 0x0001<<(base*3+offset)) != 0)
        {
            dptr = &bmp_inst.bmp_mat1[base][offset*6];
            bmp_inst.stage = 1;
            bmp_inst.mat_bitmap[1] = 0x0001<<(base*3+offset);
            ret = 1;
        }
        else
        {
            dptr = &bmp_inst.bmp_mat0[base][offset*6];
            bmp_inst.mat_bitmap[0] |= 0x0001<<(base*3+offset);
            if(bmp_inst.mat_bitmap[0] == 0x00ffffff)
                ret = 2;
        }        
    }
    else
    {
        if((bmp_inst.mat_bitmap[1] & 0x0001<<(base*3+offset)) != 0)
        {
            dptr = &bmp_inst.bmp_mat0[base][offset*6];
            bmp_inst.stage = 0;
            bmp_inst.mat_bitmap[0] = 0x0001<<(base*3+offset);
            ret = 1;
        }
        else
        {
            dptr = &bmp_inst.bmp_mat1[base][offset*6];
            bmp_inst.mat_bitmap[1] |= 0x0001<<(base*3+offset);
            if(bmp_inst.mat_bitmap[1] == 0x00ffffff)
                ret = 2;
        }  
    }
    
    for(i=0;i<6;i++)
        *(dptr + i) = can_din->data[i];
    
    return ret;
}

static rt_err_t can_rx_ind(rt_device_t device, rt_size_t len)
{
    extern sys_reg_st  g_sys;
    if(g_sys.stat.lc_addr == g_sys.conf.com_addr)
        rt_event_send(&can_event, CAN_RX_EVENT);
    return RT_EOK;
}

static rt_err_t can_open()
{
    can_device = rt_device_find("can1");
    if (RT_NULL == can_device)
    {
        rt_kprintf("can1 device not found!\n");
        return -RT_ERROR;
    }

    /* 初始化事件对象 */
    rt_event_init(&can_event, "can_rx_event", RT_IPC_FLAG_FIFO);
    /* 设置回调函数发送事件 */
    rt_device_set_rx_indicate(can_device, can_rx_ind);
    return rt_device_open(can_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
}

void print_can_msg(struct rt_can_msg *data)
{
    rt_kprintf("\n");
    if (RT_CAN_STDID == data->ide)
    {
        rt_kprintf("STDID=%03X ", data->id);
    }
    else if (RT_CAN_EXTID == data->ide)
    {
        rt_kprintf("EXTID=%08X ", data->id);
    }
    rt_kprintf("HDR=%02X ", data->hdr);
    if (RT_CAN_DTR == data->rtr)
    {
        rt_kprintf("TYPE=DATA LEN=%02X ", data->len);
        for (int i = 0; i < data->len; ++i)
        {
            rt_kprintf("%02X ", data->data[i]);
        }
    }
    else if (RT_CAN_RTR == data->rtr)
    {
        rt_kprintf("TYPE=RMOT LEN=%02X ", data->len);
    }
}

rt_err_t can_write(struct rt_can_msg *data, rt_size_t len)
{
    return rt_device_write(can_device, 0, data, len);
}

rt_err_t can_read(struct rt_can_msg *data, rt_size_t len)
{
    extern sys_reg_st  g_sys;
    rt_err_t ret = 0;
    rt_uint32_t e;
    while (1)
    {
        ret = rt_event_recv(&can_event, CAN_RX_EVENT, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
        if (RT_EOK != ret)
        {
                    
        }
        else
        {
//            rt_device_read(can_device, 0, data, len);
//            print_can_msg(data);
            while(rt_device_read(can_device, 0, data, len))
            {
                if(bmp_data_in(data) > 0)
                {
                    frame_up();
                }
            }
        }
    }
}

static void can_thread_entry(void *param)
{
    struct rt_can_msg msg =
    {
        .id = 1,
        .ide = RT_CAN_STDID,
        .rtr = RT_CAN_DTR,
        .len = 8,
        .data = {0, 1, 2, 3, 4, 5, 6, 7}
    };

    for (;;)
    {
        can_read(&msg, sizeof(struct rt_can_msg));
    }
}

#define CAN_RX_THREAD_PRIORITY  15
int can_thread_init(void)
{
    rt_thread_mdelay(500);
    if (RT_EOK != can_open()) return -RT_ERROR;
    rt_thread_t can_thread = rt_thread_create("can", can_thread_entry, RT_NULL, 1024, CAN_RX_THREAD_PRIORITY, 10);
    if (RT_NULL != can_thread)
    {
        rt_thread_startup(can_thread);
    }
    return -RT_ERROR;
}
INIT_APP_EXPORT(can_thread_init);

rt_size_t can_sendmsg(const rt_uint8_t *data, const rt_uint32_t len)
{
    extern sys_reg_st  g_sys;
    struct rt_can_msg msg = {0};
    msg.id = g_sys.stat.lc_addr;
    msg.ide = RT_CAN_STDID;
    msg.rtr = 0;
    msg.len = len;
    rt_memcpy(msg.data, data, len);

    return rt_device_write(can_device, 0, &msg, sizeof(msg));
}

rt_size_t can_sendmsg_addr(const rt_uint8_t *data, const rt_uint32_t len, uint16_t addr)
{
    extern sys_reg_st  g_sys;
    struct rt_can_msg msg = {0};
    msg.id = addr;
    msg.ide = RT_CAN_STDID;
    msg.rtr = 0;
    msg.len = len;
    rt_memcpy(msg.data, data, len);

    return rt_device_write(can_device, 0, &msg, sizeof(msg));
}

int can_write_test(void)
{
    uint8_t txd[16];
    for(int i=0;i<16;i++)
        txd[i] = i;
    can_sendmsg(txd, 8);
    return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(can_write_test, test can write);
MSH_CMD_EXPORT(bmp_info, show bmp info);

