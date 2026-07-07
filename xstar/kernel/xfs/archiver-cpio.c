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

#include <driver/block/block.h>
#include <kernel/xfs/archiver.h>

struct cpio_newc_header_t {
	uint8_t c_magic[6];
	uint8_t c_ino[8];
	uint8_t c_mode[8];
	uint8_t c_uid[8];
	uint8_t c_gid[8];
	uint8_t c_nlink[8];
	uint8_t c_mtime[8];
	uint8_t c_filesize[8];
	uint8_t c_devmajor[8];
	uint8_t c_devminor[8];
	uint8_t c_rdevmajor[8];
	uint8_t c_rdevminor[8];
	uint8_t c_namesize[8];
	uint8_t c_check[8];
} __attribute__ ((packed));

struct mhandle_cpio_t {
	struct list_head_t list;
	struct hlist_head_t * hash;
	int hsize;
	struct block_t * blk;
};

struct fhandle_cpio_t
{
	struct list_head_t head;
	struct hlist_node_t node;
	char * name;
	int64_t start;
	int64_t size;
	int64_t offset;
	int isdir;
	int mode;
	struct block_t * blk;
};

static struct hlist_head_t * fhandle_hash(struct mhandle_cpio_t * m, const char * name)
{
	return &m->hash[shash(name) % m->hsize];
}

static struct fhandle_cpio_t * search_fhandle(struct mhandle_cpio_t * m, const char * name)
{
	struct fhandle_cpio_t * pos;
	struct hlist_node_t * n;

	if(!name)
		return NULL;

	hlist_for_each_entry_safe(pos, n, fhandle_hash(m, name), node)
	{
		if((xos_strcmp(pos->name, name) == 0))
			return pos;
	}
	return NULL;
}

static struct mhandle_cpio_t * alloc_mhandle(struct block_t * blk)
{
	struct cpio_newc_header_t header;
	struct mhandle_cpio_t * m;
	struct fhandle_cpio_t * f;
	int64_t offset, size, namesize;
	uint32_t mode;
	char path[1024];
	char buf[9] = { 0 };
	int hsize = 0;

	offset = 0;
	while(1)
	{
		if(block_read(blk, (uint8_t *)(&header), offset, sizeof(struct cpio_newc_header_t)) != sizeof(struct cpio_newc_header_t))
			break;

		if(xos_strncmp((const char *)header.c_magic, "070701", 6) != 0)
			break;

		xos_memcpy(buf, &header.c_filesize, 8);
		size = xos_strtoll((const char *)buf, NULL, 16);

		xos_memcpy(buf, &header.c_mode, 8);
		mode = strtoul((const char *)buf, NULL, 16);

		xos_memcpy(buf, &header.c_namesize, 8);
		namesize = xos_strtoll((const char *)buf, NULL, 16);

		if(block_read(blk, (uint8_t *)path, offset + sizeof(struct cpio_newc_header_t), namesize) != namesize)
			break;

		if((xos_strncmp(path, "TRAILER!!!", 10) == 0) && (namesize == 11))
			break;

		if(((mode & 00170000) == 0100000) || ((mode & 00170000) == 0040000))
			hsize++;

		offset += sizeof(struct cpio_newc_header_t);
		offset += (((namesize + 1) & ~3) + 2) + size;
		offset = (offset + 3) & ~0x3;
	}
	if(hsize == 0)
		return NULL;

	m = xos_mem_malloc(sizeof(struct mhandle_cpio_t));
	if(!m)
		return NULL;

	m->hsize = hsize * 2;
	m->blk = blk;
	m->hash = xos_mem_malloc(sizeof(struct hlist_head_t) * m->hsize);
	if(!m->hash)
	{
		xos_mem_free(m);
		return NULL;
	}
	init_list_head(&m->list);
	for(int i = 0; i < m->hsize; i++)
		init_hlist_head(&m->hash[i]);

	offset = 0;
	while(1)
	{
		if(block_read(blk, (uint8_t *)(&header), offset, sizeof(struct cpio_newc_header_t)) != sizeof(struct cpio_newc_header_t))
			break;

		if(xos_strncmp((const char *)header.c_magic, "070701", 6) != 0)
			break;

		xos_memcpy(buf, &header.c_filesize, 8);
		size = xos_strtoll((const char *)buf, NULL, 16);

		xos_memcpy(buf, &header.c_mode, 8);
		mode = strtoul((const char *)buf, NULL, 16);

		xos_memcpy(buf, &header.c_namesize, 8);
		namesize = xos_strtoll((const char *)buf, NULL, 16);

		if(block_read(blk, (uint8_t *)path, offset + sizeof(struct cpio_newc_header_t), namesize) != namesize)
			break;

		if((xos_strncmp(path, "TRAILER!!!", 10) == 0) && (namesize == 11))
			break;

		if(((mode & 00170000) == 0100000) || ((mode & 00170000) == 0040000))
		{
			f = xos_mem_malloc(sizeof(struct fhandle_cpio_t));
			if(f)
			{
				f->name = xos_strdup(path);
				f->start = offset + sizeof(struct cpio_newc_header_t) + (((namesize + 1) & ~3) + 2);
				f->size = size;
				f->offset = 0;
				f->isdir = ((mode & 00170000) == 0040000) ? TRUE : FALSE;
				f->mode = mode & 0000777;
				f->blk = blk;
				init_list_head(&f->head);
				list_add_tail(&f->head, &m->list);
				init_hlist_node(&f->node);
				hlist_add_head(&f->node, fhandle_hash(m, f->name));
			}
		}
		offset += sizeof(struct cpio_newc_header_t);
		offset += (((namesize + 1) & ~3) + 2) + size;
		offset = (offset + 3) & ~0x3;
	}

	return m;
}

static void free_mhandle(struct mhandle_cpio_t * m)
{
	struct fhandle_cpio_t * pos, * n;

	if(m)
	{
		list_for_each_entry_safe(pos, n, &m->list, head)
		{
			list_del(&pos->head);
			hlist_del(&pos->node);
			xos_mem_free(pos->name);
			xos_mem_free(pos);
		}
		xos_mem_free(m->hash);
		xos_mem_free(m);
	}
}

static void * cpio_mount(const char * path, int * writable)
{
	struct cpio_newc_header_t header;
	struct mhandle_cpio_t * m;
	struct block_t * blk;

	blk = search_block(path);
	if(!blk)
		return NULL;

	if(block_capacity(blk) <= sizeof(struct cpio_newc_header_t))
		return NULL;

	if(block_read(blk, (uint8_t *)(&header), 0, sizeof(struct cpio_newc_header_t)) != sizeof(struct cpio_newc_header_t))
		return NULL;

	if(xos_strncmp((const char *)header.c_magic, "070701", 6) != 0)
		return NULL;

	m = alloc_mhandle(blk);
	if(!m)
		return NULL;

	if(writable)
		*writable = 0;
	return m;
}

static void cpio_umount(void * m)
{
	struct mhandle_cpio_t * mh = (struct mhandle_cpio_t *)m;

	if(mh)
		free_mhandle(mh);
}

static void cpio_walk(void * m, const char * name, xfs_walk_callback_t cb, void * data)
{
	struct mhandle_cpio_t * mh = (struct mhandle_cpio_t *)m;
	struct fhandle_cpio_t * fh = search_fhandle(mh, name);
	struct fhandle_cpio_t * pos, * n;
	char * p;
	int l = xos_strlen(name);

	if((l == 0) && name)
	{
		list_for_each_entry_safe(pos, n, &mh->list, head)
		{
			if(xos_strncmp(name, pos->name, l) == 0)
			{
				p = &pos->name[l];
				if(p && !xos_strchr(p, '/'))
				{
					if(cb)
						cb(name, p, data);
				}
			}
		}
	}
	else if(fh && fh->isdir)
	{
		list_for_each_entry_safe(pos, n, &mh->list, head)
		{
			if(xos_strncmp(name, pos->name, l) == 0)
			{
				p = &pos->name[l];
				if(*p++ == '/')
				{
					if(p && !xos_strchr(p, '/'))
					{
						if(cb)
							cb(name, p, data);
					}
				}
			}
		}
	}
}

static int cpio_isdir(void * m, const char * name)
{
	if(name && (name[0] == '\0'))
		return TRUE;
	else
	{
		struct mhandle_cpio_t * mh = (struct mhandle_cpio_t *)m;
		struct fhandle_cpio_t * fh = search_fhandle(mh, name);
		return (fh && fh->isdir) ? TRUE : FALSE;
	}
}

static int cpio_isfile(void * m, const char * name)
{
	struct mhandle_cpio_t * mh = (struct mhandle_cpio_t *)m;
	struct fhandle_cpio_t * fh = search_fhandle(mh, name);
	return (fh && !fh->isdir) ? TRUE : FALSE;
}

static int cpio_mode(void * m, const char * name)
{
	struct mhandle_cpio_t * mh = (struct mhandle_cpio_t *)m;
	struct fhandle_cpio_t * fh = search_fhandle(mh, name);
	return (fh && fh->mode) ? fh->mode : 0;
}

static int cpio_mkdir(void * m, const char * name)
{
	return FALSE;
}

static int cpio_remove(void * m, const char * name)
{
	return FALSE;
}

static void * cpio_open(void * m, const char * name, int mode)
{
	struct mhandle_cpio_t * mh = (struct mhandle_cpio_t *)m;
	struct fhandle_cpio_t * fh;

	if(mode != XFS_OPEN_MODE_READ)
		return NULL;
	fh = search_fhandle(mh, name);
	if(!fh || fh->isdir)
		return NULL;
	fh->offset = 0;
	return ((void *)fh);
}

static int64_t cpio_read(void * f, void * buf, int64_t size)
{
	struct fhandle_cpio_t * fh = (struct fhandle_cpio_t *)f;
	int64_t len;
	if(size > fh->size - fh->offset)
		size = fh->size - fh->offset;
	len = block_read(fh->blk, buf, fh->start + fh->offset, size);
	fh->offset += len;
	return len;
}

static int64_t cpio_write(void * f, void * buf, int64_t size)
{
	return 0;
}

static int64_t cpio_seek(void * f, int64_t offset)
{
	struct fhandle_cpio_t * fh = (struct fhandle_cpio_t *)f;
	if(offset < 0)
		fh->offset = 0;
	else if(offset > fh->size)
		fh->offset = fh->size;
	else
		fh->offset = offset;
	return fh->offset;
}

static int64_t cpio_tell(void * f)
{
	struct fhandle_cpio_t * fh = (struct fhandle_cpio_t *)f;
	return fh->offset;
}

static int64_t cpio_length(void * f)
{
	struct fhandle_cpio_t * fh = (struct fhandle_cpio_t *)f;
	return fh->size;
}

static void cpio_flush(void * f)
{
}

static void cpio_close(void * f)
{
	struct fhandle_cpio_t * fh = (struct fhandle_cpio_t *)f;
	fh->offset = 0;
}

static struct xfs_archiver_t archiver_cpio = {
	.name		= "cpio",
	.mount		= cpio_mount,
	.umount		= cpio_umount,
	.walk		= cpio_walk,
	.isdir		= cpio_isdir,
	.isfile		= cpio_isfile,
	.mode		= cpio_mode,
	.mkdir		= cpio_mkdir,
	.remove		= cpio_remove,
	.open		= cpio_open,
	.read		= cpio_read,
	.write		= cpio_write,
	.seek		= cpio_seek,
	.tell		= cpio_tell,
	.length		= cpio_length,
	.flush		= cpio_flush,
	.close		= cpio_close,
};

static void archiver_cpio_init(void)
{
	register_archiver(&archiver_cpio);
}

static void archiver_cpio_exit(void)
{
	unregister_archiver(&archiver_cpio);
}

core_initcall(archiver_cpio_init);
core_exitcall(archiver_cpio_exit);
