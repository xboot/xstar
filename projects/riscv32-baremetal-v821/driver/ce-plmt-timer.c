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
#include <riscv32.h>

extern void hook_core_interrupt(int cause, void (*func)(void *), void * data);

#define PLMT_MTIME			(0x0000)
#define PLMT_MTIMECMP(cpu)	(0x0008 + ((cpu) * 8))

struct ce_plmt_timer_pdata_t
{
	io_addr_t addr;
	char * clk;
	int cpu;
};

static void ce_plmt_timer_interrupt(void * data)
{
	struct clockevent_t * ce = (struct clockevent_t *)data;
	struct ce_plmt_timer_pdata_t * pdat = (struct ce_plmt_timer_pdata_t *)ce->priv;
	xos_io_write64(pdat->addr + PLMT_MTIMECMP(pdat->cpu), 0xffffffffffffffff);
	csr_clear(mie, MIE_MTIE);
	ce->handler(ce, ce->data);
}

static int ce_plmt_timer_next(struct clockevent_t * ce, uint64_t evt)
{
	struct ce_plmt_timer_pdata_t * pdat = (struct ce_plmt_timer_pdata_t *)ce->priv;
	uint64_t last = xos_io_read64(pdat->addr + PLMT_MTIME) + evt;
	xos_io_write64(pdat->addr + PLMT_MTIMECMP(pdat->cpu), last);
	csr_set(mie, MIE_MTIE);
	return 1;
}

static struct device_t * ce_plmt_timer_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct ce_plmt_timer_pdata_t * pdat;
	struct clockevent_t * ce;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct ce_plmt_timer_pdata_t));
	if(!pdat)
		return NULL;

	ce = xos_mem_malloc(sizeof(struct clockevent_t));
	if(!ce)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->clk = xos_strdup(clk);
	pdat->cpu = smp_processor_id();

	clk_enable(pdat->clk);
	clockevent_calc_mult_shift(ce, clk_get_rate(pdat->clk), 10);
	ce->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
	ce->max_delta_ns = clockevent_delta2ns(ce, 0xffffffffffffffff);
	ce->next = ce_plmt_timer_next;
	ce->priv = pdat;

	hook_core_interrupt(7, ce_plmt_timer_interrupt, ce);
	xos_io_write64(pdat->addr + PLMT_MTIMECMP(pdat->cpu), 0xffffffffffffffff);
	csr_clear(mie, MIE_MTIE);
	csr_set(mstatus, MSTATUS_MIE);

	if(!(dev = register_clockevent(ce, drv)))
	{
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	return dev;
}

static void ce_plmt_timer_remove(struct device_t * dev)
{
	struct clockevent_t * ce = (struct clockevent_t *)dev->priv;
	struct ce_plmt_timer_pdata_t * pdat = (struct ce_plmt_timer_pdata_t *)ce->priv;

	if(ce)
	{
		unregister_clockevent(ce);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
	}
}

static void ce_plmt_timer_suspend(struct device_t * dev)
{
}

static void ce_plmt_timer_resume(struct device_t * dev)
{
}

static struct driver_t ce_plmt_timer = {
	.name		= "ce-plmt-timer",
	.probe		= ce_plmt_timer_probe,
	.remove		= ce_plmt_timer_remove,
	.suspend	= ce_plmt_timer_suspend,
	.resume		= ce_plmt_timer_resume,
};

static void ce_plmt_timer_driver_init(void)
{
	register_driver(&ce_plmt_timer);
}

static void ce_plmt_timer_driver_exit(void)
{
	unregister_driver(&ce_plmt_timer);
}

driver_initcall(ce_plmt_timer_driver_init);
driver_exitcall(ce_plmt_timer_driver_exit);
