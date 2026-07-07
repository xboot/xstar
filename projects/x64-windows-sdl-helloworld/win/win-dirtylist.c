#include <win/win.h>

struct win_dirtylist_t * win_dirtylist_alloc(unsigned int size)
{
	struct win_dirtylist_t * l;
	struct win_dirtylist_item_t * items;

	if(size < 16)
		size = 16;

	items = win_mem_malloc(size * sizeof(struct win_dirtylist_item_t));
	if(!items)
		return NULL;

	l = win_mem_malloc(sizeof(struct win_dirtylist_t));
	if(!l)
	{
		win_mem_free(items);
		return NULL;
	}

	l->items = items;
	l->size = size;
	l->count = 0;
	return l;
}

void win_dirtylist_free(struct win_dirtylist_t * l)
{
	if(l)
	{
		win_mem_free(l->items);
		win_mem_free(l);
	}
}

static inline void win_dirtylist_resize(struct win_dirtylist_t * l, unsigned int size)
{
	if(l && (l->size != size))
	{
		if(size < 16)
			size = 16;
		l->size = size;
		l->items = win_mem_realloc(l->items, l->size * sizeof(struct win_dirtylist_item_t));
	}
}

void win_dirtylist_clone(struct win_dirtylist_t * l, struct win_dirtylist_t * o)
{
	int count;

	if(l)
	{
		if(!o)
			l->count = 0;
		else
		{
			if(l->size < o->size)
				win_dirtylist_resize(l, o->size);
			if((count = o->count) > 0)
				memcpy(l->items, o->items, sizeof(struct win_dirtylist_item_t) * count);
			l->count = count;
		}
	}
}

void win_dirtylist_merge(struct win_dirtylist_t * l, struct win_dirtylist_t * o)
{
	int count;
	int i;

	if(l && o && ((count = o->count) > 0))
	{
		for(i = 0; i < count; i++)
			win_dirtylist_add(l, &o->items[i].region);
	}
}

void win_dirtylist_clear(struct win_dirtylist_t * l)
{
	if(l)
		l->count = 0;
}

static inline int __win_region_area_intersect(struct win_region_t * a, struct win_region_t * b)
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

static inline int __win_region_area_union(struct win_region_t * a, struct win_region_t * b)
{
	int ar = a->x + a->w;
	int ab = a->y + a->h;
	int br = b->x + b->w;
	int bb = b->y + b->h;
	int w = XMAX(ar, br) - XMIN(a->x, b->x);
	int h = XMAX(ab, bb) - XMIN(a->y, b->y);
	return w * h;
}

static inline int __win_dirtylist_area_rule1(struct win_dirtylist_t * l, int i, int j)
{
	struct win_dirtylist_item_t * p = &l->items[i];
	struct win_dirtylist_item_t * q = &l->items[j];

	if(__win_region_area_union(&p->region, &q->region) < (p->area + q->area))
		return __win_region_area_intersect(&p->region, &q->region);
	return 0;
}

static inline int __win_dirtylist_area_rule2(struct win_dirtylist_t * l, int i, int j)
{
	struct win_dirtylist_item_t * p = &l->items[i];
	struct win_dirtylist_item_t * q = &l->items[j];

	return __win_region_area_union(&p->region, &q->region) - (p->area + q->area);
}

static inline int __win_dirtylist_flush(struct win_dirtylist_t * l)
{
	int count = l->count;

	l->count = 0;
	for(int i = 0; i < count; i++)
	{
		struct win_dirtylist_item_t * p = &l->items[i];
		if(p->area > 0)
		{
			struct win_dirtylist_item_t * q = &l->items[l->count];
			if(q != p)
				memcpy(q, p, sizeof(struct win_dirtylist_item_t));
			l->count++;
		}
	}
	return l->count;
}

static inline int __win_dirtylist_optimize_rule1(struct win_dirtylist_t * l)
{
	int area_max = 0;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __win_dirtylist_area_rule1(l, i, j);
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
		struct win_dirtylist_item_t * p = &l->items[XMIN(best_i, best_j)];
		struct win_dirtylist_item_t * q = &l->items[XMAX(best_i, best_j)];
		win_region_union(&p->region, &p->region, &q->region);
		p->area = p->region.w * p->region.h;
		q->area = 0;
		__win_dirtylist_flush(l);
		return 1;
	}
	return 0;
}

static inline int __win_dirtylist_optimize_rule2(struct win_dirtylist_t * l)
{
	int area_min = INT_MAX;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __win_dirtylist_area_rule2(l, i, j);
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
		struct win_dirtylist_item_t * p = &l->items[XMIN(best_i, best_j)];
		struct win_dirtylist_item_t * q = &l->items[XMAX(best_i, best_j)];
		win_region_union(&p->region, &p->region, &q->region);
		p->area = p->region.w * p->region.h;
		q->area = 0;
		__win_dirtylist_flush(l);
		return 1;
	}
	return 0;
}

static inline void win_dirtylist_optimize(struct win_dirtylist_t * l, int n)
{
	if(l->count > 1)
	{
		while(__win_dirtylist_optimize_rule1(l));
		while(l->count > n)
			__win_dirtylist_optimize_rule2(l);
	}
}

void win_dirtylist_add(struct win_dirtylist_t * l, struct win_region_t * r)
{
	if(l && r)
	{
		if(l->size <= l->count)
			win_dirtylist_resize(l, l->size << 1);
		struct win_dirtylist_item_t * item = &l->items[l->count];
		item->region.x = r->x;
		item->region.y = r->y;
		item->region.w = r->w;
		item->region.h = r->h;
		item->area = r->w * r->h;
		l->count++;
		win_dirtylist_optimize(l, 3);
	}
}
