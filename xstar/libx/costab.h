#ifndef __XSTAR_LIBX_COSTAB_H__
#define __XSTAR_LIBX_COSTAB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

/*
 * Integer trigonometric functions using 16-bit fixed-point angles (2*PI/65536 units)
 */
extern const int16_t icostab[1024];

static inline int isin(int x)
{
	return (int)icostab[((x + 0xc000) >> 6) & 0x3ff];
}

static inline int icos(int x)
{
	return (int)icostab[(x >> 6) & 0x3ff];
}

/*
 * Floating-point trigonometric functions using 16-bit fixed-point angles (2*PI/65536 units)
 */
extern const float fcostab[1024];

static inline float fsin(int x)
{
	return (float)fcostab[((x + 0xc000) >> 6) & 0x3ff];
}

static inline float fcos(int x)
{
	return (float)fcostab[(x >> 6) & 0x3ff];
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_COSTAB_H__ */
