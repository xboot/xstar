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

#include <kernel/xfs/archiver.h>

struct mhandle_sys_t {
	struct kobj_t * root;
};

struct fhandle_sys_t {
	struct kobj_t * kobj;
	int64_t offset;
};

static void * sys_mount(const char * path, int * writable)
{
	struct mhandle_sys_t * m;

	if(xos_strcmp(path, "sys") != 0)
		return NULL;
	m = xos_mem_malloc(sizeof(struct mhandle_sys_t));
	if(!m)
		return NULL;
	m->root = kobj_get_root();
	if(writable)
		*writable = 1;
	return m;
}

static void sys_umount(void * m)
{
	struct mhandle_sys_t * mh = (struct mhandle_sys_t *)m;

	if(mh)
		xos_mem_free(mh);
}

struct kobj_t * kobj_search_from_path(struct kobj_t * parent, const char * path)
{
	struct kobj_t * kobj = NULL;

	if(path && xos_strncmp(path, "sys", 3) == 0)
	{
		if(path[3] == '\0')
		{
			kobj = parent;
		}
		else if(path[3] == '/')
		{
			kobj = parent;
			char * t = xos_strdup(&path[4]);
			char * r, * p = t;
			if(t)
			{
				while((r = xos_strsep(&p, "/")) != NULL)
				{
					kobj = kobj_search(kobj, r);
					if(!kobj)
						break;
				}
				xos_mem_free(t);
			}
		}
	}
	return kobj;
}

static void sys_walk(void * m, const char * name, xfs_walk_callback_t cb, void * data)
{
	if(name && (name[0] == '\0'))
	{
		if(cb)
			cb(name, "sys", data);
	}
	else
	{
		struct mhandle_sys_t * mh = (struct mhandle_sys_t *)m;
		struct kobj_t * kobj = kobj_search_from_path(mh->root, name);
		if(kobj)
		{
			struct kobj_t * pos, * n;
			list_for_each_entry_safe(pos, n, &kobj->children, entry)
			{
				if(cb)
					cb(name, pos->name, data);
			}
		}
	}
}

static int sys_isdir(void * m, const char * name)
{
	if(name && (name[0] == '\0'))
		return TRUE;
	else
	{
		struct mhandle_sys_t * mh = (struct mhandle_sys_t *)m;
		struct kobj_t * kobj = kobj_search_from_path(mh->root, name);
		return (kobj && (kobj->type == KOBJ_TYPE_DIR)) ? TRUE : FALSE;
	}
}

static int sys_isfile(void * m, const char * name)
{
	struct mhandle_sys_t * mh = (struct mhandle_sys_t *)m;
	struct kobj_t * kobj = kobj_search_from_path(mh->root, name);

	return (kobj && (kobj->type == KOBJ_TYPE_REG)) ? TRUE : FALSE;
}

static int sys_mode(void * m, const char * name)
{
	struct mhandle_sys_t * mh = (struct mhandle_sys_t *)m;
	struct kobj_t * kobj = kobj_search_from_path(mh->root, name);
	int mode = 0;

	if(kobj)
	{
		if(kobj->type == KOBJ_TYPE_DIR)
		{
			mode = 0777;
		}
		else if(kobj->type == KOBJ_TYPE_REG)
		{
			if(kobj->read)
				mode |= 0444;
			if(kobj->write)
				mode |= 0222;
		}
	}
	return mode;
}

static int sys_mkdir(void * m, const char * name)
{
	return FALSE;
}

static int sys_remove(void * m, const char * name)
{
	return FALSE;
}

static void * sys_open(void * m, const char * name, int mode)
{
	struct mhandle_sys_t * mh = (struct mhandle_sys_t *)m;
	struct fhandle_sys_t * fh;
	struct kobj_t * kobj = kobj_search_from_path(mh->root, name);

	if(kobj && (kobj->type == KOBJ_TYPE_REG))
	{
		if(((mode == XFS_OPEN_MODE_READ) && kobj->read) || ((mode == XFS_OPEN_MODE_WRITE) && kobj->write))
		{
			fh = xos_mem_malloc(sizeof(struct fhandle_sys_t));
			if(fh)
			{
				fh->kobj = kobj;
				fh->offset = 0;
				return ((void *)fh);
			}
		}
	}
	return NULL;
}

static int64_t sys_read(void * f, void * buf, int64_t size)
{
	struct fhandle_sys_t * fh = (struct fhandle_sys_t *)f;
	int64_t len;

	if(fh->offset == 0)
	{
		if(fh->kobj && fh->kobj->read)
		{
			len = fh->kobj->read(fh->kobj, buf, size);
			fh->offset += len;
			return len;
		}
	}
	return 0;
}

static int64_t sys_write(void * f, void * buf, int64_t size)
{
	struct fhandle_sys_t * fh = (struct fhandle_sys_t *)f;
	int64_t len;

	if(fh->offset == 0)
	{
		if(fh->kobj && fh->kobj->write)
		{
			len = fh->kobj->write(fh->kobj, buf, size);
			fh->offset += len;
			return len;
		}
	}
	return 0;
}

static int64_t sys_seek(void * f, int64_t offset)
{
	struct fhandle_sys_t * fh = (struct fhandle_sys_t *)f;

	fh->offset = 0;
	return fh->offset;
}

static int64_t sys_tell(void * f)
{
	struct fhandle_sys_t * fh = (struct fhandle_sys_t *)f;

	return fh->offset;
}

static int64_t sys_length(void * f)
{
	return 0;
}

static void sys_flush(void * f)
{
}

static void sys_close(void * f)
{
	struct fhandle_sys_t * fh = (struct fhandle_sys_t *)f;

	xos_mem_free(fh);
}

static struct xfs_archiver_t archiver_sys = {
	.name		= "sys",
	.mount		= sys_mount,
	.umount 	= sys_umount,
	.walk		= sys_walk,
	.isdir		= sys_isdir,
	.isfile		= sys_isfile,
	.mode		= sys_mode,
	.mkdir		= sys_mkdir,
	.remove		= sys_remove,
	.open		= sys_open,
	.read		= sys_read,
	.write		= sys_write,
	.seek		= sys_seek,
	.tell		= sys_tell,
	.length		= sys_length,
	.flush		= sys_flush,
	.close		= sys_close,
};

static void archiver_sys_init(void)
{
	register_archiver(&archiver_sys);
}

static void archiver_sys_exit(void)
{
	unregister_archiver(&archiver_sys);
}

core_initcall(archiver_sys_init);
core_exitcall(archiver_sys_exit);
