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

#include <driver/spinlock/spinlock.h>
#include <driver/clocksource/clocksource.h>
#include <kernel/core/logger.h>

static struct logger_ctx_t {
	char buffer[CONFIG_XSTAR_LOGGER_SIZE];
	int head;
	int tail;
	int disabled;
	struct spinlock_t lock;
} __logger_ctx = { .head = 0, .tail = 0, .disabled = 0, .lock = { 0 } };

static void logger_push(struct logger_ctx_t * ctx, char c)
{
	spinlock_lock(&ctx->lock);
	{
		if(((ctx->tail + sizeof(ctx->buffer) - ctx->head) % sizeof(ctx->buffer)) == 1)
			ctx->tail = (ctx->tail + 1) % sizeof(ctx->buffer);
		ctx->buffer[ctx->head] = c;
		ctx->head = (ctx->head + 1) % sizeof(ctx->buffer);
	}
	spinlock_unlock(&ctx->lock);
}

static void logger_pop(struct logger_ctx_t * ctx)
{
	spinlock_lock(&ctx->lock);
	{
		while(ctx->tail != ctx->head)
		{
			char c = ctx->buffer[ctx->tail];
			if(xos_stdio_write(&c, 1) != 1)
				break;
			ctx->tail = (ctx->tail + 1) % sizeof(ctx->buffer);
		}
	}
	spinlock_unlock(&ctx->lock);
}

void logger_enable(void)
{
	struct logger_ctx_t * ctx = &__logger_ctx;

	spinlock_lock(&ctx->lock);
	{
		ctx->disabled = 0;
	}
	spinlock_unlock(&ctx->lock);
}

void logger_disable(void)
{
	struct logger_ctx_t * ctx = &__logger_ctx;

	spinlock_lock(&ctx->lock);
	{
		ctx->disabled = 1;
	}
	spinlock_unlock(&ctx->lock);
}

int logger_status(void)
{
	struct logger_ctx_t * ctx = &__logger_ctx;
	return ctx->disabled ? 0 : 1;
}

int logger(const char * fmt, ...)
{
	struct logger_ctx_t * ctx = &__logger_ctx;
	int len = 0;

	if(ctx && !ctx->disabled)
	{
		char * p1 = NULL;
		uint64_t us = ktime_to_us(ktime_get());
		len += xos_asprintf(&p1, "[%5u.%06u]", (unsigned long)(us / 1000000), (unsigned long)((us % 1000000)));
		for(char * p = p1; *p; p++)
			logger_push(ctx, *p);
		if(p1)
			xos_mem_free(p1);

		char * p2 = NULL;
		va_list ap;
		va_start(ap, fmt);
		len += xos_vasprintf(&p2, fmt, ap);
		va_end(ap);
		for(char * p = p2; *p; p++)
			logger_push(ctx, *p);
		if(p2)
			xos_mem_free(p2);

		logger_pop(ctx);
	}
	return len;
}
