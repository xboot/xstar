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
#include <libx/ewma.h>

struct ewma_filter_t * ewma_alloc(float weight)
{
	struct ewma_filter_t * filter;

	filter = xos_mem_malloc(sizeof(struct ewma_filter_t));
	if(!filter)
		return NULL;

	filter->weight = weight;
	filter->last = NAN;

	return filter;
}

void ewma_free(struct ewma_filter_t * filter)
{
	if(filter)
		xos_mem_free(filter);
}

float ewma_update(struct ewma_filter_t * filter, float value)
{
	if(isnan(filter->last))
		filter->last = value;
	else
		filter->last = filter->weight * value + (1 - filter->weight) * filter->last;
	return filter->last;
}

void ewma_clear(struct ewma_filter_t * filter)
{
	if(filter)
		filter->last = NAN;
}
