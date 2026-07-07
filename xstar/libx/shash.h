#ifndef __XSTAR_LIBX_SHASH_H__
#define __XSTAR_LIBX_SHASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

static inline __attribute__((always_inline)) uint32_t shash(const char * s)
{
	uint32_t v = 5381;
	if(s)
	{
		while(*s)
			v = (v << 5) + v + (*s++);
	}
	return v;
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_SHASH_H__ */
