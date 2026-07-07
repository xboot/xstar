#ifndef __XSTAR_LIBX_RC4_H__
#define __XSTAR_LIBX_RC4_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

void rc4_crypt(uint8_t * key, int kl, uint8_t * dat, int dl);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_RC4_H__ */
