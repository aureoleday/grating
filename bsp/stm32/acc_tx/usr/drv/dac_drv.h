
#ifndef __DRV_DAC_H__
#define __DRV_DAC_H__

#include <board.h>

void dac_init(void);
int dac_start(void);
void dac_set(uint32_t dac_val);

#endif /* __DRV_DAC_H__ */
