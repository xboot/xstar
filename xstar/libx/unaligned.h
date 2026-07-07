#ifndef __XSTAR_LIBX_UNALIGNED_H__
#define __XSTAR_LIBX_UNALIGNED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstarcfg.h>

/*
 * Get unaligned
 */
static inline uint16_t __get_unaligned_le16(const uint8_t * p)
{
	return ((uint16_t)p[0] << 0) | ((uint16_t)p[1] << 8);
}

static inline uint16_t __get_unaligned_be16(const uint8_t * p)
{
	return ((uint16_t)p[0] << 8) | ((uint16_t)p[1] << 0);
}

static inline uint32_t __get_unaligned_le32(const uint8_t * p)
{
	return ((uint32_t)p[0] << 0) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline uint32_t __get_unaligned_be32(const uint8_t * p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | ((uint32_t)p[3] << 0);
}

static inline uint64_t __get_unaligned_le64(const uint8_t * p)
{
	return ((uint64_t)__get_unaligned_le32(p + 4) << 32) | (uint64_t)__get_unaligned_le32(p);
}

static inline uint64_t __get_unaligned_be64(const uint8_t * p)
{
	return ((uint64_t)__get_unaligned_be32(p) << 32) | (uint64_t)__get_unaligned_be32(p + 4);
}

static inline uint16_t get_unaligned_le16(const void * p)
{
	return __get_unaligned_le16((const uint8_t *)p);
}

static inline uint16_t get_unaligned_be16(const void * p)
{
	return __get_unaligned_be16((const uint8_t *)p);
}

static inline uint32_t get_unaligned_le32(const void * p)
{
	return __get_unaligned_le32((const uint8_t *)p);
}

static inline uint32_t get_unaligned_be32(const void * p)
{
	return __get_unaligned_be32((const uint8_t *)p);
}

static inline uint64_t get_unaligned_le64(const void * p)
{
	return __get_unaligned_le64((const uint8_t *)p);
}

static inline uint64_t get_unaligned_be64(const void * p)
{
	return __get_unaligned_be64((const uint8_t *)p);
}

/*
 * Put unaligned
 */
static inline void __put_unaligned_le16(uint8_t * p, uint16_t val)
{
	*p++ = val;
	*p++ = val >> 8;
}

static inline void __put_unaligned_be16(uint8_t * p, uint16_t val)
{
	*p++ = val >> 8;
	*p++ = val;
}

static inline void __put_unaligned_le32(uint8_t * p, uint32_t val)
{
	__put_unaligned_le16(p + 2, val >> 16);
	__put_unaligned_le16(p, val);
}

static inline void __put_unaligned_be32(uint8_t * p, uint32_t val)
{
	__put_unaligned_be16(p, val >> 16);
	__put_unaligned_be16(p + 2, val);
}

static inline void __put_unaligned_le64(uint8_t * p, uint64_t val)
{
	__put_unaligned_le32(p + 4, val >> 32);
	__put_unaligned_le32(p, val);
}

static inline void __put_unaligned_be64(uint8_t * p, uint64_t val)
{
	__put_unaligned_be32(p, val >> 32);
	__put_unaligned_be32(p + 4, val);
}

static inline void put_unaligned_le16(void * p, uint16_t val)
{
	__put_unaligned_le16(p, val);
}

static inline void put_unaligned_be16(void * p, uint16_t val)
{
	__put_unaligned_be16(p, val);
}

static inline void put_unaligned_le32(void * p, uint32_t val)
{
	__put_unaligned_le32(p, val);
}

static inline void put_unaligned_be32(void * p, uint32_t val)
{
	__put_unaligned_be32(p, val);
}

static inline void put_unaligned_le64(void * p, uint64_t val)
{
	__put_unaligned_le64(p, val);
}

static inline void put_unaligned_be64(void * p, uint64_t val)
{
	__put_unaligned_be64(p, val);
}

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_UNALIGNED_H__ */
