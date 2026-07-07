#include <linux/linux.h>

struct linux_dirtylist_t * linux_dirtylist_alloc(unsigned int size)
{
	struct linux_dirtylist_t * l;
	struct linux_dirtylist_item_t * items;

	if(size < 16)
		size = 16;

	items = malloc(size * sizeof(struct linux_dirtylist_item_t));
	if(!items)
		return NULL;

	l = malloc(sizeof(struct linux_dirtylist_t));
	if(!l)
	{
		free(items);
		return NULL;
	}

	l->items = items;
	l->size = size;
	l->count = 0;
	return l;
}

void linux_dirtylist_free(struct linux_dirtylist_t * l)
{
	if(l)
	{
		free(l->items);
		free(l);
	}
}

static inline void linux_dirtylist_resize(struct linux_dirtylist_t * l, unsigned int size)
{
	if(l && (l->size != size))
	{
		if(size < 16)
			size = 16;
		l->size = size;
		l->items = realloc(l->items, l->size * sizeof(struct linux_dirtylist_item_t));
	}
}

void linux_dirtylist_clone(struct linux_dirtylist_t * l, struct linux_dirtylist_t * o)
{
	int count;

	if(l)
	{
		if(!o)
			l->count = 0;
		else
		{
			if(l->size < o->size)
				linux_dirtylist_resize(l, o->size);
			if((count = o->count) > 0)
				memcpy(l->items, o->items, sizeof(struct linux_dirtylist_item_t) * count);
			l->count = count;
		}
	}
}

void linux_dirtylist_merge(struct linux_dirtylist_t * l, struct linux_dirtylist_t * o)
{
	int count;
	int i;

	if(l && o && ((count = o->count) > 0))
	{
		for(i = 0; i < count; i++)
			linux_dirtylist_add(l, &o->items[i].region);
	}
}

void linux_dirtylist_clear(struct linux_dirtylist_t * l)
{
	if(l)
		l->count = 0;
}

static inline int __linux_region_area_intersect(struct linux_region_t * a, struct linux_region_t * b)
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

static inline int __linux_region_area_union(struct linux_region_t * a, struct linux_region_t * b)
{
	int ar = a->x + a->w;
	int ab = a->y + a->h;
	int br = b->x + b->w;
	int bb = b->y + b->h;
	int w = XMAX(ar, br) - XMIN(a->x, b->x);
	int h = XMAX(ab, bb) - XMIN(a->y, b->y);
	return w * h;
}

static inline int __linux_dirtylist_area_rule1(struct linux_dirtylist_t * l, int i, int j)
{
	struct linux_dirtylist_item_t * p = &l->items[i];
	struct linux_dirtylist_item_t * q = &l->items[j];

	if(__linux_region_area_union(&p->region, &q->region) < (p->area + q->area))
		return __linux_region_area_intersect(&p->region, &q->region);
	return 0;
}

static inline int __linux_dirtylist_area_rule2(struct linux_dirtylist_t * l, int i, int j)
{
	struct linux_dirtylist_item_t * p = &l->items[i];
	struct linux_dirtylist_item_t * q = &l->items[j];

	return __linux_region_area_union(&p->region, &q->region) - (p->area + q->area);
}

static inline int __linux_dirtylist_flush(struct linux_dirtylist_t * l)
{
	int count = l->count;

	l->count = 0;
	for(int i = 0; i < count; i++)
	{
		struct linux_dirtylist_item_t * p = &l->items[i];
		if(p->area > 0)
		{
			struct linux_dirtylist_item_t * q = &l->items[l->count];
			if(q != p)
				memcpy(q, p, sizeof(struct linux_dirtylist_item_t));
			l->count++;
		}
	}
	return l->count;
}

static inline int __linux_dirtylist_optimize_rule1(struct linux_dirtylist_t * l)
{
	int area_max = 0;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __linux_dirtylist_area_rule1(l, i, j);
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
		struct linux_dirtylist_item_t * p = &l->items[XMIN(best_i, best_j)];
		struct linux_dirtylist_item_t * q = &l->items[XMAX(best_i, best_j)];
		linux_region_union(&p->region, &p->region, &q->region);
		p->area = p->region.w * p->region.h;
		q->area = 0;
		__linux_dirtylist_flush(l);
		return 1;
	}
	return 0;
}

static inline int __linux_dirtylist_optimize_rule2(struct linux_dirtylist_t * l)
{
	int area_min = INT_MAX;
	int best_i = 0, best_j = 0;

	for(int i = 0; i < l->count; i++)
	{
		for(int j = 0; j < l->count; j++)
		{
			if(i != j)
			{
				int area = __linux_dirtylist_area_rule2(l, i, j);
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
		struct linux_dirtylist_item_t * p = &l->items[XMIN(best_i, best_j)];
		struct linux_dirtylist_item_t * q = &l->items[XMAX(best_i, best_j)];
		linux_region_union(&p->region, &p->region, &q->region);
		p->area = p->region.w * p->region.h;
		q->area = 0;
		__linux_dirtylist_flush(l);
		return 1;
	}
	return 0;
}

static inline void linux_dirtylist_optimize(struct linux_dirtylist_t * l, int n)
{
	if(l->count > 1)
	{
		while(__linux_dirtylist_optimize_rule1(l));
		while(l->count > n)
			__linux_dirtylist_optimize_rule2(l);
	}
}

void linux_dirtylist_add(struct linux_dirtylist_t * l, struct linux_region_t * r)
{
	if(l && r)
	{
		if(l->size <= l->count)
			linux_dirtylist_resize(l, l->size << 1);
		struct linux_dirtylist_item_t * item = &l->items[l->count];
		item->region.x = r->x;
		item->region.y = r->y;
		item->region.w = r->w;
		item->region.h = r->h;
		item->area = r->w * r->h;
		l->count++;
		linux_dirtylist_optimize(l, 3);
	}
}
