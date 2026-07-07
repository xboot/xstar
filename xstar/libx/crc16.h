#ifndef __XSTAR_LIBX_CRC16_H__
#define __XSTAR_LIBX_CRC16_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

uint16_t crc16_sum(uint16_t crc, const uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_CRC16_H__ */
