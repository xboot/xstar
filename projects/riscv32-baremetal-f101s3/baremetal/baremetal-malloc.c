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

#include <baremetal/baremetal.h>

static void * __heap_pool = NULL;
static struct mutex_t __heap_lock;

static int heap_pool_ready(void)
{
	extern unsigned char __heap_start[];
	extern unsigned char __heap_end[];
	if(!__heap_pool)
	{
		xos_mutex_init(&__heap_lock);
		__heap_pool = mm_create((void *)__heap_start, (size_t)(__heap_end - __heap_start));
	}
	return __heap_pool ? 1 : 0;
}

void * baremetal_malloc(size_t size)
{
	void * m;

	if(heap_pool_ready())
	{
		xos_mutex_lock(&__heap_lock);
		m = mm_malloc(__heap_pool, size);
		xos_mutex_unlock(&__heap_lock);
		return m;
	}
	return NULL;
}

void * baremetal_memalign(size_t align, size_t size)
{
	void * m;

	if(heap_pool_ready())
	{
		xos_mutex_lock(&__heap_lock);
		m = mm_memalign(__heap_pool, align, size);
		xos_mutex_unlock(&__heap_lock);
		return m;
	}
	return NULL;
}

void * baremetal_realloc(void * ptr, size_t size)
{
	void * m;

	if(heap_pool_ready())
	{
		xos_mutex_lock(&__heap_lock);
		m = mm_realloc(__heap_pool, ptr, size);
		xos_mutex_unlock(&__heap_lock);
		return m;
	}
	return NULL;
}

void * baremetal_calloc(size_t nmemb, size_t size)
{
	void * m;

	if((m = baremetal_malloc(nmemb * size)))
		xos_memset(m, 0, nmemb * size);
	return m;
}

void baremetal_free(void * ptr)
{
	if(heap_pool_ready())
	{
		xos_mutex_lock(&__heap_lock);
		mm_free(__heap_pool, ptr);
		xos_mutex_unlock(&__heap_lock);
	}
}

void baremetal_meminfo(size_t * mused, size_t * mfree)
{
	if(heap_pool_ready())
	{
		if(mused && mfree)
			mm_info(mm_get(__heap_pool), mused, mfree);
	}
}
