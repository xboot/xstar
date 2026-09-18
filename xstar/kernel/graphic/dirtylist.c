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
	if(size < 16)
		size = 16;

	struct dirtylist_item_t * items = xos_mem_malloc(size * sizeof(struct dirtylist_item_t));
	if(!items)
		return NULL;

	struct dirtylist_t * l = xos_mem_malloc(sizeof(struct dirtylist_t));
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
	if(l && (l->size < size))
	{
		l->size = size;
		l->items = xos_mem_realloc(l->items, l->size * sizeof(struct dirtylist_item_t));
	}
}

void dirtylist_clone(struct dirtylist_t * l, struct dirtylist_t * o)
{
	if(l)
	{
		if(!o)
			l->count = 0;
		else
		{
			if(l->size < o->size)
				dirtylist_resize(l, o->size);
			if(o->count > 0)
				xos_memcpy(l->items, o->items, sizeof(struct dirtylist_item_t) * o->count);
			l->count = o->count;
		}
	}
}

void dirtylist_merge(struct dirtylist_t * l, struct dirtylist_t * o)
{
	if(l && o)
	{
		for(int i = 0; i < o->count; i++)
			dirtylist_add(l, &o->items[i].region);
	}
}

void dirtylist_clear(struct dirtylist_t * l)
{
	if(l)
		l->count = 0;
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
	}
}

/*
 * Exact union stage: sweep the sorted y coordinates in bands, merge the
 * overlapping x spans within each band, then coalesce vertically adjacent
 * rects with identical x spans. The output is pairwise non-overlapping
 * and covers every input pixel exactly once.
 */
struct __dirtylist_span_t {
	int x1;
	int x2;
};

static void __dirtylist_sort_int(int * a, int n)
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

static void __dirtylist_sort_span(struct __dirtylist_span_t * a, int n)
{
	for(int i = 1; i < n; i++)
	{
		struct __dirtylist_span_t v = a[i];
		int j = i - 1;
		while((j >= 0) && (a[j].x1 > v.x1))
		{
			a[j + 1] = a[j];
			j--;
		}
		a[j + 1] = v;
	}
}

static void __dirtylist_optimize_exact(struct dirtylist_t * l, int max)
{
	if(l->count > 1)
	{
		int k = l->count;
		int * ys = xos_mem_malloc(sizeof(int) * k * 2);
		struct __dirtylist_span_t * spans = xos_mem_malloc(sizeof(struct __dirtylist_span_t) * k);
		unsigned int outsz = (k < 16) ? 16 : k;
		int outcnt = 0;
		struct region_t * out = xos_mem_malloc(sizeof(struct region_t) * outsz);

		if(!ys || !spans || !out)
		{
			if(ys)
				xos_mem_free(ys);
			if(spans)
				xos_mem_free(spans);
			if(out)
				xos_mem_free(out);
			return;
		}

		int nys = 0;
		for(int i = 0; i < k; i++)
		{
			struct region_t * r = &l->items[i].region;
			if((r->w > 0) && (r->h > 0))
			{
				ys[nys++] = r->y;
				ys[nys++] = r->y + r->h;
			}
		}
		if(nys > 0)
		{
			__dirtylist_sort_int(ys, nys);
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
					struct region_t * r = &l->items[i].region;
					if((r->w > 0) && (r->h > 0) && (r->y <= y1) && (y2 <= r->y + r->h))
					{
						spans[ns].x1 = r->x;
						spans[ns].x2 = r->x + r->w;
						ns++;
					}
				}
				if(ns > 0)
				{
					while((unsigned int)(outcnt + ns) > outsz)
						outsz <<= 1;
					if(outsz > ((k < 16) ? 16 : k))
					{
						struct region_t * no = xos_mem_realloc(out, sizeof(struct region_t) * outsz);
						if(!no)
						{
							xos_mem_free(ys);
							xos_mem_free(spans);
							xos_mem_free(out);
							return;
						}
						out = no;
					}
					__dirtylist_sort_span(spans, ns);
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

		xos_mem_free(ys);
		xos_mem_free(spans);

		if(outcnt == 0)
		{
			l->count = 0;
			xos_mem_free(out);
			return;
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

		if(outcnt > max)
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

		if(l->size < (unsigned int)outcnt)
			dirtylist_resize(l, outcnt);
		for(int i = 0; i < outcnt; i++)
		{
			l->items[i].region = out[i];
			l->items[i].area = out[i].w * out[i].h;
		}
		l->count = outcnt;
		xos_mem_free(out);
	}
}

/*
 * Penalty compression stage: repeatedly merge the pair whose bounding box
 * adds the fewest extra pixels until at most n rects remain. Each merge
 * trades some pixel overdraw for a smaller rect count.
 */
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

static inline int __dirtylist_area_penalty(struct dirtylist_t * l, int i, int j)
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

static inline void __dirtylist_optimize_penalty(struct dirtylist_t * l)
{
	int area_min = INT_MAX;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __dirtylist_area_penalty(l, i, j);
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
		__dirtylist_optimize_exact(l, 128);
		if(n > 0)
		{
			while(l->count > n)
				__dirtylist_optimize_penalty(l);
		}
	}
}
