#ifndef __XSTAR_KERNEL_GRAPHIC_DIRTYLIST_H__
#define __XSTAR_KERNEL_GRAPHIC_DIRTYLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>
#include <kernel/graphic/region.h>

struct dirtylist_item_t {
	struct region_t region;
	int area;
};

struct dirtylist_t {
	struct dirtylist_item_t * items;
	unsigned int size;
	unsigned int count;
};

struct dirtylist_t * dirtylist_alloc(unsigned int size);
void dirtylist_free(struct dirtylist_t * l);
void dirtylist_clone(struct dirtylist_t * l, struct dirtylist_t * o);
void dirtylist_merge(struct dirtylist_t * l, struct dirtylist_t * o);
void dirtylist_clear(struct dirtylist_t * l);
void dirtylist_add(struct dirtylist_t * l, struct region_t * r);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_GRAPHIC_DIRTYLIST_H__ */
