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

#include <xos/xos.h>
#include <libx/mean.h>

struct mean_filter_t * mean_alloc(int length)
{
	struct mean_filter_t * filter;
	int i;

	if(length <= 0)
		return NULL;

	filter = xos_mem_malloc(sizeof(struct mean_filter_t));
	if(!filter)
		return NULL;

	filter->buffer = xos_mem_malloc(sizeof(int) * length);
	if(!filter->buffer)
	{
		xos_mem_free(filter);
		return NULL;
	}
	for(i = 0; i < length; i++)
		filter->buffer[i] = 0;
	filter->length = length;
	filter->index = 0;
	filter->count = 0;
	filter->sum = 0;

	return filter;
}

void mean_free(struct mean_filter_t * filter)
{
	if(filter)
	{
		if(filter->buffer)
			xos_mem_free(filter->buffer);
		xos_mem_free(filter);
	}
}

int mean_update(struct mean_filter_t * filter, int value)
{
	filter->sum -= filter->buffer[filter->index];
	filter->sum += value;
	filter->buffer[filter->index] = value;
	filter->index = (filter->index + 1) % filter->length;

	if(filter->count < filter->length)
		filter->count++;
	return filter->sum / filter->count;
}

void mean_clear(struct mean_filter_t * filter)
{
	int i;

	if(filter)
	{
		for(i = 0; i < filter->length; i++)
			filter->buffer[i] = 0;
		filter->index = 0;
		filter->count = 0;
		filter->sum = 0;
	}
}
