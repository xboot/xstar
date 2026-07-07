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
#include <kernel/time/timer.h>
#include <kernel/xfs/xfs.h>
#include <kernel/core/setting.h>

struct setting_t {
	struct timer_t timer;
	struct hmap_t * map;
	struct mutex_t lock;
	int dirty;

	uint64_t (*capacity)(struct setting_t * setting);
	uint64_t (*read)(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count);
	uint64_t (*write)(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count);
	void (*sync)(struct setting_t * setting);
	void * priv;
};
static struct setting_t __setting = { 0 };

static int start_with(const char * str, const char * prefix)
{
	if(str && prefix)
	{
		while(*prefix)
		{
			if((*str == '\0') || (*str != *prefix))
				return 0;
			str++;
			prefix++;
		}
		return 1;
	}
	return 0;
}

void setting_set(const char * key, const char * value)
{
	struct setting_t * setting = &__setting;

	if(setting && key)
	{
		xos_mutex_lock(&setting->lock);
		char * v = hmap_search(setting->map, key);
		if(v)
		{
			hmap_remove(setting->map, key);
			xos_mem_free(v);
			if(!start_with(key, "volatile."))
				setting->dirty = 1;
		}
		if(value)
		{
			hmap_add(setting->map, key, xos_strdup(value));
			if(!start_with(key, "volatile."))
				setting->dirty = 1;
		}
		if(setting->dirty)
			timer_start(&setting->timer, ms_to_ktime(10000));
		xos_mutex_unlock(&setting->lock);
	}
}

const char * setting_get(const char * key, const char * def)
{
	struct setting_t * setting = &__setting;
	const char * v = NULL;

	if(setting && key)
	{
		xos_mutex_lock(&setting->lock);
		v = hmap_search(setting->map, key);
		xos_mutex_unlock(&setting->lock);
	}
	return v ? v : def;
}

void setting_clear(void)
{
	struct setting_t * setting = &__setting;

	if(setting)
	{
		xos_mutex_lock(&setting->lock);
		hmap_clear(setting->map);
		setting->dirty = 1;
		timer_start(&setting->timer, ms_to_ktime(10000));
		xos_mutex_unlock(&setting->lock);
	}
}

void setting_sync(void)
{
	struct setting_t * setting = &__setting;

	if(setting->dirty)
	{
		timer_start(&setting->timer, ms_to_ktime(1));
		while(setting->dirty)
		{
			xos_thread_sleep(1000000ULL);
		}
	}
}

void setting_foreach(void (*cb)(const char * key, const char * value))
{
	struct setting_t * setting = &__setting;
	struct hmap_entry_t * e;

	hmap_sort(setting->map);
	hmap_for_each_entry(e, setting->map)
	{
		if(cb)
			cb(e->key, e->value);
	}
}

static uint64_t __setting_capacity(struct setting_t * setting)
{
	if(setting && setting->capacity)
		return setting->capacity(setting);
	return 0;
}

static uint64_t __setting_read(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count)
{
	if(setting && setting->capacity && setting->read)
	{
		uint64_t capacity = setting->capacity(setting);
		offset = XCLAMP(offset, (uint64_t)0, capacity);
		count = XCLAMP(count, (uint64_t)0, capacity - offset);
		return setting->read(setting, buf, offset, count);
	}
	return 0;
}

static uint64_t __setting_write(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count)
{
	if(setting && setting->capacity && setting->write)
	{
		uint64_t capacity = setting->capacity(setting);
		offset = XCLAMP(offset, (uint64_t)0, capacity);
		count = XCLAMP(count, (uint64_t)0, capacity - offset);
		return setting->write(setting, buf, offset, count);
	}
	return 0;
}

static void __setting_sync(struct setting_t * setting)
{
	if(setting && setting->sync)
		setting->sync(setting);
}

static int setting_timer_function(struct timer_t * timer, void * data)
{
	struct setting_t * setting = (struct setting_t *)data;

	if(setting->dirty)
	{
		xos_mutex_lock(&setting->lock);
		hmap_sort(setting->map);

		uint64_t size = __setting_capacity(setting);
		if(size > 0)
		{
			unsigned char * s = xos_mem_malloc(size + 1);
			if(s)
			{
				struct hmap_entry_t * e;
				uint64_t l = 12;
				hmap_for_each_entry(e, setting->map)
				{
					if(start_with(e->key, "volatile."))
						break;
					if(l + xos_strlen(e->key) + xos_strlen(e->value) + 3 > size)
						break;
					l += xos_sprintf((char *)(s + l), "%s:%s|", e->key, (char *)e->value);
				}
				s[l++] = '\0';
				s[8] = ((l - 12) >>  0) & 0xff;
				s[9] = ((l - 12) >>  8) & 0xff;
				s[10] = ((l - 12) >> 16) & 0xff;
				s[11] = ((l - 12) >> 24) & 0xff;
				uint32_t c = crc32_sum(0, (const uint8_t *)(&s[8]), l - 8);
				s[0] = 'S';
				s[1] = 'T';
				s[2] = 'A';
				s[3] = 'R';
				s[4] = (c >>  0) & 0xff;
				s[5] = (c >>  8) & 0xff;
				s[6] = (c >> 16) & 0xff;
				s[7] = (c >> 24) & 0xff;
				__setting_write(setting, s, 0, l);
				__setting_sync(setting);
				xos_mem_free(s);
			}
		}
		setting->dirty = 0;
		xos_mutex_unlock(&setting->lock);
	}
	return 0;
}

static void hmap_entry_callback(struct hmap_t * m, struct hmap_entry_t * e)
{
	if(e)
		xos_mem_free(e->value);
}

static void * setting_block_detect(void)
{
	struct device_t * pos, * n;

	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_BLOCK], head)
	{
		struct block_t * blk = (struct block_t *)pos->priv;
		if(xos_strstr(blk->name, "setting"))
			return blk;
	}
	return NULL;
}

static uint64_t setting_block_capacity(struct setting_t * setting)
{
	struct block_t * blk = (struct block_t *)setting->priv;
	return block_capacity(blk);
}

static uint64_t setting_block_read(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count)
{
	struct block_t * blk = (struct block_t *)setting->priv;
	return block_read(blk, buf, offset, count);
}

static uint64_t setting_block_write(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count)
{
	struct block_t * blk = (struct block_t *)setting->priv;
	return block_write(blk, buf, offset, count);
}

static void setting_block_sync(struct setting_t * setting)
{
	struct block_t * blk = (struct block_t *)setting->priv;
	return block_sync(blk);
}

static void * setting_file_detect(void)
{
	struct xfs_context_t * ctx = xfs_alloc();
	void * result = NULL;

	if(ctx)
	{
		struct xfs_file_t * file = xfs_open_append(ctx, "setting.db");
		if(file)
		{
			xfs_close(file);
			result = (void *)0xcafebabe;
		}
		xfs_free(ctx);
	}
	return result;
}

static uint64_t setting_file_capacity(struct setting_t * setting)
{
	return 1 * 1024 * 1024;
}

static uint64_t setting_file_read(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count)
{
	struct xfs_context_t * ctx = xfs_alloc();
	int64_t n = 0;

	if(ctx)
	{
		struct xfs_file_t * file = xfs_open_read(ctx, "setting.db");
		if(file)
		{
			int64_t len = xfs_length(file);
			if((len > 0) && (offset < len))
			{
				xfs_seek(file, offset);
				n = xfs_read(file, buf, count);
			}
			xfs_close(file);
		}
		xfs_free(ctx);
	}
	return n;
}

static uint64_t setting_file_write(struct setting_t * setting, void * buf, uint64_t offset, uint64_t count)
{
	struct xfs_context_t * ctx = xfs_alloc();
	int64_t n = 0;

	if(ctx)
	{
		struct xfs_file_t * file = xfs_open_write(ctx, "setting.db");
		if(file)
		{
			n = xfs_write(file, buf, count);
			xfs_close(file);
		}
		xfs_free(ctx);
	}
	return n;
}

static void setting_file_sync(struct setting_t * setting)
{
}

void do_init_setting(void)
{
	struct setting_t * setting = &__setting;
	void * priv;

	timer_init(&setting->timer, setting_timer_function, setting);
	setting->map = hmap_alloc(0, hmap_entry_callback);
	xos_mutex_init(&setting->lock);
	setting->dirty = 0;

	if((priv = setting_block_detect()))
	{
		setting->capacity = setting_block_capacity;
		setting->read = setting_block_read;
		setting->write = setting_block_write;
		setting->sync = setting_block_sync;
		setting->priv = priv;
	}
	else if((priv = setting_file_detect()))
	{
		setting->capacity = setting_file_capacity;
		setting->read = setting_file_read;
		setting->write = setting_file_write;
		setting->sync = setting_file_sync;
		setting->priv = priv;
	}
	else
	{
		setting->capacity = NULL;
		setting->read = NULL;
		setting->write = NULL;
		setting->sync = NULL;
		setting->priv = NULL;
	}

	xos_mutex_lock(&setting->lock);
	uint64_t size = __setting_capacity(setting);
	uint8_t h[12];
	if((size > 12) && (__setting_read(setting, h, 0, 12) == 12))
	{
		if((h[0] == 'S') && (h[1] == 'T') && (h[2] == 'A') && (h[3] == 'R'))
		{
			uint32_t c = (h[7] << 24) | (h[6] << 16) | (h[5] << 8) | (h[4] << 0);
			uint64_t l = (h[11] << 24) | (h[10] << 16) | (h[9] << 8) | (h[8] << 0);
			if((l > 0) && (l + 12 < size))
			{
				unsigned char * s = xos_mem_malloc(l);
				if(__setting_read(setting, s, 12, l) == l)
				{
					uint32_t crc = 0;
					crc = crc32_sum(crc, (const uint8_t *)(&h[8]), 4);
					crc = crc32_sum(crc, (const uint8_t *)s, l);
					if(crc == c)
					{
						char * p = (char * )s;
						char * r, * k, * v;
						while((r = xos_strsep(&p, "|")) != NULL)
						{
							if(xos_strchr(r, ':'))
							{
								k = xos_strim(xos_strsep(&r, ":"));
								v = xos_strim(r);
								k = (k && (*k != '\0')) ? k : NULL;
								v = (v && (*v != '\0')) ? v : NULL;
								if(k && v)
									hmap_add(setting->map, k, xos_strdup(v));
							}
						}
					}
				}
				xos_mem_free(s);
			}
		}
	}
	xos_mutex_unlock(&setting->lock);
}
