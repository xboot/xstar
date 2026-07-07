#ifndef __JCONFIG_H__
#define __JCONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

#define HAVE_PROTOTYPES 1
#define HAVE_UNSIGNED_CHAR 1
#define HAVE_UNSIGNED_SHORT 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
typedef unsigned char boolean;
#define HAVE_BOOLEAN
#define NO_GETENV 1
#ifdef JPEG_INTERNALS
#define INLINE __inline__
#endif

#undef malloc
#undef memalign
#undef realloc
#undef calloc
#undef free
#undef meminfo
#define malloc 		xos_mem_malloc
#define memalign	xos_mem_memalign
#define realloc		xos_mem_realloc
#define calloc		xos_mem_calloc
#define free		xos_mem_free
#define meminfo		xos_mem_meminfo

#undef memset
#undef memcpy
#undef memcmp
#define memset		xos_memset
#define memcpy		xos_memcpy
#define memcmp		xos_memcmp

#undef strlen
#undef strnlen
#undef strcpy
#undef strncpy
#undef sprintf
#define strlen		xos_strlen
#define strnlen		xos_strnlen
#define strcpy		xos_strcpy
#define strncpy		xos_strncpy
#define sprintf		xos_sprintf

#ifdef __cplusplus
}
#endif

#endif /* __JCONFIG_H__ */
