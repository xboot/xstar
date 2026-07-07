#ifndef __XSTAR_LIBX_KOBJ_H__
#define __XSTAR_LIBX_KOBJ_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libx/list.h>

enum kobj_type_t {
	KOBJ_TYPE_DIR,
	KOBJ_TYPE_REG,
};

struct kobj_t {
	/* kobj name */
	char * name;

	/* kobj type DIR or REG */
	enum kobj_type_t type;

	/* kobj's parent */
	struct kobj_t * parent;

	/* kobj's entry */
	struct list_head_t entry;

	/* kobj's children */
	struct list_head_t children;

	/* kobj read */
	ssize_t (*read)(struct kobj_t * kobj, void * buf, size_t size);

	/* kobj write */
	ssize_t (*write)(struct kobj_t * kobj, void * buf, size_t size);

	/* private data */
	void * priv;
};

typedef ssize_t (*kobj_read_t)(struct kobj_t * kobj, void * buf, size_t size);
typedef ssize_t (*kobj_write_t)(struct kobj_t * kobj, void * buf, size_t size);

struct kobj_t * kobj_get_root(void);
struct kobj_t * kobj_search(struct kobj_t * parent, const char * name);
struct kobj_t * kobj_search_directory_with_create(struct kobj_t * parent, const char * name);
struct kobj_t * kobj_alloc_directory(const char * name);
struct kobj_t * kobj_alloc_regular(const char * name, kobj_read_t read, kobj_write_t write, void * priv);
int kobj_free(struct kobj_t * kobj);
int kobj_add(struct kobj_t * parent, struct kobj_t * kobj);
int kobj_remove(struct kobj_t * parent, struct kobj_t * kobj);
int kobj_add_directory(struct kobj_t * parent, const char * name);
int kobj_add_regular(struct kobj_t * parent, const char * name, kobj_read_t read, kobj_write_t write, void * priv);
int kobj_remove_self(struct kobj_t * kobj);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_LIBX_KOBJ_H__ */
