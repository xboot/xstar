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
#include <libx/kalman.h>

struct kalman_filter_t * kalman_alloc(float a, float h, float q, float r)
{
	struct kalman_filter_t * filter;

	filter = xos_mem_malloc(sizeof(struct kalman_filter_t));
	if(!filter)
		return NULL;

	filter->a = a;
	filter->h = h;
	filter->q = q;
	filter->r = r;
	filter->x = NAN;
	filter->p = q;
	filter->k = 1;
	filter->a2 = a * a;
	filter->h2 = h * h;

	return filter;
}

void kalman_free(struct kalman_filter_t * filter)
{
	if(filter)
		xos_mem_free(filter);
}

float kalman_update(struct kalman_filter_t * filter, float value)
{
	if(isnan(filter->x))
		filter->x = value;
	filter->x = filter->a * filter->x;
	filter->p = filter->a2 * filter->p + filter->q;
	filter->k = filter->p * filter->h / (filter->h2 * filter->p + filter->r);
	filter->x = filter->x + filter->k * (value - filter->h * filter->x);
	filter->p = (1 - filter->k * filter->h) * filter->p;

	return filter->h * filter->x;
}

void kalman_clear(struct kalman_filter_t * filter)
{
	if(filter)
		filter->x = NAN;
}
