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

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM

#include <xstar.h>
#include <lvgl/src/stdlib/lv_mem.h>

void lv_mem_init(void)
{
	return;
}

void lv_mem_deinit(void)
{
	return;
}

lv_mem_pool_t lv_mem_add_pool(void * mem, size_t bytes)
{
	return NULL;
}

void lv_mem_remove_pool(lv_mem_pool_t pool)
{
	return;
}

void * lv_malloc_core(size_t size)
{
	return xos_mem_malloc(size);
}

void * lv_realloc_core(void * p, size_t new_size)
{
	return xos_mem_realloc(p, new_size);
}

void lv_free_core(void * p)
{
	xos_mem_free(p);
}

void lv_mem_monitor_core(lv_mem_monitor_t * mon_p)
{
	return;
}

lv_result_t lv_mem_test_core(void)
{
	return LV_RESULT_OK;
}

#endif
