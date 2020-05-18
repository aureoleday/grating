#ifndef __APP_CAN_H__
#define __APP_CAN_H__

#include <board.h>

rt_size_t can_sendmsg(const rt_uint8_t *data, const rt_uint32_t len);
rt_size_t can_sendmsg_addr(const rt_uint8_t *data, const rt_uint32_t len, uint16_t addr);
#endif /* __APP_CAN_H__ */
