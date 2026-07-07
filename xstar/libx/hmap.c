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

#include <xos/xos.h>
#include <libx/hmap.h>

struct hmap_t * hmap_alloc(int size, void (*cb)(struct hmap_t *, struct hmap_entry_t *))
{
	struct hmap_t * m;
	int i;

	if(size < 16)
		size = 16;
	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	m = xos_mem_malloc(sizeof(struct hmap_t));
	if(!m)
		return NULL;

	m->hash = xos_mem_malloc(sizeof(struct hlist_head_t) * size);
	if(!m->hash)
	{
		xos_mem_free(m);
		return NULL;
	}
	for(i = 0; i < size; i++)
		init_hlist_head(&m->hash[i]);
	init_list_head(&m->list);
	xos_mutex_init(&m->lock);
	m->size = size;
	m->n = 0;
	m->callback = cb;

	return m;
}

void hmap_free(struct hmap_t * m)
{
	if(m)
	{
		hmap_clear(m);
		xos_mutex_exit(&m->lock);
		xos_mem_free(m->hash);
		xos_mem_free(m);
	}
}

void hmap_clear(struct hmap_t * m)
{
	struct hmap_entry_t * pos, * n;

	if(m)
	{
		list_for_each_entry_safe(pos, n, &m->list, head)
		{
			xos_mutex_lock(&m->lock);
			hlist_del(&pos->node);
			list_del(&pos->head);
			m->n--;
			xos_mutex_unlock(&m->lock);
			if(m->callback)
				m->callback(m, pos);
			xos_mem_free(pos->key);
			xos_mem_free(pos);
		}
	}
}

static void hmap_resize(struct hmap_t * m, unsigned int size)
{
	struct hmap_entry_t * pos, * n;
	struct hlist_head_t * hash;
	int i;

	if(!m)
		return;

	if(size < 16)
		size = 16;
	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	hash = xos_mem_malloc(sizeof(struct hlist_head_t) * size);
	if(!hash)
		return;
	for(i = 0; i < size; i++)
		init_hlist_head(&hash[i]);

	xos_mutex_lock(&m->lock);
	list_for_each_entry_safe(pos, n, &m->list, head)
	{
		hlist_del(&pos->node);
	}
	xos_mutex_unlock(&m->lock);
	xos_mem_free(m->hash);

	xos_mutex_lock(&m->lock);
	m->hash = hash;
	m->size = size;
	list_for_each_entry_safe(pos, n, &m->list, head)
	{
		hlist_add_head(&pos->node, &m->hash[shash(pos->key) & (m->size - 1)]);
	}
	xos_mutex_unlock(&m->lock);
}

void hmap_add(struct hmap_t * m, const char * key, void * value)
{
	struct hmap_entry_t * pos;
	struct hlist_node_t * n;

	if(!m || !key)
		return;

	hlist_for_each_entry_safe(pos, n, &m->hash[shash(key) & (m->size - 1)], node)
	{
		if(xos_strcmp(pos->key, key) == 0)
		{
			if(pos->value != value)
				pos->value = value;
			return;
		}
	}

	if(m->n > (m->size >> 1))
		hmap_resize(m, m->size << 1);

	pos = xos_mem_malloc(sizeof(struct hmap_entry_t));
	if(!pos)
		return;

	pos->key = xos_strdup(key);
	pos->value = value;
	xos_mutex_lock(&m->lock);
	init_hlist_node(&pos->node);
	hlist_add_head(&pos->node, &m->hash[shash(pos->key) & (m->size - 1)]);
	init_list_head(&pos->head);
	list_add_tail(&pos->head, &m->list);
	m->n++;
	xos_mutex_unlock(&m->lock);
}

void hmap_remove(struct hmap_t * m, const char * key)
{
	struct hmap_entry_t * pos;
	struct hlist_node_t * n;

	if(!m || !key)
		return;

	if((m->size > 16) && (m->n < (m->size >> 2)))
		hmap_resize(m, m->size >> 1);

	hlist_for_each_entry_safe(pos, n, &m->hash[shash(key) & (m->size - 1)], node)
	{
		if(xos_strcmp(pos->key, key) == 0)
		{
			xos_mutex_lock(&m->lock);
			hlist_del(&pos->node);
			list_del(&pos->head);
			m->n--;
			xos_mutex_unlock(&m->lock);
			xos_mem_free(pos->key);
			xos_mem_free(pos);
			return;
		}
	}
}

static int hmap_compare(void * priv, struct list_head_t * a, struct list_head_t * b)
{
	char * keya = (char *)list_entry(a, struct hmap_entry_t, head)->key;
	char * keyb = (char *)list_entry(b, struct hmap_entry_t, head)->key;
	return xos_strcmp(keya, keyb);
}

void hmap_sort(struct hmap_t * m)
{
	if(m)
	{
		xos_mutex_lock(&m->lock);
		lsort(NULL, &m->list, hmap_compare);
		xos_mutex_unlock(&m->lock);
	}
}

static int hmap_compare_natural(void * priv, struct list_head_t * a, struct list_head_t * b)
{
	char * keya = (char *)list_entry(a, struct hmap_entry_t, head)->key;
	char * keyb = (char *)list_entry(b, struct hmap_entry_t, head)->key;
	return xos_strnatcmp(keya, keyb);
}

void hmap_natsort(struct hmap_t * m)
{
	if(m)
	{
		xos_mutex_lock(&m->lock);
		lsort(NULL, &m->list, hmap_compare_natural);
		xos_mutex_unlock(&m->lock);
	}
}

void hmap_sort_with(struct hmap_t * m, int (*cmp)(void *, struct list_head_t *, struct list_head_t *))
{
	if(m && cmp)
	{
		xos_mutex_lock(&m->lock);
		lsort(NULL, &m->list, cmp);
		xos_mutex_unlock(&m->lock);
	}
}

void * hmap_search(struct hmap_t * m, const char * key)
{
	struct hmap_entry_t * pos;
	struct hlist_node_t * n;

	if(!m || !key)
		return NULL;

	hlist_for_each_entry_safe(pos, n, &m->hash[shash(key) & (m->size - 1)], node)
	{
		if(xos_strcmp(pos->key, key) == 0)
			return pos->value;
	}
	return NULL;
}
