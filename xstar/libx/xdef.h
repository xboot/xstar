#ifndef __XSTAR_LIBX_XDEF_H__
#define __XSTAR_LIBX_XDEF_H__

#include <xstarcfg.h>

#define SZ_16							(0x00000010)
#define SZ_256							(0x00000100)
#define SZ_512							(0x00000200)
#define SZ_1K							(0x00000400)
#define SZ_4K							(0x00001000)
#define SZ_8K							(0x00002000)
#define SZ_16K							(0x00004000)
#define SZ_32K							(0x00008000)
#define SZ_64K							(0x00010000)
#define SZ_128K							(0x00020000)
#define SZ_256K							(0x00040000)
#define SZ_512K							(0x00080000)
#define SZ_1M							(0x00100000)
#define SZ_2M							(0x00200000)
#define SZ_4M							(0x00400000)
#define SZ_8M							(0x00800000)
#define SZ_16M							(0x01000000)
#define SZ_32M							(0x02000000)
#define SZ_64M							(0x04000000)
#define SZ_128M							(0x08000000)
#define SZ_256M							(0x10000000)
#define SZ_512M							(0x20000000)
#define SZ_1G							(0x40000000)
#define SZ_2G							(0x80000000)

#if !(defined(NULL))
#if defined(__cplusplus)
#define NULL							(0)
#else
#define NULL							((void *)0)
#endif
#endif

#if !(defined(FALSE))
#define FALSE							(0)
#endif

#if !(defined(TRUE))
#define TRUE							(1)
#endif

#if !(defined(offsetof))
#if (defined(__GNUC__) && (__GNUC__ >= 4))
#define offsetof(type, member)			__builtin_offsetof(type, member)
#else
#define offsetof(type, field)			((size_t)(&((type *)0)->field))
#endif
#endif

#if !(defined(container_of))
#define container_of(ptr, type, member)	({const typeof(((type *)0)->member) *__mptr = (ptr); (type *)((char *)__mptr - offsetof(type,member));})
#endif

#if !(defined(likely))
#if (defined(__GNUC__) && (__GNUC__ >= 3))
#define likely(expr)					(__builtin_expect(!!(expr), 1))
#else
#define likely(expr)					(!!(expr))
#endif
#endif

#if !(defined(unlikely))
#if (defined(__GNUC__) && (__GNUC__ >= 3))
#define unlikely(expr)					(__builtin_expect(!!(expr), 0))
#else
#define unlikely(expr)					(!!(expr))
#endif
#endif

#if !(defined(STATIC_ASSERT))
#define STATIC_ASSERT(cond)				typedef char __static_assert[(cond) ? 1 : -1]
#endif

#define XMAP(x, ia, ib, oa, ob)			((((x) - (ia)) * ((ob) - (oa)) / ((ib) - (ia)) + (oa)))
#define XMIN(a, b)						({typeof(a) _amin = (a); typeof(b) _bmin = (b); (void)(&_amin == &_bmin); _amin < _bmin ? _amin : _bmin;})
#define XMAX(a, b)						({typeof(a) _amax = (a); typeof(b) _bmax = (b); (void)(&_amax == &_bmax); _amax > _bmax ? _amax : _bmax;})
#define XCLAMP(v, a, b)					XMIN(XMAX(a, v), b)

#define XFLOOR(x)						((x) > 0 ? (int)(x) : (int)((x) - 0.9999999999))
#define XROUND(x)						((x) > 0 ? (int)((x) + 0.5) : (int)((x) - 0.5))
#define XCEIL(x)						((x) > 0 ? (int)((x) + 0.9999999999) : (int)(x))
#define XDIV255(x)						((((int)(x) + 1) * 257) >> 16)
#define XBYTEMUL(x, a)					((((((x) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + (((((x) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff))

#define ARRAY_SIZE(array)				(sizeof(array) / sizeof((array)[0]))
#define X(...)							("" #__VA_ARGS__ "")

static inline __attribute__((always_inline)) int xffs(int x)
{
	return __builtin_ffs(x);
}

static inline __attribute__((always_inline)) int xfls(int x)
{
	return x ? sizeof(x) * 8 - __builtin_clz(x) : 0;
}

static inline __attribute__((always_inline)) unsigned long __xffs(unsigned long x)
{
	return __builtin_ctzl(x);
}

static inline __attribute__((always_inline)) unsigned long __xfls(unsigned long x)
{
	return (sizeof(x) * 8) - 1 - __builtin_clzl(x);
}

#endif /* __XSTAR_LIBX_XDEF_H__ */
