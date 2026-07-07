#ifndef __XSTAR_KERNEL_XFS_ARCHIVER_H__
#define __XSTAR_KERNEL_XFS_ARCHIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <xos/xos.h>

typedef void (*xfs_walk_callback_t)(const char * dir, const char * name, void * data);

enum {
	XFS_OPEN_MODE_READ		= 0,
	XFS_OPEN_MODE_WRITE		= 1,
	XFS_OPEN_MODE_APPEND	= 2,
};

struct xfs_archiver_t
{
	char * name;
	struct list_head_t list;

	void * (*mount)(const char * path, int * writable);
	void (*umount)(void * m);
	void (*walk)(void * m, const char * name, xfs_walk_callback_t cb, void * data);
	int (*isdir)(void * m, const char * name);
	int (*isfile)(void * m, const char * name);
	int (*mode)(void * m, const char * name);
	int (*mkdir)(void * m, const char * name);
	int (*remove)(void * m, const char * name);
	void * (*open)(void * m, const char * name, int mode);
	int64_t (*read)(void * f, void * buf, int64_t size);
	int64_t (*write)(void * f, void * buf, int64_t size);
	int64_t (*seek)(void * f, int64_t offset);
	int64_t (*tell)(void * f);
	int64_t (*length)(void * f);
	void (*flush)(void * f);
	void (*close)(void * f);
};

int register_archiver(struct xfs_archiver_t * archiver);
int unregister_archiver(struct xfs_archiver_t * archiver);
void * mount_archiver(const char * path, struct xfs_archiver_t ** archiver, int * writable);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_XFS_ARCHIVER_H__ */
