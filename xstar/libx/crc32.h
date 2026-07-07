#ifndef __XSTAR_LIBX_CRC32_H__
#define __XSTAR_LIBX_CRC32_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

uint32_t crc32_sum(uint32_t crc, const uint8_t * buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_CRC32_H__ */
