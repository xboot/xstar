#ifndef __XSTAR_LIBX_PS_H__
#define __XSTAR_LIBX_PS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

struct psctx_t {
	struct hlist_head_t * hash;
	struct list_head_t list;
	unsigned int size;
	unsigned int n;
	struct mutex_t lock;
};

struct psctx_t * ps_alloc(int size);
void ps_free(struct psctx_t * ctx);

void ps_publish(struct psctx_t * ctx, const char * topic, void * pdat);
void ps_subscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot);
void ps_unsubscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_PS_H__ */
