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

void * freertos_mem_malloc(size_t size)
{
	return pvPortMalloc(size);
}

void * freertos_mem_memalign(size_t align, size_t size)
{
	return pvPortMalloc(size);
}

void * freertos_mem_realloc(void * ptr, size_t size)
{
	extern void * pvPortRealloc(void *, size_t);
	return pvPortRealloc(ptr, size);
}

void * freertos_mem_calloc(size_t nmemb, size_t size)
{
	return pvPortCalloc(nmemb, size);
}

void freertos_mem_free(void * ptr)
{
	vPortFree(ptr);
}

void freertos_mem_meminfo(size_t * mused, size_t * mfree)
{
	extern unsigned char __heap_start[];
	extern unsigned char __heap_end[];
	if(mused)
		*mused = (size_t)(__heap_end - __heap_start) - xPortGetFreeHeapSize();
	if(mfree)
		*mfree = xPortGetFreeHeapSize();
}
