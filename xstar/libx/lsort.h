#ifndef __XSTAR_LIBX_LSORT_H__
#define __XSTAR_LIBX_LSORT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/list.h>

void lsort(void * priv, struct list_head_t * head, int (*cmp)(void * priv, struct list_head_t * a, struct list_head_t * b));

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_LSORT_H__ */
