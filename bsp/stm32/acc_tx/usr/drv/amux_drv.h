
#ifndef __DRV_AMUX_H__
#define __DRV_AMUX_H__

#include <board.h>

void amux_drv_init(void);
void amux_auto_inc(void);
void amux_set_chan(uint8_t chan_id);
void amux_en(uint8_t enable);
uint8_t get_dig_val(void);

#endif /* __DRV_AMUX_H__ */

