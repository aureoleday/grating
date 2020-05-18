#include "rtthread.h"
#include "rtdevice.h"
#include "sys_conf.h"
#include "addr_drv.h"
#include "uart_com.h"

#define CAN_CONF_FLAG       0x100
#define CAN_RESP_FLAG       0x200
#define CAN_BROADCAST_FLAG  0x80

#define MAT_ROW_MAX 8
#define MAT_COL_MAX 18

//static uint8_t com_tx_buf[256];

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

static rt_err_t can_rx_ind(rt_device_t device, rt_size_t len)
{
    extern sys_reg_st  g_sys;
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

//    struct rt_can_filter_item items[1] =
//    {
//        RT_CAN_FILTER_ITEM_INIT(0x01, 0, 0, 1, 0x1ff),
//        /* 过滤 ID 为 0x01,match ID:0x100~0x1ff，hdr 为 - 1，设置默认过滤表 */
//    };
//    struct rt_can_filter_config cfg = {1, 1, items}; /* 一共有 1 个过滤表 */
//    /* 设置硬件过滤表 */
//    rt_device_control(can_device, RT_CAN_CMD_SET_FILTER, &cfg);    
    
    /* 初始化事件对象 */
    rt_event_init(&can_event, "can_rx_event", RT_IPC_FLAG_FIFO);
    /* 设置回调函数发送事件 */
    rt_device_set_rx_indicate(can_device, can_rx_ind);
    return rt_device_open(can_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
}

static rt_size_t can_sendmsg_resp(const rt_uint8_t *data, const rt_uint32_t len)
{
    extern sys_reg_st  g_sys;
    struct rt_can_msg msg = {0};
    msg.id = g_sys.stat.lc_addr|CAN_RESP_FLAG;
    msg.ide = RT_CAN_STDID;
    msg.rtr = 0;
    msg.len = len;
    rt_memcpy(msg.data, data, len);

    return rt_device_write(can_device, 0, &msg, sizeof(msg));
}

void can_frame_up(uint8_t rw_flag, uint16_t reg_addr, uint16_t reg_data, uint8_t err_no)
{
    uint8_t data[8];
    uint8_t tx_len;

    data[0] = rw_flag;
    data[1] = reg_addr;    
    
    if((rw_flag == 0)||(rw_flag == 2))
    {
        if(err_no != REGMAP_ERR_NOERR)
        {
            data[2] = 0;
            data[3] = 0;
        }
        else
        {
            data[2] = reg_data>>8;
            data[3] = reg_data&0x00ff;            
        }
        data[4] = err_no;
        tx_len = 5;
    }
    else
    {
        data[2] = err_no;
        tx_len = 3;
    }    
    can_sendmsg_resp(data, tx_len);
}

void resolve_can_msg(struct rt_can_msg *data)
{
    extern sys_reg_st  g_sys;
    uint16_t reg_data,err_no,can_id;
    uint16_t broadcast_addr,unicast_addr;
    broadcast_addr = CAN_CONF_FLAG | CAN_BROADCAST_FLAG;
    unicast_addr = CAN_CONF_FLAG | g_sys.stat.lc_addr;
    can_id = data->id;

    if ((can_id == broadcast_addr)|(can_id == unicast_addr))
    {
        if(data->data[0] == 0)  //read conf reg
        {
            err_no = reg_map_read(data->data[1],&reg_data,1);   
            rt_kprintf("can rd conf:addr: %d, %d, %d\n",data->data[1],reg_data,err_no);
        }
        else if(data->data[0] == 2)  //read status reg
        {
            err_no = reg_map_read((data->data[1])|0x8000,&reg_data,1);   
            rt_kprintf("can rd status:addr: %d, %d, %d\n",data->data[1],reg_data,err_no);
        }
        else
        {
            reg_data = (data->data[2]<<8)|data->data[3];
            err_no = reg_map_write(data->data[1],&reg_data,1);
            rt_kprintf("can wr:addr: %d, %d, %d\n",data->data[1],reg_data,err_no);
        }
        can_frame_up(data->data[0], data->data[1], reg_data, err_no);
    }
}

void print_can_msg(struct rt_can_msg *data)
{
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
            while(rt_device_read(can_device, 0, data, len))
            {
                rt_device_read(can_device, 0, data, len);
                if(g_sys.conf.dbg == 1)
                    print_can_msg(data);
                resolve_can_msg(data);
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

