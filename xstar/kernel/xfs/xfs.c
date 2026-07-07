/*
 * Copyright(c) Jianjun Jiang <8192542@qq.com>
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <kernel/xfs/xfs.h>

static char * normal_path(const char * path)
{
	char * p, * q, * buf;
	char c;

	if(!path)
		return NULL;
	while(*path == '/')
		path++;
	p = q = buf = xos_mem_malloc(xos_strlen(path) + 1);

	do
	{
		c = *(path++);
		if((c == ':') || (c == '\\'))
		{
			xos_mem_free(buf);
			return NULL;
		}
		if(c == '/')
		{
			*q = '\0';
			if((xos_strcmp(p, ".") == 0) || (xos_strcmp(p, "..") == 0))
			{
				xos_mem_free(buf);
				return NULL;
			}
			while(*path == '/')
				path++;
			if(*path == '\0')
				break;
			p = q + 1;
		}
		*(q++) = c;
	} while(c != '\0');

	return buf;
}

int xfs_mount(struct xfs_context_t * ctx, const char * path, int writable)
{
	struct xfs_path_t * pos, * n;
	struct xfs_path_t * p;
	int w;

	if(!ctx || !path)
		return FALSE;

	list_for_each_entry_safe(pos, n, &ctx->mounts.list, list)
	{
		if(xos_strcmp(pos->mpath, path) == 0)
			return FALSE;
	}

	p = xos_mem_malloc(sizeof(struct xfs_path_t));
	if(!p)
		return FALSE;

	p->mhandle = mount_archiver(path, &p->archiver, &w);
	if(!p->mhandle)
	{
		xos_mem_free(p);
		return FALSE;
	}
	p->mpath = xos_strdup(path);
	p->writable = (writable && w) ? 1 : 0;

	xos_mutex_lock(&ctx->lock);
	init_list_head(&p->list);
	list_add_tail(&p->list, &ctx->mounts.list);
	xos_mutex_unlock(&ctx->lock);

	return TRUE;
}

int xfs_umount(struct xfs_context_t * ctx, const char * path)
{
	struct xfs_path_t * pos, * n;

	if(ctx && path)
	{
		list_for_each_entry_safe(pos, n, &ctx->mounts.list, list)
		{
			if(xos_strcmp(pos->mpath, path) == 0)
			{
				xos_mutex_lock(&ctx->lock);
				list_del(&pos->list);
				xos_mutex_unlock(&ctx->lock);

				pos->archiver->umount(pos->mhandle);
				xos_mem_free(pos->mpath);
				xos_mem_free(pos);
				return TRUE;
			}
		}
	}
	return FALSE;
}

void xfs_walk(struct xfs_context_t * ctx, const char * name, xfs_walk_callback_t cb, void * data)
{
	struct xfs_path_t * pos, * n;
	char * path;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			pos->archiver->walk(pos->mhandle, path, cb, data);
		}
		xos_mem_free(path);
	}
}

int xfs_isdir(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	char * path;
	int ret = FALSE;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			ret = pos->archiver->isdir(pos->mhandle, path);
			if(ret)
				break;
		}
		xos_mem_free(path);
	}
	return ret;
}

int xfs_isfile(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	char * path;
	int ret = FALSE;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			ret = pos->archiver->isfile(pos->mhandle, path);
			if(ret)
				break;
		}
		xos_mem_free(path);
	}
	return ret;
}

int xfs_mode(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	char * path;
	int mode = 0;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			mode = pos->archiver->mode(pos->mhandle, path);
			if(mode != 0)
				break;
		}
		xos_mem_free(path);
	}
	return mode;
}

int xfs_mkdir(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	char * path;
	int ret = FALSE;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			if(pos->writable)
			{
				ret = pos->archiver->mkdir(pos->mhandle, path);
				if(ret)
					break;
			}
		}
		xos_mem_free(path);
	}
	return ret;
}

int xfs_remove(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	char * path;
	int ret = FALSE;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			if(pos->writable)
			{
				ret = pos->archiver->remove(pos->mhandle, path);
				if(ret)
					break;
			}
		}
		xos_mem_free(path);
	}
	return ret;
}

struct xfs_file_t * xfs_open_read(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	struct xfs_file_t * file = NULL;
	char * path;
	void * f;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			f = pos->archiver->open(pos->mhandle, path, XFS_OPEN_MODE_READ);
			if(f)
			{
				file = xos_mem_malloc(sizeof(struct xfs_file_t));
				if(file)
				{
					file->ctx = ctx;
					file->path = pos;
					file->fhandle = f;
				}
				break;
			}
		}
		xos_mem_free(path);
	}
	return file;
}

struct xfs_file_t * xfs_open_write(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	struct xfs_file_t * file = NULL;
	char * path;
	void * f;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			if(pos->writable)
			{
				f = pos->archiver->open(pos->mhandle, path, XFS_OPEN_MODE_WRITE);
				if(f)
				{
					file = xos_mem_malloc(sizeof(struct xfs_file_t));
					if(file)
					{
						file->ctx = ctx;
						file->path = pos;
						file->fhandle = f;
					}
					break;
				}
			}
		}
		xos_mem_free(path);
	}
	return file;
}

struct xfs_file_t * xfs_open_append(struct xfs_context_t * ctx, const char * name)
{
	struct xfs_path_t * pos, * n;
	struct xfs_file_t * file = NULL;
	char * path;
	void * f;

	if(ctx && (path = normal_path(name)))
	{
		list_for_each_entry_safe_reverse(pos, n, &ctx->mounts.list, list)
		{
			if(pos->writable)
			{
				f = pos->archiver->open(pos->mhandle, path, XFS_OPEN_MODE_APPEND);
				if(f)
				{
					file = xos_mem_malloc(sizeof(struct xfs_file_t));
					if(file)
					{
						file->ctx = ctx;
						file->path = pos;
						file->fhandle = f;
					}
					break;
				}
			}
		}
		xos_mem_free(path);
	}
	return file;
}

int64_t xfs_read(struct xfs_file_t * file, void * buf, int64_t size)
{
	if(file)
		return file->path->archiver->read(file->fhandle, buf, size);
	return 0;
}

int64_t xfs_write(struct xfs_file_t * file, void * buf, int64_t size)
{
	if(file && file->path->writable)
		return file->path->archiver->write(file->fhandle, buf, size);
	return 0;
}

int64_t xfs_seek(struct xfs_file_t * file, int64_t offset)
{
	if(file)
		return file->path->archiver->seek(file->fhandle, offset);
	return 0;
}

int64_t xfs_tell(struct xfs_file_t * file)
{
	if(file)
		return file->path->archiver->tell(file->fhandle);
	return 0;
}

int64_t xfs_length(struct xfs_file_t * file)
{
	if(file)
		return file->path->archiver->length(file->fhandle);
	return 0;
}

void xfs_flush(struct xfs_file_t * file)
{
	if(file)
		file->path->archiver->flush(file->fhandle);
}

void xfs_close(struct xfs_file_t * file)
{
	if(file)
	{
		file->path->archiver->close(file->fhandle);
		xos_mem_free(file);
	}
}

struct xfs_context_t * xfs_alloc(void)
{
	struct xfs_context_t * ctx;
	char * path;

	ctx = xos_mem_malloc(sizeof(struct xfs_context_t));
	if(!ctx)
		return NULL;

	xos_memset(ctx, 0, sizeof(struct xfs_context_t));
	init_list_head(&ctx->mounts.list);
	xos_mutex_init(&ctx->lock);

	xfs_mount(ctx, "blk-romdisk.0", 0);
	xfs_mount(ctx, "sys", 1);

	path = xos_file_cwd();
	if(path && is_absolute_path(path))
		xfs_mount(ctx, path, 1);

	return ctx;
}

void xfs_free(struct xfs_context_t * ctx)
{
	struct xfs_path_t * pos, * n;

	if(!ctx)
		return;

	list_for_each_entry_safe(pos, n, &ctx->mounts.list, list)
	{
		xos_mutex_lock(&ctx->lock);
		list_del(&pos->list);
		xos_mutex_unlock(&ctx->lock);

		pos->archiver->umount(pos->mhandle);
		xos_mem_free(pos->mpath);
		xos_mem_free(pos);
	}
	xos_mutex_exit(&ctx->lock);
	xos_mem_free(ctx);
}
