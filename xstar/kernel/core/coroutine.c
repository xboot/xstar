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
#include <kernel/core/coroutine.h>

static inline struct coroutine_t * scheduler_next_coroutine(struct scheduler_t * sched)
{
	if(list_empty(&sched->ready))
		return NULL;
	if(list_is_last(&sched->running->list, &sched->ready))
		return list_first_entry(&sched->ready, struct coroutine_t, list);
	else
		return list_next_entry(sched->running, list);
}

static inline void scheduler_switch_coroutine(struct scheduler_t * sched, struct coroutine_t * co)
{
	struct coroutine_t * running = sched->running;
	sched->running = co;
	struct co_transfer_t from = xos_coroutine_jump(co->fctx, running);
	struct coroutine_t * t = (struct coroutine_t *)from.priv;
	t->fctx = from.fctx;
}

static void fcontext_entry(struct co_transfer_t from)
{
	struct coroutine_t * t = (struct coroutine_t *)from.priv;
	struct scheduler_t * sched = t->sched;
	struct coroutine_t * co = sched->running;

	if(!sched->fctx)
		sched->fctx = from.fctx;

	t->fctx = from.fctx;
	co->func(sched, co->data);
	struct coroutine_t * next = scheduler_next_coroutine(sched);
	list_del(&co->list);
	xos_mem_free(co->stack);
	xos_mem_free(co);

	if(next && (next != sched->running))
		scheduler_switch_coroutine(sched, next);
	else
		xos_coroutine_jump(sched->fctx, NULL);
}

struct coroutine_t * coroutine_start(struct scheduler_t * sched, void (*func)(struct scheduler_t *, void *), void * data, size_t stksz)
{
	struct coroutine_t * co;
	void * stack;

	if(!sched || !func)
		return NULL;

	if(stksz <= 0)
		stksz = 4096;

	co = xos_mem_malloc(sizeof(struct coroutine_t));
	if(!co)
		return NULL;

	stack = xos_mem_malloc(stksz);
	if(!stack)
	{
		xos_mem_free(co);
		return NULL;
	}

	init_list_head(&co->list);
	list_add_tail(&co->list, &sched->ready);
	co->sched = sched;
	co->stack = stack;
	co->fctx = xos_coroutine_make(co->stack + stksz, stksz, fcontext_entry);
	co->func = func;
	co->data = data;

	return co;
}

void coroutine_yield(struct scheduler_t * sched)
{
	if(sched)
	{
		struct coroutine_t * next = scheduler_next_coroutine(sched);
		if(next && (next != sched->running))
			scheduler_switch_coroutine(sched, next);
	}
}

void coroutine_nsleep(struct scheduler_t * sched, uint32_t ns)
{
	ktime_t timeout = ktime_add_ns(ktime_get(), ns);

	do {
		coroutine_yield(sched);
	} while(ktime_before(ktime_get(), timeout));
}

void coroutine_usleep(struct scheduler_t * sched, uint32_t us)
{
	ktime_t timeout = ktime_add_us(ktime_get(), us);

	do {
		coroutine_yield(sched);
	} while(ktime_before(ktime_get(), timeout));
}

void coroutine_msleep(struct scheduler_t * sched, uint32_t ms)
{
	ktime_t timeout = ktime_add_ms(ktime_get(), ms);

	do {
		coroutine_yield(sched);
	} while(ktime_before(ktime_get(), timeout));
}

void scheduler_init(struct scheduler_t * sched)
{
	if(sched)
	{
		init_list_head(&sched->ready);
		sched->running = NULL;
		sched->fctx = NULL;
	}
}

void scheduler_loop(struct scheduler_t * sched)
{
	if(sched && !list_empty(&sched->ready))
	{
		sched->running = list_first_entry(&sched->ready, struct coroutine_t, list);
		xos_coroutine_jump(sched->running->fctx, sched->running);
	}
}
