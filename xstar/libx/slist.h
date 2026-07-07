#ifndef __XSTAR_LIBX_SLIST_H__
#define __XSTAR_LIBX_SLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/list.h>

struct slist_t {
	struct list_head_t list;
	char * key;
	void * priv;
};

#define slist_for_each_entry(entry, sl) \
	list_for_each_entry(entry, &(sl)->list, list)

#define slist_for_each_entry_reverse(entry, sl) \
	list_for_each_entry_reverse(entry, &(sl)->list, list)

struct slist_t * slist_alloc(void);
void slist_free(struct slist_t * sl);
void slist_add(struct slist_t * sl, void * priv, const char * fmt, ...);
void slist_sort(struct slist_t * sl);
void slist_natsort(struct slist_t * sl);
void slist_sort_with(struct slist_t * sl, int (*cmp)(void *, struct list_head_t *, struct list_head_t *));
int slist_isempty(struct slist_t * sl);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_SLIST_H__ */
