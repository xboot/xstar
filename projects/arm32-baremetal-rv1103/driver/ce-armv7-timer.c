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

#include <xstar.h>

#define armv7_read_cntpctl()		({ uint32_t val; __asm__ __volatile__("mrc p15, 0, %0, c14, c2, 1" : "=r" (val)); val; })
#define armv7_write_cntpctl(val)	__asm__ __volatile__("mcr p15, 0, %0, c14, c2, 1\nisb sy\n" : : "r" (val));

static inline void armv7_timer_start(void)
{
	uint32_t ctrl = armv7_read_cntpctl();
	if(!(ctrl & (1 << 0)))
	{
		ctrl |= (1 << 0);
		armv7_write_cntpctl(ctrl);
	}
}

static inline void armv7_timer_stop(void)
{
	uint32_t ctrl = armv7_read_cntpctl();
	if((ctrl & (1 << 0)))
	{
		ctrl &= ~(1 << 0);
		armv7_write_cntpctl(ctrl);
	}
}

static inline void armv7_timer_interrupt_enable(void)
{
	uint32_t ctrl = armv7_read_cntpctl();
	if(ctrl & (1 << 1))
	{
		ctrl &= ~(1 << 1);
		armv7_write_cntpctl(ctrl);
	}
}

static inline void armv7_timer_interrupt_disable(void)
{
	uint32_t ctrl = armv7_read_cntpctl();
	if(!(ctrl & (1 << 1)))
	{
		ctrl |= (1 << 1);
		armv7_write_cntpctl(ctrl);
	}
}

static inline uint64_t armv7_timer_frequecy(void)
{
	uint32_t v;

	__asm__ __volatile__("mrc p15, 0, %0, c14, c0, 0" : "=r" (v));
	return (v != 0) ? (uint64_t)v : 1000000;
}

static inline uint64_t armv7_timer_read(void)
{
	uint32_t l, h;

	__asm__ __volatile__("mrrc p15, 0, %0, %1, c14" : "=r" (l), "=r" (h));
	return ((uint64_t)h << 32) | l;
}

static inline void armv7_timer_compare(uint64_t interval)
{
	uint64_t last = armv7_timer_read() + interval;

	__asm__ __volatile__("mcrr p15, 2, %Q0, %R0, c14" : : "r" (last));
}

struct ce_armv7_timer_pdata_t
{
	int irq;
};

static void ce_armv7_timer_interrupt(void * data)
{
	struct clockevent_t * ce = (struct clockevent_t *)data;
	ce->handler(ce, ce->data);
}

static int ce_armv7_timer_next(struct clockevent_t * ce, uint64_t evt)
{
	armv7_timer_compare(evt);
	armv7_timer_interrupt_enable();
	armv7_timer_start();
	return TRUE;
}

static struct device_t * ce_armv7_timer_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct ce_armv7_timer_pdata_t * pdat;
	struct clockevent_t * ce;
	struct device_t * dev;
	int64_t rate = (int64_t)dt_read_long(n, "clock-frequency", -1);
	int irq = dt_read_int(n, "interrupt", -1);

	if(!irq_is_valid(irq))
		return NULL;

	if(rate <= 0)
		rate = armv7_timer_frequecy();

	pdat = xos_mem_malloc(sizeof(struct ce_armv7_timer_pdata_t));
	if(!pdat)
		return NULL;

	ce = xos_mem_malloc(sizeof(struct clockevent_t));
	if(!ce)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->irq = irq;

	clockevent_calc_mult_shift(ce, rate, 10);
	ce->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
	ce->max_delta_ns = clockevent_delta2ns(ce, 0xffffffff);
	ce->next = ce_armv7_timer_next;
	ce->priv = pdat;

	if(!request_irq(pdat->irq, ce_armv7_timer_interrupt, IRQ_TYPE_NONE, ce))
	{
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	armv7_timer_compare(0xffffffff);
	armv7_timer_interrupt_disable();
	armv7_timer_stop();

	if(!(dev = register_clockevent(ce, drv)))
	{
		free_irq(pdat->irq);
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	return dev;
}

static void ce_armv7_timer_remove(struct device_t * dev)
{
	struct clockevent_t * ce = (struct clockevent_t *)dev->priv;
	struct ce_armv7_timer_pdata_t * pdat = (struct ce_armv7_timer_pdata_t *)ce->priv;

	if(ce)
	{
		unregister_clockevent(ce);
		free_irq(pdat->irq);
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
	}
}

static void ce_armv7_timer_suspend(struct device_t * dev)
{
}

static void ce_armv7_timer_resume(struct device_t * dev)
{
}

static struct driver_t ce_armv7_timer = {
	.name		= "ce-armv7-timer",
	.probe		= ce_armv7_timer_probe,
	.remove		= ce_armv7_timer_remove,
	.suspend	= ce_armv7_timer_suspend,
	.resume		= ce_armv7_timer_resume,
};

static void ce_armv7_timer_driver_init(void)
{
	register_driver(&ce_armv7_timer);
}

static void ce_armv7_timer_driver_exit(void)
{
	unregister_driver(&ce_armv7_timer);
}

driver_initcall(ce_armv7_timer_driver_init);
driver_exitcall(ce_armv7_timer_driver_exit);
