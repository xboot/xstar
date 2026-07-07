#ifndef __XSTAR_KERNEL_XFS_H__
#define __XSTAR_KERNEL_XFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <kernel/xfs/archiver.h>

struct xfs_path_t {
	char * mpath;
	void * mhandle;
	int writable;
	struct xfs_archiver_t * archiver;
	struct list_head_t list;
};

struct xfs_context_t {
	struct xfs_path_t mounts;
	struct mutex_t lock;
};

struct xfs_file_t {
	struct xfs_context_t * ctx;
	struct xfs_path_t * path;
	void * fhandle;
};

int xfs_mount(struct xfs_context_t * ctx, const char * path, int writable);
int xfs_umount(struct xfs_context_t * ctx, const char * path);
void xfs_walk(struct xfs_context_t * ctx, const char * name, xfs_walk_callback_t cb, void * data);
int xfs_isdir(struct xfs_context_t * ctx, const char * name);
int xfs_isfile(struct xfs_context_t * ctx, const char * name);
int xfs_mode(struct xfs_context_t * ctx, const char * name);
int xfs_mkdir(struct xfs_context_t * ctx, const char * name);
int xfs_remove(struct xfs_context_t * ctx, const char * name);
struct xfs_file_t * xfs_open_read(struct xfs_context_t * ctx, const char * name);
struct xfs_file_t * xfs_open_write(struct xfs_context_t * ctx, const char * name);
struct xfs_file_t * xfs_open_append(struct xfs_context_t * ctx, const char * name);
int64_t xfs_read(struct xfs_file_t * file, void * buf, int64_t size);
int64_t xfs_write(struct xfs_file_t * file, void * buf, int64_t size);
int64_t xfs_seek(struct xfs_file_t * file, int64_t offset);
int64_t xfs_tell(struct xfs_file_t * file);
int64_t xfs_length(struct xfs_file_t * file);
void xfs_flush(struct xfs_file_t * file);
void xfs_close(struct xfs_file_t * file);

struct xfs_context_t * xfs_alloc(void);
void xfs_free(struct xfs_context_t * ctx);

#ifdef __cplusplus
}
#endif

#endif /* __XSTAR_KERNEL_XFS_H__ */
