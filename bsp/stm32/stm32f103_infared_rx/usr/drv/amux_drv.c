#include <stdlib.h>
#include <rtthread.h>
#include <board.h>
#include "sys_conf.h"
#include "display.h"

///* defined the LED0 pin: PB1 */
//#define LED0_PIN    GET_PIN(B, 15)

#define TEST_PIN    GET_PIN(D, 2)

//#define RX_SYNC     GET_PIN(A, 7)
#define RX_SYNC     GET_PIN(C, 9)
#define RX_RSTN     GET_PIN(A, 8)
#define RX_DATA     GET_PIN(A, 15)

#define AMUX_A0     GET_PIN(C, 0)
#define AMUX_A1     GET_PIN(C, 1)
#define AMUX_A2     GET_PIN(C, 2)
#define AMUX_A3     GET_PIN(C, 3)
#define AMUX_A4     GET_PIN(C, 4)
#define AMUX_A5     GET_PIN(C, 5)
        
#define AMUX_EN     GET_PIN(C, 12)

#define DIG_A       GET_PIN(B, 0)
#define DIG_B       GET_PIN(B, 1)
#define DIG_C       GET_PIN(B, 2)
        
#define ANA_A       GET_PIN(A, 1)
#define ANA_B       GET_PIN(A, 0)
#define ANA_C       GET_PIN(A, 2)

static rt_base_t    amux_pid[6];
static uint8_t      amux_sts;

static void rx_sync_irq_cb(void *args);
static void rx_rstn_irq_cb(void *args);
static void amux_set_chan(uint8_t chan_id);
static uint8_t get_dig_val(void);

static void rx_pin_init(void)
{
    rt_pin_mode(RX_SYNC,PIN_MODE_INPUT);
    rt_pin_mode(RX_RSTN,PIN_MODE_INPUT);
    rt_pin_mode(RX_DATA,PIN_MODE_INPUT);
    rt_pin_attach_irq(RX_SYNC, PIN_IRQ_MODE_FALLING, rx_sync_irq_cb, RT_NULL);
    rt_pin_attach_irq(RX_RSTN, PIN_IRQ_MODE_FALLING, rx_rstn_irq_cb, RT_NULL);
    rt_pin_irq_enable(RX_SYNC, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(RX_RSTN, PIN_IRQ_ENABLE);    
}

static void rd_pin_init(void)
{    
    rt_pin_mode(DIG_A,PIN_MODE_INPUT);
    rt_pin_mode(DIG_B,PIN_MODE_INPUT);
    rt_pin_mode(DIG_C,PIN_MODE_INPUT);
    rt_pin_mode(TEST_PIN,PIN_MODE_OUTPUT);
    rt_pin_write(TEST_PIN,PIN_LOW);
}

static void amux_pin_map_mat(void)
{
    amux_pid[0] = AMUX_A0;
    amux_pid[1] = AMUX_A1;
    amux_pid[2] = AMUX_A2;
    amux_pid[3] = AMUX_A3;
    amux_pid[4] = AMUX_A4;
    amux_pid[5] = AMUX_A5;
}

static void amux_set_chan(uint8_t chan_id)
{
    uint8_t i;
    for(i=0;i<6;i++)
    {
        rt_pin_write(amux_pid[i], (chan_id>>i)&0x01);
    }    
}

void amux_en(uint8_t enable)
{
    if(enable)
        rt_pin_write(AMUX_EN,PIN_HIGH);
    else
        rt_pin_write(AMUX_EN,PIN_LOW);
}

void amux_drv_init(void)
{
    uint8_t i;
    amux_pin_map_mat();
    rd_pin_init();
    rx_pin_init();
    rt_pin_mode(AMUX_EN,PIN_MODE_OUTPUT);
    rt_pin_write(AMUX_EN,PIN_HIGH);
    
    for(i=0;i<6;i++)
    {
        rt_pin_mode(amux_pid[i],PIN_MODE_OUTPUT);
        rt_pin_write(amux_pid[i],PIN_LOW);
    }   
    amux_sts = 0;
    amux_set_chan(amux_sts);
}

static uint8_t get_dig_val(void)
{
    uint8_t res = 0;
    res = rt_pin_read(DIG_C);
    res = (res<<1)|rt_pin_read(DIG_B);
    res = (res<<1)|rt_pin_read(DIG_A);
    return res;
}

static void rx_sync_irq_cb(void *args)
{
    extern sys_reg_st  g_sys;
    rt_pin_write(TEST_PIN,PIN_HIGH);
    pixel_in(amux_sts, get_dig_val());
    rt_pin_write(TEST_PIN,PIN_LOW);
    amux_sts++;
    amux_set_chan(amux_sts);
//    amux_set_chan(chan_reg);
//    if(chan_reg == 0)
//        chan_reg = 48;
//    else
//        chan_reg = 0;
}

static void rx_rstn_irq_cb(void *args)
{ 
    amux_sts = 0;
    amux_set_chan(amux_sts);
}

static void amux_dec(int argc, char**argv)
{
    if (argc < 2)
    {
        rt_kprintf("Please input'amux_dec <chan_id>'\n");
        return;
    }
    amux_set_chan(atoi(argv[1]));
}

MSH_CMD_EXPORT(get_dig_val, get_readings);
MSH_CMD_EXPORT(amux_dec, set amux chan-id);
