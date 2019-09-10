#include <rtthread.h>
#include <board.h>
#include "amux_drv.h"

#define DIG_A       GET_PIN(B, 0)
#define DIG_B       GET_PIN(B, 1)
#define DIG_C       GET_PIN(B, 1)

#define ANA_A       GET_PIN(A, 1)
#define ANA_B       GET_PIN(A, 0)
#define ANA_C       GET_PIN(A, 2)

void read_drv_init(void)
{
    rt_pin_mode(DIG_A,PIN_MODE_INPUT);
    rt_pin_mode(DIG_B,PIN_MODE_INPUT);
    rt_pin_mode(DIG_C,PIN_MODE_INPUT);
}

uint8_t get_read_val(void)
{
    uint8_t res = 0;
    uint8_t i;
    for(i=0;i<2;i++)
    res |= rt_pin_read(DIG_C)<<1;
    res |= rt_pin_read(DIG_B)<<1;
    res |= rt_pin_read(DIG_A);
    return res;
}
