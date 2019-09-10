#include <rtthread.h>
#include <board.h>

#define AMUX_A0    GET_PIN(C, 0)
#define AMUX_B0    GET_PIN(C, 1)
#define AMUX_C0    GET_PIN(C, 2)
#define AMUX_A1    GET_PIN(C, 3)
#define AMUX_B1    GET_PIN(C, 4)
#define AMUX_C1    GET_PIN(C, 5)
#define AMUX_A2    GET_PIN(C, 6)
#define AMUX_B2    GET_PIN(C, 7)
#define AMUX_C2    GET_PIN(C, 8)
#define AMUX_A3    GET_PIN(C, 9)
#define AMUX_B3    GET_PIN(C, 10)
#define AMUX_C3    GET_PIN(C, 11)
#define AMUX_EN    GET_PIN(C, 12)

#define DIG_A       GET_PIN(B, 0)
#define DIG_B       GET_PIN(B, 1)
#define DIG_C       GET_PIN(B, 1)

#define ANA_A       GET_PIN(A, 1)
#define ANA_B       GET_PIN(A, 0)
#define ANA_C       GET_PIN(A, 2)

static rt_base_t    amux_pid[4][3];
static uint8_t      amux_sts[3];

static void rd_pin_init(void)
{
    rt_pin_mode(DIG_A,PIN_MODE_INPUT);
    rt_pin_mode(DIG_B,PIN_MODE_INPUT);
    rt_pin_mode(DIG_C,PIN_MODE_INPUT);
}

static void amux_pin_map_mat(void)
{
    amux_pid[0][0] = AMUX_A0;
    amux_pid[0][1] = AMUX_B0;
    amux_pid[0][2] = AMUX_C0;
    amux_pid[1][0] = AMUX_A1;
    amux_pid[1][1] = AMUX_B1;
    amux_pid[1][2] = AMUX_C1;
    amux_pid[2][0] = AMUX_A2;
    amux_pid[2][1] = AMUX_B2;
    amux_pid[2][2] = AMUX_C2;
    amux_pid[3][0] = AMUX_A3;
    amux_pid[3][1] = AMUX_B3;
    amux_pid[3][2] = AMUX_C3;
}

static void amux_decode(uint8_t pos, uint8_t addr)
{
    uint8_t i;
    for(i=0;i<3;i++)
    {
        rt_pin_write(amux_pid[i][pos], (addr>>i)&0x01);
    }    
}

void amux_reset(void)
{
    uint8_t i;
    
    for(i=0;i<2;i++)
    {
        amux_sts[0] = 0;
        amux_decode(i,amux_sts[i]);
    }
}

void amux_drv_init(void)
{
    uint8_t i,j;
    
    amux_pin_map_mat();
    rd_pin_init();
    
    rt_pin_mode(AMUX_EN,PIN_MODE_OUTPUT);
    for(i=0;i<4;i++)
    {
        for(j=0;j<3;j++)
        {
            rt_pin_mode(amux_pid[i][j],PIN_MODE_OUTPUT);
        }
    }
    
    rt_pin_write(AMUX_EN,PIN_LOW);
    for(i=0;i<4;i++)
    {
        for(j=0;j<3;j++)
        {
            rt_pin_write(amux_pid[i][j],PIN_LOW);
        }
    }    
    amux_reset();
}

void amux_set_chan(uint8_t chan_id)
{
    uint8_t i;
    for(i=0;i<2;i++)
        amux_decode(i,chan_id);
}

void amux_auto_inc(void)
{
    uint8_t i;
    if(amux_sts[0] == amux_sts[2])
    {
        amux_sts[0]++;
        amux_decode(0,amux_sts[0]&0x0f);
    }
    else
    {
        if(amux_sts[0] == amux_sts[1])
        {
            amux_sts[2]++;            
            if(amux_sts[0]>15)
            {
                for(i=0;i<3;i++)
                    amux_sts[i] = 0;
            }
            amux_decode(2,amux_sts[2]&0x0f);
        }
        else
        {
            amux_sts[1]++;
            amux_decode(1,amux_sts[2]&0x0f);
        }
    }
}

void amux_en(uint8_t enable)
{
    if(enable)
        rt_pin_write(AMUX_EN,PIN_HIGH);
    else
        rt_pin_write(AMUX_EN,PIN_LOW);
}

uint8_t get_dig_val(void)
{
    uint8_t res = 0;
    res |= rt_pin_read(DIG_C)<<1;
    res |= rt_pin_read(DIG_B)<<1;
    res |= rt_pin_read(DIG_A);
    return res;
}

MSH_CMD_EXPORT(amux_auto_inc, amux-auto_inc);
MSH_CMD_EXPORT(amux_reset, amux reset);
MSH_CMD_EXPORT(get_dig_val, get_readings);
