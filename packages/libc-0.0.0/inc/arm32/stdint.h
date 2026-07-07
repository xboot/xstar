#ifndef __ARM32_STDINT_H__
#define __ARM32_STDINT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char			int8_t;
typedef unsigned char		uint8_t;

typedef signed short		int16_t;
typedef unsigned short		uint16_t;

typedef signed int			int32_t;
typedef unsigned int		uint32_t;

typedef signed long long	int64_t;
typedef unsigned long long	uint64_t;

typedef signed long long	intmax_t;
typedef unsigned long long	uintmax_t;

typedef signed int			ptrdiff_t;
typedef signed int			intptr_t;
typedef unsigned int 		uintptr_t;

typedef unsigned int		size_t;
typedef signed int			ssize_t;

typedef int8_t				int_least8_t;
typedef int16_t				int_least16_t;
typedef int32_t				int_least32_t;
typedef int64_t				int_least64_t;
typedef uint8_t				uint_least8_t;
typedef uint16_t			uint_least16_t;
typedef uint32_t			uint_least32_t;
typedef uint64_t			uint_least64_t;

typedef int8_t				int_fast8_t;
typedef int16_t				int_fast16_t;
typedef int32_t				int_fast32_t;
typedef int64_t				int_fast64_t;
typedef uint8_t				uint_fast8_t;
typedef uint16_t			uint_fast16_t;
typedef uint32_t			uint_fast32_t;
typedef uint64_t			uint_fast64_t;

#define INT8_MIN			(-1 - 0x7f)
#define INT16_MIN			(-1 - 0x7fff)
#define INT32_MIN			(-1 - 0x7fffffff)
#define INT64_MIN			(-1 - 0x7fffffffffffffff)

#define INT8_MAX			(0x7f)
#define INT16_MAX			(0x7fff)
#define INT32_MAX			(0x7fffffff)
#define INT64_MAX			(0x7fffffffffffffff)

#define UINT8_MAX			(0xff)
#define UINT16_MAX			(0xffff)
#define UINT32_MAX			(0xffffffffU)
#define UINT64_MAX			(0xffffffffffffffffU)

#define INTPTR_MIN			INT32_MIN
#define INTPTR_MAX			INT32_MAX
#define UINTPTR_MAX			UINT32_MAX
#define PTRDIFF_MIN			INT32_MIN
#define PTRDIFF_MAX			INT32_MAX
#define SIZE_MAX			UINT32_MAX

#ifdef __cplusplus
}
#endif

#endif /* __ARM32_STDINT_H__ */
