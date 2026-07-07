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
#include <libx/complex.h>

void complex_add(struct complex_t * z, struct complex_t * a, struct complex_t * b)
{
	z->real = a->real + b->real;
	z->imag = a->imag + b->imag;
}

void complex_sub(struct complex_t * z, struct complex_t * a, struct complex_t * b)
{
	z->real = a->real - b->real;
	z->imag = a->imag - b->imag;
}

void complex_mul(struct complex_t * z, struct complex_t * a, struct complex_t * b)
{
	struct complex_t t;

	t.real = a->real * b->real - a->imag * b->imag;
	t.imag = a->real * b->imag + a->imag * b->real;
	z->real = t.real;
	z->imag = t.imag;
}

void complex_div(struct complex_t * z, struct complex_t * a, struct complex_t * b)
{
	struct complex_t t;

	if(fabsf(b->real) >= fabsf(b->imag))
	{
		float r = b->imag / b->real;
		float den = b->real + r * b->imag;
		t.real = (a->real + r * a->imag) / den;
		t.imag = (a->imag - r * a->real) / den;
	}
	else
	{
		float r = b->real / b->imag;
		float den = b->imag + r * b->real;
		t.real = (a->real * r + a->imag) / den;
		t.imag = (a->imag * r - a->real) / den;
	}
	z->real = t.real;
	z->imag = t.imag;
}

void complex_neg(struct complex_t * z, struct complex_t * a)
{
	z->real = -a->real;
	z->imag = -a->imag;
}

void complex_conj(struct complex_t * z, struct complex_t * a)
{
	z->real = a->real;
	z->imag = -a->imag;
}

void complex_proj(struct complex_t * z, struct complex_t * a)
{
	if(isinf(a->real) || isinf(a->imag))
	{
		union { float f; uint32_t i; } ux = { 0.0f }, uy = { a->imag };
		ux.i &= 0x7fffffff;
		ux.i |= uy.i & 0x80000000;
		z->real = INFINITY;
		z->imag = ux.f;
	}
	else
	{
		z->real = a->real;
		z->imag = a->imag;
	}
}

void complex_expj(struct complex_t * z, float phi)
{
	z->real = cosf(phi);
	z->imag = sinf(phi);
}

void complex_normalize(struct complex_t * z, struct complex_t * a)
{
	float av = hypotf(a->real, a->imag);
	z->real = a->real / av;
	z->imag = -a->imag / av;
}

float complex_abs(struct complex_t * z)
{
	return hypotf(z->real, z->imag);
}

float complex_arg(struct complex_t * z)
{
	return atan2f(z->imag, z->real);
}

