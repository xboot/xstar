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
#include <libx/lsort.h>
#include <libx/slist.h>

struct slist_t * slist_alloc(void)
{
	struct slist_t * sl;

	sl = xos_mem_malloc(sizeof(struct slist_t));
	if(!sl)
		return NULL;

	sl->key = NULL;
	init_list_head(&sl->list);
	return sl;
}

void slist_free(struct slist_t * sl)
{
	struct slist_t * pos, * n;

	if(sl)
	{
		list_for_each_entry_safe(pos, n, &sl->list, list)
		{
			if(pos->key)
				xos_mem_free(pos->key);
			xos_mem_free(pos);
		}
		if(sl->key)
			xos_mem_free(sl->key);
		xos_mem_free(sl);
	}
}

static char * slist_vasprintf(const char * fmt, va_list ap)
{
	char * p;
	int len;

	len = xos_vasprintf(&p, fmt, ap);
	if(len < 0)
		return NULL;
	return p;
}

void slist_add(struct slist_t * sl, void * priv, const char * fmt, ...)
{
	struct slist_t * n;
	va_list ap;

	if(!sl)
		return;

	n = xos_mem_malloc(sizeof(struct slist_t));
	if(!n)
		return;

	va_start(ap, fmt);
	n->key = slist_vasprintf(fmt, ap);
	va_end(ap);
	n->priv = priv;
	if(n->key)
		list_add_tail(&n->list, &sl->list);
	else
		xos_mem_free(n);
}

static int slist_compare(void * priv, struct list_head_t * a, struct list_head_t * b)
{
	char * keya = (char *)list_entry(a, struct slist_t, list)->key;
	char * keyb = (char *)list_entry(b, struct slist_t, list)->key;
	return xos_strcmp(keya, keyb);
}

void slist_sort(struct slist_t * sl)
{
	if(sl)
		lsort(NULL, &sl->list, slist_compare);
}

static int slist_compare_natural(void * priv, struct list_head_t * a, struct list_head_t * b)
{
	char * keya = (char *)list_entry(a, struct slist_t, list)->key;
	char * keyb = (char *)list_entry(b, struct slist_t, list)->key;
	return xos_strnatcmp(keya, keyb);
}

void slist_natsort(struct slist_t * sl)
{
	if(sl)
		lsort(NULL, &sl->list, slist_compare_natural);
}

void slist_sort_with(struct slist_t * sl, int (*cmp)(void *, struct list_head_t *, struct list_head_t *))
{
	if(sl && cmp)
		lsort(NULL, &sl->list, cmp);
}

int slist_isempty(struct slist_t * sl)
{
	if(sl && !list_empty_careful(&sl->list))
		return 0;
	return 1;
}
