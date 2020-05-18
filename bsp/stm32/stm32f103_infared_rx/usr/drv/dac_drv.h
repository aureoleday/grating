
#ifndef __DRV_DAC_H__
#define __DRV_DAC_H__

#include <board.h>

void dac_init(void);
int dac_start(void);
void dac_set(uint32_t dac_val);
int16_t sr_hwt_start(int16_t sec,int16_t u_sec);
int16_t sr_hwt_stop(void);

#endif /* __DRV_DAC_H__ */
