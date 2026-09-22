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

struct dirtylist_span_t {
	int x1;
	int x2;
};

struct dirtylist_workspace_t {
	int * ys;
	unsigned int ys_capacity;
	int * areas;
	unsigned int areas_capacity;
	struct dirtylist_span_t * spans;
	unsigned int spans_capacity;
	struct region_t * out;
	unsigned int out_capacity;
};

static inline unsigned int dirtylist_capacity(unsigned int size)
{
	unsigned int max = UINT_MAX / sizeof(struct region_t);

	if(size < 16)
		size = 16;
	if(size > max)
		return 0;
	if(size & (size - 1))
		size = roundup_pow_of_two(size);
	return (size <= max) ? size : 0;
}

struct dirtylist_t * dirtylist_alloc(unsigned int size)
{
	size = dirtylist_capacity(size);
	if(!size)
		return NULL;

	struct region_t * items = xos_mem_malloc(size * sizeof(struct region_t));
	if(!items)
		return NULL;

	struct dirtylist_t * l = xos_mem_malloc(sizeof(struct dirtylist_t));
	if(!l)
	{
		xos_mem_free(items);
		return NULL;
	}

	l->items = items;
	l->nitems = 0;
	l->size = size;
	l->priv = NULL;
	return l;
}

void dirtylist_free(struct dirtylist_t * l)
{
	if(l)
	{
		struct dirtylist_workspace_t * w = l->priv;
		if(w)
		{
			xos_mem_free(w->ys);
			xos_mem_free(w->areas);
			xos_mem_free(w->spans);
			xos_mem_free(w->out);
			xos_mem_free(w);
		}
		xos_mem_free(l->items);
		xos_mem_free(l);
	}
}

static inline int dirtylist_resize(struct dirtylist_t * l, unsigned int size)
{
	if(!l)
		return 0;
	size = dirtylist_capacity(size);
	if(!size)
		return 0;
	if(l->size < size)
	{
		struct region_t * items = xos_mem_realloc(l->items, size * sizeof(struct region_t));
		if(!items)
			return 0;
		l->items = items;
		l->size = size;
	}
	return 1;
}

void dirtylist_clone(struct dirtylist_t * l, struct dirtylist_t * o)
{
	if(l)
	{
		if(!o)
			l->nitems = 0;
		else
		{
			if((l->size < o->size) && !dirtylist_resize(l, o->size))
				return;
			if(o->nitems > 0)
				xos_memcpy(l->items, o->items, sizeof(struct region_t) * o->nitems);
			l->nitems = o->nitems;
		}
	}
}

void dirtylist_merge(struct dirtylist_t * l, struct dirtylist_t * o)
{
	if(l && o && (l != o))
	{
		for(int i = 0; i < o->nitems; i++)
			dirtylist_add(l, &o->items[i]);
	}
}

void dirtylist_clear(struct dirtylist_t * l)
{
	if(l)
		l->nitems = 0;
}

void dirtylist_add(struct dirtylist_t * l, struct region_t * r)
{
	if(l && r)
	{
		if(l->size <= l->nitems)
		{
			if((l->size > (UINT_MAX >> 1)) || !dirtylist_resize(l, l->size << 1))
				return;
		}
		l->items[l->nitems] = *r;
		l->nitems++;
	}
}

/*
 * Exact union stage: sweep the sorted y coordinates in bands, merge the
 * overlapping x spans within each band, then coalesce vertically adjacent
 * rects with identical x spans. The output is pairwise non-overlapping
 * and covers every input pixel exactly once.
 */
static void dirtylist_sort_int(int * a, int n)
{
	for(int i = 1; i < n; i++)
	{
		int v = a[i];
		int j = i - 1;
		while((j >= 0) && (a[j] > v))
		{
			a[j + 1] = a[j];
			j--;
		}
		a[j + 1] = v;
	}
}

static void dirtylist_sort_span(struct dirtylist_span_t * a, int n)
{
	for(int i = 1; i < n; i++)
	{
		struct dirtylist_span_t v = a[i];
		int j = i - 1;
		while((j >= 0) && (a[j].x1 > v.x1))
		{
			a[j + 1] = a[j];
			j--;
		}
		a[j + 1] = v;
	}
}

static int dirtylist_workspace_reserve_output(struct dirtylist_workspace_t * w, unsigned int size)
{
	void * p;

	if(w->out_capacity < size)
	{
		if(w->out)
			p = xos_mem_realloc(w->out, sizeof(struct region_t) * size);
		else
			p = xos_mem_malloc(sizeof(struct region_t) * size);
		if(!p)
			return 0;
		w->out = p;
		w->out_capacity = size;
	}
	if(w->areas_capacity < size)
	{
		if(w->areas)
			p = xos_mem_realloc(w->areas, sizeof(int) * size);
		else
			p = xos_mem_malloc(sizeof(int) * size);
		if(!p)
			return 0;
		w->areas = p;
		w->areas_capacity = size;
	}
	return 1;
}

static struct dirtylist_workspace_t * dirtylist_workspace_reserve(struct dirtylist_t * l, unsigned int count)
{
	struct dirtylist_workspace_t * w = l->priv;
	void * p;
	unsigned int capacity = dirtylist_capacity(count);

	if(!capacity)
		return NULL;
	if(!w)
	{
		w = xos_mem_malloc(sizeof(struct dirtylist_workspace_t));
		if(!w)
			return NULL;
		w->ys = NULL;
		w->areas = NULL;
		w->spans = NULL;
		w->out = NULL;
		w->ys_capacity = 0;
		w->areas_capacity = 0;
		w->spans_capacity = 0;
		w->out_capacity = 0;
		l->priv = w;
	}
	if(w->ys_capacity < capacity * 2)
	{
		if(w->ys)
			p = xos_mem_realloc(w->ys, sizeof(int) * capacity * 2);
		else
			p = xos_mem_malloc(sizeof(int) * capacity * 2);
		if(!p)
			return NULL;
		w->ys = p;
		w->ys_capacity = capacity * 2;
	}
	if(w->spans_capacity < capacity)
	{
		if(w->spans)
			p = xos_mem_realloc(w->spans, sizeof(struct dirtylist_span_t) * capacity);
		else
			p = xos_mem_malloc(sizeof(struct dirtylist_span_t) * capacity);
		if(!p)
			return NULL;
		w->spans = p;
		w->spans_capacity = capacity;
	}
	if(!dirtylist_workspace_reserve_output(w, capacity))
		return NULL;
	return w;
}

static int dirtylist_optimize_exact(struct dirtylist_t * l, int collapse_threshold)
{
	if(l->nitems == 1)
	{
		if((l->items[0].w <= 0) || (l->items[0].h <= 0))
			l->nitems = 0;
		return 1;
	}
	if(l->nitems > 1)
	{
		int k = l->nitems;
		struct dirtylist_workspace_t * w = dirtylist_workspace_reserve(l, k);
		if(!w)
			return 0;
		int * ys = w->ys;
		struct dirtylist_span_t * spans = w->spans;
		struct region_t * out = w->out;
		int outcnt = 0;

		int nys = 0;
		for(int i = 0; i < k; i++)
		{
			struct region_t * r = &l->items[i];
			if((r->w > 0) && (r->h > 0))
			{
				ys[nys++] = r->y;
				ys[nys++] = r->y + r->h;
			}
		}
		if(nys > 0)
		{
			dirtylist_sort_int(ys, nys);
			int m = 1;
			for(int i = 1; i < nys; i++)
			{
				if(ys[i] != ys[m - 1])
					ys[m++] = ys[i];
			}
			for(int b = 0; b + 1 < m; b++)
			{
				int y1 = ys[b];
				int y2 = ys[b + 1];
				int ns = 0;
				for(int i = 0; i < k; i++)
				{
					struct region_t * r = &l->items[i];
					if((r->w > 0) && (r->h > 0) && (r->y <= y1) && (y2 <= r->y + r->h))
					{
						spans[ns].x1 = r->x;
						spans[ns].x2 = r->x + r->w;
						ns++;
					}
				}
				if(ns > 0)
				{
					if((unsigned int)(outcnt + ns) > w->out_capacity)
					{
						unsigned int size = w->out_capacity;
						while((unsigned int)(outcnt + ns) > size)
							size <<= 1;
						if(!dirtylist_workspace_reserve_output(w, size))
							return 0;
						out = w->out;
					}
					dirtylist_sort_span(spans, ns);
					int x1 = spans[0].x1;
					int x2 = spans[0].x2;
					for(int i = 1; i < ns; i++)
					{
						if(spans[i].x1 <= x2)
						{
							if(spans[i].x2 > x2)
								x2 = spans[i].x2;
						}
						else
						{
							out[outcnt].x = x1;
							out[outcnt].y = y1;
							out[outcnt].w = x2 - x1;
							out[outcnt].h = y2 - y1;
							outcnt++;
							x1 = spans[i].x1;
							x2 = spans[i].x2;
						}
					}
					out[outcnt].x = x1;
					out[outcnt].y = y1;
					out[outcnt].w = x2 - x1;
					out[outcnt].h = y2 - y1;
					outcnt++;
				}
			}
		}

		if(outcnt == 0)
		{
			l->nitems = 0;
			return 1;
		}

		int changed = 1;
		while(changed)
		{
			changed = 0;
			for(int i = 0; i < outcnt; i++)
			{
				for(int j = 0; j < i; j++)
				{
					if((out[j].y + out[j].h == out[i].y) && (out[j].x == out[i].x) && (out[j].w == out[i].w))
					{
						out[j].h += out[i].h;
						out[i].w = 0;
						out[i].h = 0;
						changed = 1;
						break;
					}
				}
			}
			int m2 = 0;
			for(int i = 0; i < outcnt; i++)
			{
				if((out[i].w > 0) && (out[i].h > 0))
					out[m2++] = out[i];
			}
			outcnt = m2;
		}

		if(outcnt > collapse_threshold)
		{
			int x1 = out[0].x;
			int y1 = out[0].y;
			int x2 = x1 + out[0].w;
			int y2 = y1 + out[0].h;
			for(int i = 1; i < outcnt; i++)
			{
				if(out[i].x < x1)
					x1 = out[i].x;
				if(out[i].y < y1)
					y1 = out[i].y;
				if(out[i].x + out[i].w > x2)
					x2 = out[i].x + out[i].w;
				if(out[i].y + out[i].h > y2)
					y2 = out[i].y + out[i].h;
			}
			out[0].x = x1;
			out[0].y = y1;
			out[0].w = x2 - x1;
			out[0].h = y2 - y1;
			outcnt = 1;
		}

		if((l->size < (unsigned int)outcnt) && !dirtylist_resize(l, outcnt))
			return 0;
		for(int i = 0; i < outcnt; i++)
		{
			l->items[i] = out[i];
			w->areas[i] = out[i].w * out[i].h;
		}
		l->nitems = outcnt;
	}
	return 1;
}

/*
 * Penalty compression stage: repeatedly merge the pair whose bounding box
 * adds the fewest extra pixels until at most n rects remain. Each merge
 * trades some pixel overdraw for a smaller rect count.
 */
static inline int dirtylist_region_union_area(struct region_t * a, struct region_t * b)
{
	int ar = a->x + a->w;
	int ab = a->y + a->h;
	int br = b->x + b->w;
	int bb = b->y + b->h;
	int w = XMAX(ar, br) - XMIN(a->x, b->x);
	int h = XMAX(ab, bb) - XMIN(a->y, b->y);
	return w * h;
}

static inline int dirtylist_region_intersection_area(struct region_t * a, struct region_t * b)
{
	int w = XMIN(a->x + a->w, b->x + b->w) - XMAX(a->x, b->x);
	int h = XMIN(a->y + a->h, b->y + b->h) - XMAX(a->y, b->y);
	return ((w > 0) && (h > 0)) ? w * h : 0;
}

static inline int dirtylist_area_penalty(struct dirtylist_t * l, int * areas, int i, int j)
{
	struct region_t * p = &l->items[i];
	struct region_t * q = &l->items[j];
	return dirtylist_region_union_area(p, q) - areas[i] - areas[j] + dirtylist_region_intersection_area(p, q);
}

static inline int dirtylist_flush(struct dirtylist_t * l, int * areas)
{
	int count = l->nitems;

	l->nitems = 0;
	for(int i = 0; i < count; i++)
	{
		struct region_t * p = &l->items[i];
		if(areas[i] > 0)
		{
			int n = l->nitems;
			if(n != i)
			{
				l->items[n] = *p;
				areas[n] = areas[i];
			}
			l->nitems++;
		}
	}
	return l->nitems;
}

static inline void dirtylist_optimize_penalty(struct dirtylist_t * l, int * areas)
{
	int area_min = INT_MAX;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->nitems - 1; i++)
	{
		for(int j = i + 1; j < l->nitems; j++)
		{
			int area = dirtylist_area_penalty(l, areas, i, j);
			if(area_min > area)
			{
				area_min = area;
				best_i = i;
				best_j = j;
			}
		}
	}
	if(area_min < INT_MAX)
	{
		struct region_t * p = &l->items[best_i];
		struct region_t * q = &l->items[best_j];
		region_union(p, p, q);
		areas[best_i] = p->w * p->h;
		areas[best_j] = 0;
		dirtylist_flush(l, areas);
	}
}

/*
 * Rebuild the list as a pixel-exact non-overlapping union, then merge the
 * least-penalty pairs until at most n rects remain. n <= 0 skips compression.
 */
void dirtylist_optimize(struct dirtylist_t * l, int n)
{
	if(l)
	{
		if(dirtylist_optimize_exact(l, 32) && (n > 0))
		{
			struct dirtylist_workspace_t * w = l->priv;
			while(l->nitems > n)
				dirtylist_optimize_penalty(l, w->areas);
		}
	}
}
