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

#include <driver/clocksource/clocksource.h>
#include <kernel/core/logger.h>

static struct logger_ctx_t {
	char buffer[CONFIG_XSTAR_LOGGER_SIZE];
	int head;
	int tail;
	struct spinlock_t lock;
} __logger_ctx = { 0 };

static int logger_push(struct logger_ctx_t * ctx, const char * buf, int len)
{
	xos_spinlock_lock(&ctx->lock);
	for(int i = 0; i < len; i++)
	{
		if(((ctx->tail + sizeof(ctx->buffer) - ctx->head) % sizeof(ctx->buffer)) == 1)
			ctx->tail = (ctx->tail + 1) % sizeof(ctx->buffer);
		ctx->buffer[ctx->head] = buf[i];
		ctx->head = (ctx->head + 1) % sizeof(ctx->buffer);
	}
	xos_spinlock_unlock(&ctx->lock);
	return len;
}

int logger(const char * fmt, ...)
{
	char buf[1024];
	uint64_t us = ktime_to_us(ktime_get());
	int n = xos_snprintf(buf, sizeof(buf), "[%5u.%06u]", (unsigned long)(us / 1000000), (unsigned long)(us % 1000000));
	if(n < 0)
		n = 0;
	else if(n >= (int)sizeof(buf))
		n = sizeof(buf) - 1;
	va_list ap;
	va_start(ap, fmt);
	int m = xos_vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
	va_end(ap);
	if(m < 0)
		m = 0;
	else if(m >= (int)(sizeof(buf) - n))
		m = sizeof(buf) - n - 1;
	logger_push(&__logger_ctx, buf, n + m);
	return n + m;
}

int logger_dump(int * pos, int (*cb)(const char * buf, int len, void * data), void * data)
{
	if(cb)
	{
		char buf[1024];
		int total = 0;
		int start, end;
		xos_spinlock_lock(&__logger_ctx.lock);
		end = __logger_ctx.head;
		if(!pos || (*pos < 0))
			start = __logger_ctx.tail;
		else
		{
			start = *pos;
			if(((end + (int)sizeof(__logger_ctx.buffer) - start) % (int)sizeof(__logger_ctx.buffer)) > ((end + (int)sizeof(__logger_ctx.buffer) - __logger_ctx.tail) % (int)sizeof(__logger_ctx.buffer)))
				start = __logger_ctx.tail;
		}
		xos_spinlock_unlock(&__logger_ctx.lock);
		while(start != end)
		{
			int len = 0;
			xos_spinlock_lock(&__logger_ctx.lock);
			for(; (len < (int)sizeof(buf)) && (start != end); start = (start + 1) % (int)sizeof(__logger_ctx.buffer))
				buf[len++] = __logger_ctx.buffer[start];
			xos_spinlock_unlock(&__logger_ctx.lock);
			if(len > 0)
			{
				int w = cb(buf, len, data);
				if(w < 0)
					break;
				total += (w < len) ? w : len;
			}
		}
		if(pos)
			*pos = end;
		return total;
	}
	return 0;
}

void logger_clear(void)
{
	xos_spinlock_lock(&__logger_ctx.lock);
	__logger_ctx.head = 0;
	__logger_ctx.tail = 0;
	xos_spinlock_unlock(&__logger_ctx.lock);
}

static void logger_pure_init(void)
{
	__logger_ctx.head = 0;
	__logger_ctx.tail = 0;
	xos_spinlock_init(&__logger_ctx.lock);
}
pure_initcall(logger_pure_init);
