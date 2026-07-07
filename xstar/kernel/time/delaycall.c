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

#include <kernel/time/timer.h>
#include <kernel/time/delaycall.h>

struct delaycall_pdat_t {
	struct timer_t timer;
	void (*func)(void * data);
	void * data;
};

static int delaycall_timer_function(struct timer_t * timer, void * data)
{
	struct delaycall_pdat_t * pdat = (struct delaycall_pdat_t *)(data);

	if(pdat)
	{
		if(pdat->func)
			pdat->func(pdat->data);
		xos_mem_free(pdat);
	}
	return 0;
}

void delaycall(unsigned int ms, void (*func)(void *), void * data)
{
	if((ms > 0) && func)
	{
		struct delaycall_pdat_t * pdat = xos_mem_malloc(sizeof(struct delaycall_pdat_t));
		if(pdat)
		{
			pdat->func = func;
			pdat->data = data;
			timer_init(&pdat->timer, delaycall_timer_function, pdat);
			timer_start(&pdat->timer, ms_to_ktime(ms));
		}
	}
}
