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

#include <kernel/graphic/dirtylist.h>

struct dirtylist_t * dirtylist_alloc(unsigned int size)
{
	struct dirtylist_t * l;
	struct dirtylist_item_t * items;

	if(size < 16)
		size = 16;

	items = xos_mem_malloc(size * sizeof(struct dirtylist_item_t));
	if(!items)
		return NULL;

	l = xos_mem_malloc(sizeof(struct dirtylist_t));
	if(!l)
	{
		xos_mem_free(items);
		return NULL;
	}

	l->items = items;
	l->size = size;
	l->count = 0;
	return l;
}

void dirtylist_free(struct dirtylist_t * l)
{
	if(l)
	{
		xos_mem_free(l->items);
		xos_mem_free(l);
	}
}

static inline void dirtylist_resize(struct dirtylist_t * l, unsigned int size)
{
	if(l && (l->size != size))
	{
		if(size < 16)
			size = 16;
		l->size = size;
		l->items = xos_mem_realloc(l->items, l->size * sizeof(struct dirtylist_item_t));
	}
}

void dirtylist_clone(struct dirtylist_t * l, struct dirtylist_t * o)
{
	int count;

	if(l)
	{
		if(!o)
			l->count = 0;
		else
		{
			if(l->size < o->size)
				dirtylist_resize(l, o->size);
			if((count = o->count) > 0)
				xos_memcpy(l->items, o->items, sizeof(struct dirtylist_item_t) * count);
			l->count = count;
		}
	}
}

void dirtylist_merge(struct dirtylist_t * l, struct dirtylist_t * o)
{
	int count;
	int i;

	if(l && o && ((count = o->count) > 0))
	{
		for(i = 0; i < count; i++)
			dirtylist_add(l, &o->items[i].region);
	}
}

void dirtylist_clear(struct dirtylist_t * l)
{
	if(l)
		l->count = 0;
}

static inline int __region_area_intersect(struct region_t * a, struct region_t * b)
{
	int x0 = XMAX(a->x, b->x);
	int x1 = XMIN(a->x + a->w, b->x + b->w);
	if(x0 <= x1)
	{
		int y0 = XMAX(a->y, b->y);
		int y1 = XMIN(a->y + a->h, b->y + b->h);
		if(y0 <= y1)
			return (x1 - x0) * (y1 - y0);
	}
	return 0;
}

static inline int __region_area_union(struct region_t * a, struct region_t * b)
{
	int ar = a->x + a->w;
	int ab = a->y + a->h;
	int br = b->x + b->w;
	int bb = b->y + b->h;
	int w = XMAX(ar, br) - XMIN(a->x, b->x);
	int h = XMAX(ab, bb) - XMIN(a->y, b->y);
	return w * h;
}

static inline int __dirtylist_area_rule1(struct dirtylist_t * l, int i, int j)
{
	struct dirtylist_item_t * p = &l->items[i];
	struct dirtylist_item_t * q = &l->items[j];

	if(__region_area_union(&p->region, &q->region) < (p->area + q->area))
		return __region_area_intersect(&p->region, &q->region);
	return 0;
}

static inline int __dirtylist_area_rule2(struct dirtylist_t * l, int i, int j)
{
	struct dirtylist_item_t * p = &l->items[i];
	struct dirtylist_item_t * q = &l->items[j];

	return __region_area_union(&p->region, &q->region) - (p->area + q->area);
}

static inline int __dirtylist_flush(struct dirtylist_t * l)
{
	int count = l->count;

	l->count = 0;
	for(int i = 0; i < count; i++)
	{
		struct dirtylist_item_t * p = &l->items[i];
		if(p->area > 0)
		{
			struct dirtylist_item_t * q = &l->items[l->count];
			if(q != p)
				xos_memcpy(q, p, sizeof(struct dirtylist_item_t));
			l->count++;
		}
	}
	return l->count;
}

static inline int __dirtylist_optimize_rule1(struct dirtylist_t * l)
{
	int area_max = 0;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __dirtylist_area_rule1(l, i, j);
				if(area_max < area)
				{
					area_max = area;
					best_i = i;
					best_j = j;
				}
			}
		}
	}
	if(area_max > 0)
	{
		struct dirtylist_item_t * p = &l->items[XMIN(best_i, best_j)];
		struct dirtylist_item_t * q = &l->items[XMAX(best_i, best_j)];
		region_union(&p->region, &p->region, &q->region);
		p->area = p->region.w * p->region.h;
		q->area = 0;
		__dirtylist_flush(l);
		return 1;
	}
	return 0;
}

static inline int __dirtylist_optimize_rule2(struct dirtylist_t * l)
{
	int area_min = INT_MAX;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __dirtylist_area_rule2(l, i, j);
				if(area_min > area)
				{
					area_min = area;
					best_i = i;
					best_j = j;
				}
			}
		}
	}
	if(area_min < INT_MAX)
	{
		struct dirtylist_item_t * p = &l->items[XMIN(best_i, best_j)];
		struct dirtylist_item_t * q = &l->items[XMAX(best_i, best_j)];
		region_union(&p->region, &p->region, &q->region);
		p->area = p->region.w * p->region.h;
		q->area = 0;
		__dirtylist_flush(l);
		return 1;
	}
	return 0;
}

static inline void dirtylist_optimize(struct dirtylist_t * l, int n)
{
	if(l->count > 1)
	{
		while(__dirtylist_optimize_rule1(l));
		while(l->count > n)
			__dirtylist_optimize_rule2(l);
	}
}

void dirtylist_add(struct dirtylist_t * l, struct region_t * r)
{
	if(l && r)
	{
		if(l->size <= l->count)
			dirtylist_resize(l, l->size << 1);
		struct dirtylist_item_t * item = &l->items[l->count];
		item->region.x = r->x;
		item->region.y = r->y;
		item->region.w = r->w;
		item->region.h = r->h;
		item->area = r->w * r->h;
		l->count++;
		dirtylist_optimize(l, 3);
	}
}
