#ifndef __XSTAR_LIBX_BYTEORDER_H__
#define __XSTAR_LIBX_BYTEORDER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

static inline uint16_t __swab16(uint16_t x)
{
	return ((x << 8) | ( x>> 8));
}

static inline uint32_t __swab32(uint32_t x)
{
	return ((x << 24) | (x >> 24) | \
		((x & (uint32_t)0x0000ff00UL)<<8) | \
		((x & (uint32_t)0x00ff0000UL)>>8));
}

static inline uint64_t __swab64(uint64_t x)
{
	return ((x << 56) | (x >> 56) | \
		((x & (uint64_t)0x000000000000ff00ULL)<<40) | \
		((x & (uint64_t)0x0000000000ff0000ULL)<<24) | \
		((x & (uint64_t)0x00000000ff000000ULL)<< 8) | \
		((x & (uint64_t)0x000000ff00000000ULL)>> 8) | \
		((x & (uint64_t)0x0000ff0000000000ULL)>>24) | \
		((x & (uint64_t)0x00ff000000000000ULL)>>40));
}

static inline uint32_t __swahw32(uint32_t x)
{
	return (((x & (uint32_t)0x0000ffffUL) << 16) | ((x & (uint32_t)0xffff0000UL) >> 16));
}

static inline uint32_t __swahb32(uint32_t x)
{
	return (((x & (uint32_t)0x00ff00ffUL) << 8) | ((x & (uint32_t)0xff00ff00UL) >> 8));
}

static inline int cpu_is_big_endian(void)
{
	const uint16_t endian = 256;
	return (*(const uint8_t *)&endian) ? 1 : 0;
}

static inline uint64_t cpu_to_le64(uint64_t x)
{
	if(cpu_is_big_endian())
		return __swab64(x);
	return x;
}

static inline uint64_t le64_to_cpu(uint64_t x)
{
	if(cpu_is_big_endian())
		return __swab64(x);
	return x;
}

static inline uint32_t cpu_to_le32(uint32_t x)
{
	if(cpu_is_big_endian())
		return __swab32(x);
	return x;
}

static inline uint32_t le32_to_cpu(uint32_t x)
{
	if(cpu_is_big_endian())
		return __swab32(x);
	return x;
}

static inline uint16_t cpu_to_le16(uint16_t x)
{
	if(cpu_is_big_endian())
		return __swab16(x);
	return x;
}

static inline uint16_t le16_to_cpu(uint16_t x)
{
	if(cpu_is_big_endian())
		return __swab16(x);
	return x;
}

static inline uint64_t cpu_to_be64(uint64_t x)
{
	if(cpu_is_big_endian())
		return x;
	return __swab64(x);
}

static inline uint64_t be64_to_cpu(uint64_t x)
{
	if(cpu_is_big_endian())
		return x;
	return __swab64(x);
}

static inline uint32_t cpu_to_be32(uint32_t x)
{
	if(cpu_is_big_endian())
		return x;
	return __swab32(x);
}

static inline uint32_t be32_to_cpu(uint32_t x)
{
	if(cpu_is_big_endian())
		return x;
	return __swab32(x);
}

static inline uint16_t cpu_to_be16(uint16_t x)
{
	if(cpu_is_big_endian())
		return x;
	return __swab16(x);
}

static inline uint16_t be16_to_cpu(uint16_t x)
{
	if(cpu_is_big_endian())
		return x;
	return __swab16(x);
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_BYTEORDER_H__ */
