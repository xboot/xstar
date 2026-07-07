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

#include <kernel/graphic/matrix2d.h>

void matrix2d_init(struct matrix2d_t * m,
		float a, float b,
		float c, float d,
		float tx, float ty)
{
	m->a = a; m->b = b;
	m->c = c; m->d = d;
	m->tx = tx; m->ty = ty;
}

void matrix2d_init_identity(struct matrix2d_t * m)
{
	m->a = 1; m->b = 0;
	m->c = 0; m->d = 1;
	m->tx = 0; m->ty = 0;
}

void matrix2d_init_translate(struct matrix2d_t * m,
		float tx, float ty)
{
	m->a = 1;  m->b = 0;
	m->c = 0;  m->d = 1;
	m->tx = tx; m->ty = ty;
}

void matrix2d_init_scale(struct matrix2d_t * m,
		float sx, float sy)
{
	m->a = sx; m->b = 0;
	m->c = 0;  m->d = sy;
	m->tx = 0;  m->ty = 0;
}

void matrix2d_init_rotate(struct matrix2d_t * m, float r)
{
	float s = sinf(r);
	float c = cosf(r);

	m->a = c;  m->b = s;
	m->c = -s; m->d = c;
	m->tx = 0;  m->ty = 0;
}

/*
 * | [m->a]  [m->b]  [0] |   | [m1->a]  [m1->b]  [0] |   | [m2->a]  [m2->b]  [0] |
 * | [m->c]  [m->d]  [0] | = | [m1->c]  [m1->d]  [0] | x | [m2->c]  [m2->d]  [0] |
 * | [m->tx] [m->ty] [1] |   | [m1->tx] [m1->ty] [1] |   | [m2->tx] [m2->ty] [1] |
 */
void matrix2d_multiply(struct matrix2d_t * m,
		struct matrix2d_t * m1,
		struct matrix2d_t * m2)
{
	struct matrix2d_t t;

	t.a = m1->a * m2->a;
	t.b = 0.0;
	t.c = 0.0;
	t.d = m1->d * m2->d;
	t.tx = m1->tx * m2->a + m2->tx;
	t.ty = m1->ty * m2->d + m2->ty;

	if(m1->b != 0.0 || m1->c != 0.0 || m2->b != 0.0 || m2->c != 0.0)
	{
		t.a += m1->b * m2->c;
		t.b += m1->a * m2->b + m1->b * m2->d;
		t.c += m1->c * m2->a + m1->d * m2->c;
		t.d += m1->c * m2->b;
		t.tx += m1->ty * m2->c;
		t.ty += m1->tx * m2->b;
	}
	xos_memcpy(m, &t, sizeof(struct matrix2d_t));
}

void matrix2d_invert(struct matrix2d_t * m)
{
	float a, b, c, d, tx, ty;
	float det;

	if((m->c == 0.0) && (m->b == 0.0))
	{
		m->tx = -m->tx;
		m->ty = -m->ty;
		if(m->a != 1.0)
		{
			if(m->a == 0.0)
				return;
			m->a = 1.0 / m->a;
			m->tx *= m->a;
		}
		if(m->d != 1.0)
		{
			if(m->d == 0.0)
				return;
			m->d = 1.0 / m->d;
			m->ty *= m->d;
		}
	}
	else
	{
		det = m->a * m->d - m->b * m->c;
		if(det != 0.0)
		{
			a  = m->a;
			b  = m->b;
			c  = m->c;
			d  = m->d;
			tx = m->tx;
			ty = m->ty;
			m->a = d / det;
			m->b = -b / det;
			m->c = -c / det;
			m->d = a / det;
			m->tx = (c * ty - d * tx) / det;
			m->ty = (b * tx - a * ty) / det;
		}
	}
}

void matrix2d_translate(struct matrix2d_t * m, float tx, float ty)
{
	m->tx += m->a * tx + m->c * ty;
	m->ty += m->b * tx + m->d * ty;
}

void matrix2d_scale(struct matrix2d_t * m, float sx, float sy)
{
	m->a *= sx;
	m->b *= sx;
	m->c *= sy;
	m->d *= sy;
}

void matrix2d_rotate(struct matrix2d_t * m, float r)
{
	float s = sinf(r);
	float c = cosf(r);
	float ca = c * m->a;
	float cb = c * m->b;
	float cc = c * m->c;
	float cd = c * m->d;
	float sa = s * m->a;
	float sb = s * m->b;
	float sc = s * m->c;
	float sd = s * m->d;

	m->a = ca + sc;
	m->b = cb + sd;
	m->c = cc - sa;
	m->d = cd - sb;
}

void matrix2d_transform_distance(struct matrix2d_t * m, float * dx, float * dy)
{
	float nx = m->a * (*dx) + m->c * (*dy);
	float ny = m->b * (*dx) + m->d * (*dy);
	*dx = nx;
	*dy = ny;
}

void matrix2d_transform_point(struct matrix2d_t * m, float * x, float * y)
{
	float nx = m->a * (*x) + m->c * (*y) + m->tx;
	float ny = m->b * (*x) + m->d * (*y) + m->ty;
	*x = nx;
	*y = ny;
}

void matrix2d_transform_bounds(struct matrix2d_t * m, float * x1, float * y1, float * x2, float * y2)
{
	float qx[4], qy[4];
	float minx, maxx;
	float miny, maxy;
	int i;

	if((m->c == 0.0) && (m->b == 0.0))
	{
		if(m->a != 1.0)
		{
			qx[0] = (*x1) * m->a;
			qx[1] = (*x2) * m->a;
			if(qx[0] < qx[1])
			{
				*x1 = qx[0];
				*x2 = qx[1];
			}
			else
			{
				*x1 = qx[1];
				*x2 = qx[0];
			}
		}
		if(m->tx != 0.0)
		{
			*x1 += m->tx;
			*x2 += m->tx;
		}
		if(m->d != 1.0)
		{
			qy[0] = (*y1) * m->d;
			qy[1] = (*y2) * m->d;
			if(qy[0] < qy[1])
			{
				*y1 = qy[0];
				*y2 = qy[1];
			}
			else
			{
				*y1 = qy[1];
				*y2 = qy[0];
			}
		}
		if(m->ty != 0.0)
		{
			*y1 += m->ty;
			*y2 += m->ty;
		}
		return;
	}
	else
	{
		qx[0] = *x1;
		qy[0] = *y1;
		matrix2d_transform_point(m, &qx[0], &qy[0]);

		qx[1] = *x2;
		qy[1] = *y1;
		matrix2d_transform_point(m, &qx[1], &qy[1]);

		qx[2] = *x1;
		qy[2] = *y2;
		matrix2d_transform_point(m, &qx[2], &qy[2]);

		qx[3] = *x2;
		qy[3] = *y2;
		matrix2d_transform_point(m, &qx[3], &qy[3]);

		minx = maxx = qx[0];
		miny = maxy = qy[0];

		for(i = 1; i < 4; i++)
		{
			if(qx[i] < minx)
				minx = qx[i];
			if(qx[i] > maxx)
				maxx = qx[i];
			if(qy[i] < miny)
				miny = qy[i];
			if(qy[i] > maxy)
				maxy = qy[i];
		}
		*x1 = minx;
		*y1 = miny;
		*x2 = maxx;
		*y2 = maxy;
	}
}

void matrix2d_transform_region(struct matrix2d_t * m, float w, float h, struct region_t * region)
{
	float aw = m->a * w;
	float bw = m->b * w;
	float ch = m->c * h;
	float dh = m->d * h;
	float l = m->tx;
	float t = m->ty;
	float r = m->tx;
	float b = m->ty;
	float nx, ny;

	nx = aw + m->tx;
	ny = bw + m->ty;
	if(l > nx)
		l = nx;
	if(t > ny)
		t = ny;
	if(r < nx)
		r = nx;
	if(b < ny)
		b = ny;

	nx = aw + ch + m->tx;
	ny = bw + dh + m->ty;
	if(l > nx)
		l = nx;
	if(t > ny)
		t = ny;
	if(r < nx)
		r = nx;
	if(b < ny)
		b = ny;

	nx = ch + m->tx;
	ny = dh + m->ty;
	if(l > nx)
		l = nx;
	if(t > ny)
		t = ny;
	if(r < nx)
		r = nx;
	if(b < ny)
		b = ny;

	region_init(region, XFLOOR(l), XFLOOR(t), XCEIL(r - l), XCEIL(b - t));
}
