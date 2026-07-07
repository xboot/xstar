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
#include <libx/biquad.h>

/*
 * biquadratic filter online designer.
 * https://arachnoid.com/BiQuadDesigner
 */
struct biquad_filter_t * biquad_alloc(enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain)
{
	struct biquad_filter_t * filter;

	filter = xos_mem_malloc(sizeof(struct biquad_filter_t));
	if(!filter)
		return NULL;

	biquad_init(filter, type, samplerate, frequency, quality, gain);
	biquad_clear(filter);

	return filter;
}

void biquad_free(struct biquad_filter_t * filter)
{
	if(filter)
		xos_mem_free(filter);
}

void biquad_init(struct biquad_filter_t * filter, enum biquad_filter_type_t type, float samplerate, float frequency, float quality, float gain)
{
	double omega = 2.0 * M_PI * frequency / samplerate;
	double cs = cos(omega);
	double sn = sin(omega);
	double a1pha = sn / (2.0 * quality);
	double A = exp(log(10.0) * gain / 40.0);
	double beta = sqrt(A + A);

	float b0 = 0, b1 = 0, b2 = 0;
	float a0 = 1, a1 = 0, a2 = 0;

	switch(type)
	{
	case BIQUAD_FILTER_TYPE_LOWPASS:
		b0 = (1.0 - cs) / 2.0;
		b1 = 1.0 - cs;
		b2 = (1.0 - cs) / 2.0;
		a0 = 1.0 + a1pha;
		a1 = -2.0 * cs;
		a2 = 1.0 - a1pha;
		break;

	case BIQUAD_FILTER_TYPE_HIGHPASS:
		b0 = (1.0 + cs) / 2.0;
		b1 = -(1.0 + cs);
		b2 = (1.0 + cs) / 2.0;
		a0 = 1.0 + a1pha;
		a1 = -2.0 * cs;
		a2 = 1.0 - a1pha;
        break;

	case BIQUAD_FILTER_TYPE_BANDPASS:
		b0 = a1pha;
		b1 = 0.0;
		b2 = -a1pha;
		a0 = 1.0 + a1pha;
		a1 = -2.0 * cs;
		a2 = 1.0 - a1pha;
		break;

	case BIQUAD_FILTER_TYPE_LOWSHELF:
		b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
		b1 = 2 * A * ((A - 1) - (A + 1) * cs);
		b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
		a0 = (A + 1) + (A - 1) * cs + beta * sn;
		a1 = -2 * ((A - 1) + (A + 1) * cs);
		a2 = (A + 1) + (A - 1) * cs - beta * sn;
		break;

	case BIQUAD_FILTER_TYPE_HIGHSHELF:
		b0 = A * ((A + 1.0) + (A - 1.0) * cs + beta * sn);
		b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
		b2 = A * ((A + 1.0) + (A - 1.0) * cs - beta * sn);
		a0 = (A + 1.0) - (A - 1.0) * cs + beta * sn;
		a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
		a2 = (A + 1.0) - (A - 1.0) * cs - beta * sn;
		break;

	case BIQUAD_FILTER_TYPE_PEAKING:
		b0 = 1.0 + a1pha * A;
		b1 = -2.0 * cs;
		b2 = 1.0 - a1pha * A;
		a0 = 1.0 + a1pha / A;
		a1 = -2.0 * cs;
		a2 = 1.0 - a1pha / A;
		break;

	case BIQUAD_FILTER_TYPE_NOTCH:
		b0 = 1.0;
		b1 = -2.0 * cs;
		b2 = 1.0;
		a0 = 1.0 + a1pha;
		a1 = -2.0 * cs;
		a2 = 1.0 - a1pha;
		break;

	case BIQUAD_FILTER_TYPE_ALLPASS:
		b0 = 1.0 - a1pha;
		b1 = -2.0 * cs;
		b2 = 1.0 + a1pha;
		a0 = 1.0 + a1pha;
		a1 = -2.0 * cs;
		a2 = 1.0 - a1pha;
		break;

	default:
		break;
	}

	filter->b0 = b0 / a0;
	filter->b1 = b1 / a0;
	filter->b2 = b2 / a0;
	filter->a1 = a1 / a0;
	filter->a2 = a2 / a0;
}

void biquad_clear(struct biquad_filter_t * filter)
{
	if(filter)
	{
		filter->xn1 = 0;
		filter->xn2 = 0;
		filter->yn1 = 0;
		filter->yn2 = 0;
	}
}

void biquad_process(struct biquad_filter_t * filter, float * input, float * output, int len)
{
	float b0 = filter->b0;
	float b1 = filter->b1;
	float b2 = filter->b2;
	float a1 = filter->a1;
	float a2 = filter->a2;

	float xn1 = filter->xn1;
	float xn2 = filter->xn2;
	float yn1 = filter->yn1;
	float yn2 = filter->yn2;

	for(int i = 0; i < len; i++)
	{
		float x = input[i];
		float y = b0 * x + b1 * xn1 + b2 * xn2 - a1 * yn1 - a2 * yn2;
		xn2 = xn1;
		xn1 = x;
		yn2 = yn1;
		yn1 = y;
		output[i] = y;
	}
	filter->xn1 = xn1;
	filter->xn2 = xn2;
	filter->yn1 = yn1;
	filter->yn2 = yn2;
}
