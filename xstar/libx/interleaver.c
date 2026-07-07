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
#include <libx/interleaver.h>

static inline uint32_t lfsr_random(uint32_t * state)
{
	int feedback = *state & 0x1;
	*state >>= 1;
	if(feedback)
		*state ^= 0xb4bcd35c;
	return *state;
}

struct interleaver_t * interleaver_alloc(int size, int seed)
{
	if(size > 0)
	{
		struct interleaver_t * ctx = xos_mem_malloc(sizeof(struct interleaver_t) + 2 * size * sizeof(int));
		if(ctx)
		{
			ctx->size = size;
			ctx->pattern = (int *)((char *)ctx + sizeof(struct interleaver_t));
			ctx->ipattern = ctx->pattern + size;
			uint32_t state = seed;
			for(int i = 0; i < size; i++)
				ctx->pattern[i] = i;
			for(int i = size - 1; i > 0; i--)
			{
				int j = lfsr_random(&state) % (i + 1);
				int t = ctx->pattern[i];
				ctx->pattern[i] = ctx->pattern[j];
				ctx->pattern[j] = t;
			}
			for(int i = 0; i < size; i++)
				ctx->ipattern[ctx->pattern[i]] = i;
			return ctx;
		}
	}
	return NULL;
}

void interleaver_free(struct interleaver_t * ctx)
{
	if(ctx)
		xos_mem_free(ctx);
}

void interleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output)
{
	if(ctx && input && output)
	{
		for(int i = 0; i < ctx->size; i++)
			output[i] = input[ctx->pattern[i]];
	}
}

void deinterleave(struct interleaver_t * ctx, uint8_t * input, uint8_t * output)
{
	if(ctx && input && output)
	{
		for(int i = 0; i < ctx->size; i++)
			output[i] = input[ctx->ipattern[i]];
	}
}
