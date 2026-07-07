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
#include <libx/fft.h>

static inline unsigned int bitswap(unsigned int x, int size_log2)
{
	int ret = 0;

	for(int i = 0; i < size_log2; i++)
		ret |= ((x >> i) & 1) << (size_log2 - i - 1);
	return ret;
}

static inline void build_bitinverse(unsigned int * bitinverse, int size_log2)
{
	int size = 1 << size_log2;

	for(int i = 0; i < size; i++)
		bitinverse[i] = bitswap(i, size_log2);
}

static inline void build_phase_lut(struct complex_t * out, int size)
{
	out += size;
	for(int i = -size; i <= size; i++)
	{
		double phase = (M_PI * i) / size;
		out[i].real = (float)cos(phase);
		out[i].imag = (float)sin(phase);
	}
}

static inline void interleave_complex(unsigned int * bitinverse, struct complex_t * out, struct complex_t * in, int samples)
{
	for(int i = 0; i < samples; i++, in++)
		out[bitinverse[i]] = *in;
}

static inline void interleave_float(unsigned int * bitinverse, struct complex_t * out, float * in, int samples)
{
	for(int i = 0; i < samples; i++, in++)
	{
		unsigned int inv_i = bitinverse[i];
		out[inv_i].real = *in;
		out[inv_i].imag = 0.0f;
	}
}

static inline void resolve_float(float * out, struct complex_t * in, int samples, float gain)
{
	for(int i = 0; i < samples; i++, in++, out++)
		*out = gain * in->real;
}

static inline void butterfly(struct complex_t * a, struct complex_t * b, struct complex_t * c)
{
	struct complex_t t;

	t.real = b->real * c->real - b->imag * c->imag;
	t.imag = b->real * c->imag + b->imag * c->real;
	b->real = a->real - t.real;
	b->imag = a->imag - t.imag;
	a->real += t.real;
	a->imag += t.imag;
}

static inline void butterflies(struct complex_t * butterfly_buf, struct complex_t * lut, int phase_dir, int step_size, int samples)
{
	for(int i = 0; i < samples; i += step_size << 1)
	{
		int phase_step = (int)samples * phase_dir / (int)step_size;
		for(int j = i; j < i + step_size; j++)
			butterfly(&butterfly_buf[j], &butterfly_buf[j + step_size], &lut[phase_step * (int)(j - i)]);
	}
}

struct fft_t * fft_new(int size_log2)
{
	struct fft_t * fft = (struct fft_t *)xos_mem_calloc(1, sizeof(struct fft_t));
	if(!fft)
		return NULL;

	int size = 1 << size_log2;
	fft->interleave = (struct complex_t *)xos_mem_calloc(size, sizeof(struct complex_t));
	fft->bitinverse = (unsigned int *)xos_mem_calloc(size, sizeof(unsigned int));
	fft->lut = (struct complex_t *)xos_mem_calloc(2 * size + 1, sizeof(struct complex_t));
	if(!fft->interleave || !fft->bitinverse || !fft->lut)
	{
		fft_free(fft);
		return NULL;
	}

	fft->size = size;
	build_bitinverse(fft->bitinverse, size_log2);
	build_phase_lut(fft->lut, size);

	return fft;
}

void fft_free(struct fft_t * fft)
{
	if(fft)
	{
		if(fft->interleave)
			xos_mem_free(fft->interleave);
		if(fft->bitinverse)
			xos_mem_free(fft->bitinverse);
		if(fft->lut)
			xos_mem_free(fft->lut);
		xos_mem_free(fft);
	}
}

void fft_process_forward_complex(struct fft_t * fft, struct complex_t * out, struct complex_t * in)
{
	unsigned int step_size;
	unsigned int samples = fft->size;

	interleave_complex(fft->bitinverse, out, in, samples);
	for(step_size = 1; step_size < samples; step_size <<= 1)
		butterflies(out, fft->lut + samples, -1, step_size, samples);
}

void fft_process_forward(struct fft_t * fft, struct complex_t * out, float * in)
{
	unsigned int step_size;
	unsigned int samples = fft->size;

	interleave_float(fft->bitinverse, out, in, samples);
	for(step_size = 1; step_size < samples; step_size <<= 1)
		butterflies(out, fft->lut + samples, -1, step_size, samples);
}

void fft_process_inverse(struct fft_t * fft, float * out, struct complex_t * in)
{
	unsigned int step_size;
	unsigned int samples = fft->size;

	interleave_complex(fft->bitinverse, fft->interleave, in, samples);
	for(step_size = 1; step_size < samples; step_size <<= 1)
		butterflies(fft->interleave, fft->lut + samples, 1, step_size, samples);
	resolve_float(out, fft->interleave, samples, 1.0f / samples);
}
