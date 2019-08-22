
#ifndef __DRV_AMUX_H__
#define __DRV_AMUX_H__

#include <board.h>

void amux_drv_init(void);
void amux_auto_inc(void);
void amux_en(uint8_t enable);

#endif /* __DRV_AMUX_H__ */