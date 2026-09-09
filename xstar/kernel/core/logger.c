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
	int enable;
	struct spinlock_t lock;
	struct xatomic_t atomic;
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

static void logger_pop(struct logger_ctx_t * ctx)
{
	if(xatomic_cas(&ctx->atomic, 0, 1))
	{
		char buf[512];
		while(ctx->tail != ctx->head)
		{
			int len = 0;
			int start;
			int w = 0;
			xos_spinlock_lock(&ctx->lock);
			{
				start = ctx->tail;
				for(int tail = ctx->tail; (len < (int)sizeof(buf)) && (tail != ctx->head); tail = (tail + 1) % (int)sizeof(ctx->buffer))
					buf[len++] = ctx->buffer[tail];
			}
			xos_spinlock_unlock(&ctx->lock);
			if(!len)
				break;
			while(w < len)
			{
				ssize_t r = xos_stdio_write(&buf[w], len - w);
				if(r <= 0)
					break;
				w += r;
			}
			xos_spinlock_lock(&ctx->lock);
			{
				if((int)((ctx->tail + sizeof(ctx->buffer) - start) % sizeof(ctx->buffer)) < w)
					ctx->tail = (start + w) % (int)sizeof(ctx->buffer);
			}
			xos_spinlock_unlock(&ctx->lock);
			if(w < len)
				break;
		}
		xatomic_store_release(&ctx->atomic, 0);
	}
}

void logger_enable(void)
{
	xos_spinlock_lock(&__logger_ctx.lock);
	__logger_ctx.enable = 1;
	xos_spinlock_unlock(&__logger_ctx.lock);
}

void logger_disable(void)
{
	xos_spinlock_lock(&__logger_ctx.lock);
	__logger_ctx.enable = 0;
	xos_spinlock_unlock(&__logger_ctx.lock);
}

int logger_status(void)
{
	return __logger_ctx.enable ? 1 : 0;
}

int logger(const char * fmt, ...)
{
	if(__logger_ctx.enable)
	{
		char buf[512];
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
		logger_pop(&__logger_ctx);
		return n + m;
	}
	return 0;
}

static void logger_pure_init(void)
{
	__logger_ctx.head = 0;
	__logger_ctx.tail = 0;
	__logger_ctx.enable = 1;
	xos_spinlock_init(&__logger_ctx.lock);
	xatomic_store(&__logger_ctx.atomic, 0);
}
pure_initcall(logger_pure_init);
