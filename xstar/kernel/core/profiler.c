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

#include <kernel/core/logger.h>
#include <kernel/core/profiler.h>

static void hmap_entry_callback(struct hmap_t * m, struct hmap_entry_t * e)
{
	if(e && e->value)
		xos_mem_free(e->value);
}

static struct hmap_t * profiler_context_get(void)
{
	static struct hmap_t * ctx = NULL;

	if(!ctx)
		ctx = hmap_alloc(0, hmap_entry_callback);
	return ctx;
}

struct profiler_t * profiler_search(const char * name)
{
	struct hmap_t * m = profiler_context_get();
	struct profiler_t * p = NULL;

	if(m && name)
	{
		p = hmap_search(m, name);
		if(!p)
		{
			p = xos_mem_malloc(sizeof(struct profiler_t));
			if(p)
			{
				p->begin = 0;
				p->end = 0;
				p->elapsed = 0;
				p->count = 0;
				hmap_add(m, name, p);
			}
		}
	}
	return p;
}

void profiler_foreach(void (*cb)(const char * name, uint64_t count, uint64_t time))
{
	struct hmap_t * m = profiler_context_get();
	struct hmap_entry_t * e;
	struct profiler_t * p;

	if(m)
	{
		hmap_sort(m);
		hmap_for_each_entry(e, m)
		{
			p = (struct profiler_t *)e->value;
			if(cb)
				cb(e->key, p->count, (p->count > 0) ? (double)p->elapsed / (double)p->count : 0);
		}
	}
}

void profiler_clear(void)
{
	struct hmap_t * m = profiler_context_get();

	if(m)
		hmap_clear(m);
}
