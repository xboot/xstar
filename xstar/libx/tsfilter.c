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
#include <libx/tsfilter.h>

struct tsfilter_t * tsfilter_alloc(int ml, int nl)
{
	struct tsfilter_t * filter;

	if(ml <= 0 || nl <= 0)
		return NULL;

	filter = xos_mem_malloc(sizeof(struct tsfilter_t));
	if(!filter)
		return NULL;

	filter->mx = median_alloc(ml);
	filter->my = median_alloc(ml);
	filter->nx = mean_alloc(nl);
	filter->ny = mean_alloc(nl);
	filter->cal[0] = 1;
	filter->cal[1] = 0;
	filter->cal[2] = 0;
	filter->cal[3] = 0;
	filter->cal[4] = 1;
	filter->cal[5] = 0;
	filter->cal[6] = 1;

	if(!filter->mx || !filter->my || !filter->nx || !filter->ny)
	{
		if(filter->mx)
			median_free(filter->mx);
		if(filter->my)
			median_free(filter->my);
		if(filter->nx)
			mean_free(filter->nx);
		if(filter->ny)
			mean_free(filter->ny);
	}
	return filter;
}

void tsfilter_free(struct tsfilter_t * filter)
{
	if(filter)
	{
		if(filter->mx)
			median_free(filter->mx);
		if(filter->my)
			median_free(filter->my);
		if(filter->nx)
			mean_free(filter->nx);
		if(filter->ny)
			mean_free(filter->ny);
		xos_mem_free(filter);
	}
}

void tsfilter_setcal(struct tsfilter_t * filter, int * cal)
{
	if(filter)
	{
		filter->cal[0] = cal[0];
		filter->cal[1] = cal[1];
		filter->cal[2] = cal[2];
		filter->cal[3] = cal[3];
		filter->cal[4] = cal[4];
		filter->cal[5] = cal[5];
		filter->cal[6] = cal[6];
	}
}

void tsfilter_update(struct tsfilter_t * filter, int * x, int * y)
{
	int tx, ty;

	tx = median_update(filter->mx, *x);
	ty = median_update(filter->my, *y);
	tx = mean_update(filter->nx, tx);
	ty = mean_update(filter->ny, ty);
	*x = (filter->cal[2] + filter->cal[0] * tx + filter->cal[1] * ty) / filter->cal[6];
	*y = (filter->cal[5] + filter->cal[3] * tx + filter->cal[4] * ty) / filter->cal[6];
}

void tsfilter_clear(struct tsfilter_t * filter)
{
	if(filter)
	{
		if(filter->mx)
			median_clear(filter->mx);
		if(filter->my)
			median_clear(filter->my);
		if(filter->nx)
			mean_clear(filter->nx);
		if(filter->ny)
			mean_clear(filter->ny);
	}
}
