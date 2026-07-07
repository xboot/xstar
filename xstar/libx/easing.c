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

#include <libx/easing.h>

void bezier_init(struct bezier_t * b, float x1, float y1, float x2, float y2)
{
	b->cx = 3.0 * x1;
	b->bx = 3.0 * (x2 - x1) - b->cx;
	b->ax = 1.0 - b->cx - b->bx;
	b->cy = 3.0 * y1;
	b->by = 3.0 * (y2 - y1) - b->cy;
	b->ay = 1.0 - b->cy - b->by;
}

static inline float sample_curve_x(struct bezier_t * b, float t)
{
	return ((b->ax * t + b->bx) * t + b->cx) * t;
}

static inline float sample_curve_y(struct bezier_t * b, float t)
{
	return ((b->ay * t + b->by) * t + b->cy) * t;
}

static inline float sample_curve_derivative(struct bezier_t * b, float t)
{
	return (3 * b->ax * t + 2 * b->bx) * t + b->cx;
}

static inline float solve_curve_x(struct bezier_t * b, float t)
{
	float t1 = t;
	for(int i = 0; i < 8; i++)
	{
		float err = sample_curve_x(b, t1) - t;
		if(fabsf(err) < 1e-6)
			return t1;
		float dx = sample_curve_derivative(b, t1);
		if(fabsf(dx) < 1e-6)
			break;
		t1 -= err / dx;
	}
	float lower = 0.0;
	float upper = 1.0;
	float t2 = t;
	while(lower < upper)
	{
		float v = sample_curve_x(b, t2);
		if(fabsf(v - t) < 1e-6)
			return t2;
		if(t > v)
			lower = t2;
		else
			upper = t2;
		t2 = (lower + upper) / 2;
	}
	return t2;
}

float bezier_calc(struct bezier_t * b, float t)
{
	if(t <= 0.0)
		return 0.0;
	else if(t >= 1.0)
		return 1.0;
	else
		return sample_curve_y(b, solve_curve_x(b, t));
}

void easing_init(struct easing_t * e, float start, float stop, float duration, float x1, float y1, float x2, float y2)
{
	bezier_init(&e->bezier, x1, y1, x2, y2);
	e->start = start;
	e->stop = stop;
	e->duration = duration;
	e->acc = 0.0;
}

float easing_calc(struct easing_t * e, float t)
{
	return (e->stop - e->start) * bezier_calc(&e->bezier, t / e->duration) + e->start;
}

float easing_step(struct easing_t * e, float dt)
{
	if(e->acc < e->duration)
		e->acc += dt;
	return easing_calc(e, e->acc);
}

int easing_finished(struct easing_t * e)
{
	return (e->acc < e->duration) ? 0 : 1;
}
