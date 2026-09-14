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

#include <freertos/freertos.h>

static void * __dma_pool = NULL;
static struct mutex_t __dma_lock;

static int dma_pool_ready(void)
{
	extern unsigned char __dma_start[];
	extern unsigned char __dma_end[];
	if(!__dma_pool)
	{
		xos_mutex_init(&__dma_lock);
		__dma_pool = mm_create((void *)__dma_start, (size_t)(__dma_end - __dma_start));
	}
	return __dma_pool ? 1 : 0;
}

void * freertos_dma_alloc_coherent(unsigned long size)
{
	void * m;

	if(dma_pool_ready())
	{
		xos_mutex_lock(&__dma_lock);
		m = mm_memalign(__dma_pool, 4096, size);
		xos_mutex_unlock(&__dma_lock);
		return m;
	}
	return NULL;
}

void freertos_dma_free_coherent(void * addr)
{
	if(dma_pool_ready())
	{
		xos_mutex_lock(&__dma_lock);
		mm_free(__dma_pool, addr);
		xos_mutex_unlock(&__dma_lock);
	}
}

void * freertos_dma_alloc_noncoherent(unsigned long size)
{
	return freertos_mem_memalign(4096, size);
}

void freertos_dma_free_noncoherent(void * addr)
{
	freertos_mem_free(addr);
}

void freertos_dma_sync(void * addr, unsigned long size, int flag)
{
	unsigned long start = (unsigned long)addr;
	unsigned long stop = start + size;

	if(flag == DMA_SYNC_TO_DEVICE)
	{
		dcache_wb_range(start, stop);
	}
	else if(flag == DMA_SYNC_FROM_DEVICE)
	{
		dcache_inv_range(start, stop);
	}
	else
	{
		dcache_wb_range(start, stop);
		dcache_inv_range(start, stop);
	}
}
