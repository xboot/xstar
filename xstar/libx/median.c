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
#include <libx/median.h>

struct median_filter_t * median_alloc(int length)
{
	struct median_filter_t * filter;

	if(length <= 0)
		return NULL;

	filter = xos_mem_malloc(sizeof(struct median_filter_t));
	if(!filter)
		return NULL;

	filter->buffer = xos_mem_malloc(sizeof(int) * length);
	filter->index = xos_mem_malloc(sizeof(int) * length);
	if(!filter->buffer || !filter->index)
	{
		if(filter->buffer)
			xos_mem_free(filter->buffer);
		if(filter->index)
			xos_mem_free(filter->index);
		xos_mem_free(filter);
		return NULL;
	}
	filter->length = length;
	filter->position = 0;
	filter->count = 0;

	return filter;
}

void median_free(struct median_filter_t * filter)
{
	if(filter)
	{
		if(filter->buffer)
			xos_mem_free(filter->buffer);
		if(filter->index)
			xos_mem_free(filter->index);
		xos_mem_free(filter);
	}
}

int median_update(struct median_filter_t * filter, int value)
{
	int pos = filter->position;
	int cnt = filter->count;
	int * idx;
	int cidx;
	int oidx;
	int oval;
	int result;

	if(cnt > 0)
	{
		if(cnt == filter->length)
		{
			oidx = 0;
			while(filter->index[oidx] != pos)
				++oidx;
			oval = filter->buffer[pos];
		}
		else
		{
			filter->index[pos] = pos;
			oidx = pos;
			oval = INT_MAX;
		}

		filter->buffer[pos] = value;
		idx = &filter->index[oidx];
		if(oval < value)
		{
			while(++oidx != cnt)
			{
				cidx = *(++idx);
				if(filter->buffer[cidx] < value)
				{
					*idx = *(idx - 1);
					*(idx - 1) = cidx;
				}
				else
				{
					break;
				}
			}
		}
		else if(oval > value)
		{
			while(oidx-- != 0)
			{
				cidx = *(--idx);
				if (filter->buffer[cidx] > value)
				{
					*idx = *(idx + 1);
					*(idx + 1) = cidx;
				}
				else
				{
					break;
				}
			}
		}
		result = filter->buffer[filter->index[cnt / 2]];
	}
	else
	{
		filter->buffer[0] = value;
		filter->index[0] = 0;
		filter->position = 0;
		filter->count = 0;
		result = value;
	}

	pos++;
	filter->position = (pos == filter->length) ? 0 : pos;
	if(cnt < filter->length)
		filter->count++;

	return result;
}

void median_clear(struct median_filter_t * filter)
{
	if(filter)
	{
		filter->position = 0;
		filter->count = 0;
	}
}
