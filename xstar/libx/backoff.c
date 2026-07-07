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
#include <libx/backoff.h>

void backoff_init(struct backoff_t * ctx, int delay, int max_delay, int max_retry)
{
	ctx->max_delay = XMAX(max_delay, 0);
	ctx->max_retry = max_retry;
	ctx->delay = XCLAMP(delay, 0, ctx->max_delay);
	ctx->retry = 0;
}

int backoff_next(struct backoff_t * ctx)
{
	if((ctx->max_retry <= 0) || (ctx->retry < ctx->max_retry))
	{
		int next = (int)(xos_rand() % (ctx->delay + 1));
		ctx->retry++;
		if(ctx->delay < (ctx->max_delay >> 1))
			ctx->delay += XMAX(ctx->delay, 1);
		else
			ctx->delay = ctx->max_delay;
		return next;
	}
	return -1;
}
