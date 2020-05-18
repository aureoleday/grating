#ifndef __DRV_SR_H__
#define __DRV_SR_H__

#include <board.h>
int sr_init(void);
int16_t sr_hwt_start(int16_t sec,int16_t u_sec);
int16_t sr_hwt_stop(void);
#endif /* __DRV_SR_H__ */
