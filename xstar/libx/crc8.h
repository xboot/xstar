#ifndef __XSTAR_LIBX_CRC8_H__
#define __XSTAR_LIBX_CRC8_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

uint8_t crc8_sum(uint8_t crc, const uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_CRC8_H__ */
