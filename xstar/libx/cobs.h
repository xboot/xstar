#ifndef __XSTAR_LIBX_COBS_H__
#define __XSTAR_LIBX_COBS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

static inline int cobs_encode_bound(unsigned int len)
{
	return (int)(len + len / 254 + 1);
}

static inline int cobs_decode_bound(unsigned int len)
{
	return (len > 0) ? (int)(len - 1) : 0;
}

int cobs_encode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len);
int cobs_decode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif
