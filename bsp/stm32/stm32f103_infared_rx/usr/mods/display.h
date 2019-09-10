
#ifndef __DRV_DISPLAY_H__
#define __DRV_DISPLAY_H__

#include <board.h>

void kf_init(void);
uint16_t push_raw(uint16_t data);
uint16_t pop_raw(uint16_t *data_ptr, uint16_t cnt);
uint16_t pixel_in(uint16_t chan, uint16_t pixel);
#endif /* __DRV_DISPLAY_H__ */
