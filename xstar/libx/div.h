#ifndef __XSTAR_LIBX_DIV_H__
#define __XSTAR_LIBX_DIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

#define do_abs(x)	((x) < 0 ? -(x) : (x))

extern uint32_t do_udiv32(uint32_t dividend, uint32_t divisor, uint32_t * remainder);
extern uint64_t do_udiv64(uint64_t dividend, uint64_t divisor, uint64_t * remainder);

static inline uint32_t udiv32(uint32_t value, uint32_t divisor)
{
	uint32_t r;
	return do_udiv32(value, divisor, &r);
}

static inline uint32_t umod32(uint32_t value, uint32_t divisor)
{
	uint32_t r;

	do_udiv32(value, divisor, &r);
	return r;
}

static inline int32_t sdiv32(int32_t value, int32_t divisor)
{
	uint32_t r;

	if((value ^ divisor) < 0)
		return -do_udiv32(do_abs(value), do_abs(divisor), &r);
	else
		return do_udiv32(do_abs(value), do_abs(divisor), &r);
}

static inline int32_t smod32(int32_t value, int32_t divisor)
{
	uint32_t r;

	do_udiv32(do_abs(value), do_abs(divisor), &r);
	if(value < 0)
		return -r;
	else
		return r;
}

static inline uint64_t udiv64(uint64_t value, uint64_t divisor)
{
	uint64_t r;
	return do_udiv64(value, divisor, &r);
}

static inline uint64_t umod64(uint64_t value, uint64_t divisor)
{
	uint64_t r;

	do_udiv64(value, divisor, &r);
	return r;
}

static inline int64_t sdiv64(int64_t value, int64_t divisor)
{
	uint64_t r;

	if((value ^ divisor) < 0)
		return -do_udiv64(do_abs(value), do_abs(divisor), &r);
	else
		return do_udiv64(do_abs(value), do_abs(divisor), &r);
}

static inline int64_t smod64(int64_t value, int64_t divisor)
{
	uint64_t r;

	do_udiv64(do_abs(value), do_abs(divisor), &r);
	if(value < 0)
		return -r;
	else
		return r;
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_DIV_H__ */
