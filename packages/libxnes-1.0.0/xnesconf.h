#ifndef __XNES_CONF_H__
#define __XNES_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xstar.h>

#define xnes_malloc 	xos_mem_malloc
#define xnes_free		xos_mem_free
#define xnes_memcpy 	xos_memcpy
#define xnes_memset		xos_memset

#ifndef xnes_malloc
#define xnes_malloc		malloc
#endif

#ifndef xnes_free
#define xnes_free		free
#endif

#ifndef xnes_memcpy
#define xnes_memcpy		memcpy
#endif

#ifndef xnes_memset
#define xnes_memset		memset
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XNES_CONF_H__ */
