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

#define HSTIMER_IRQ_EN		(0x00)
#define HSTIMER_IRQ_STA		(0x04)
#define HSTIMER_CTRL(x)		((x + 1) * 0x20 + 0x00)
#define HSTIMER_INTV_LO(x)	((x + 1) * 0x20 + 0x04)
#define HSTIMER_INTV_HI(x)	((x + 1) * 0x20 + 0x08)
#define HSTIMER_CUR_LO(x)	((x + 1) * 0x20 + 0x0c)
#define HSTIMER_CUR_HI(x)	((x + 1) * 0x20 + 0x10)

struct ce_f101_hstimer_pdata_t {
	io_addr_t addr;
	char * clk;
	int reset;
	int irq;
};

static void ce_f101_hstimer_interrupt(void * data)
{
	struct clockevent_t * ce = (struct clockevent_t *)data;
	struct ce_f101_hstimer_pdata_t * pdat = (struct ce_f101_hstimer_pdata_t *)ce->priv;
	xos_io_write32(pdat->addr + HSTIMER_IRQ_STA, 1 << 0);
	ce->handler(ce, ce->data);
}

static int ce_f101_hstimer_next(struct clockevent_t * ce, uint64_t evt)
{
	struct ce_f101_hstimer_pdata_t * pdat = (struct ce_f101_hstimer_pdata_t *)ce->priv;
	uint32_t val;

	xos_io_write32(pdat->addr + HSTIMER_INTV_LO(0), ((evt >>  0) & 0xffffffff));
	xos_io_write32(pdat->addr + HSTIMER_INTV_HI(0), ((evt >> 32) & 0xffffffff));
	val = xos_io_read32(pdat->addr + HSTIMER_CTRL(0));
	val &= ~(0x3 << 0);
	val |= 0x2 << 0;
	xos_io_write32(pdat->addr + HSTIMER_CTRL(0), val);
	val &= ~(0x3 << 0);
	val |= 0x1 << 0;
	xos_io_write32(pdat->addr + HSTIMER_CTRL(0), val);
	return TRUE;
}

static struct device_t * ce_f101_hstimer_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct ce_f101_hstimer_pdata_t * pdat;
	struct clockevent_t * ce;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);
	int irq = dt_read_int(n, "interrupt", -1);
	uint64_t rate;
	uint32_t val;

	if(!search_clk(clk))
		return NULL;

	rate = clk_get_rate(clk);
	if(rate == 0)
		return NULL;

	if(!irq_is_valid(irq))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct ce_f101_hstimer_pdata_t));
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
	pdat->reset = dt_read_int(n, "reset", -1);
	pdat->irq = irq;

	clk_enable(pdat->clk);
	clockevent_calc_mult_shift(ce, rate, (uint32_t)XCLAMP((uint64_t)(0x00ffffffffffffffULL / rate), (uint64_t)1, (uint64_t)600));
	ce->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	ce->min_delta_ns = clockevent_delta2ns(ce, 0x1);
	ce->max_delta_ns = clockevent_delta2ns(ce, 0x00ffffffffffffffULL);
	ce->next = ce_f101_hstimer_next;
	ce->priv = pdat;

	if(pdat->reset >= 0)
		reset_deassert(pdat->reset);
	if(!request_irq(pdat->irq, ce_f101_hstimer_interrupt, IRQ_TYPE_NONE, ce))
	{
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);

		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	xos_io_write32(pdat->addr + HSTIMER_IRQ_EN, xos_io_read32(pdat->addr + HSTIMER_IRQ_EN) | (1 << 0));
	xos_io_write32(pdat->addr + HSTIMER_IRQ_STA, 1 << 0);
	val = xos_io_read32(pdat->addr + HSTIMER_CTRL(0));
	val &= ~((0x1 << 7) | (0x7 << 4) | (0x3 << 0));
	val |= (0x1 << 7) | (0x0 << 4) | (0x0 << 0);
	xos_io_write32(pdat->addr + HSTIMER_CTRL(0), val);

	if(!(dev = register_clockevent(ce, drv)))
	{
		clk_disable(pdat->clk);
		free_irq(pdat->irq);
		xos_mem_free(pdat->clk);
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
		return NULL;
	}
	return dev;
}

static void ce_f101_hstimer_remove(struct device_t * dev)
{
	struct clockevent_t * ce = (struct clockevent_t *)dev->priv;
	struct ce_f101_hstimer_pdata_t * pdat = (struct ce_f101_hstimer_pdata_t *)ce->priv;

	if(ce)
	{
		unregister_clockevent(ce);
		clk_disable(pdat->clk);
		free_irq(pdat->irq);
		xos_mem_free(pdat->clk);
		free_device_name(ce->name);
		xos_mem_free(ce->priv);
		xos_mem_free(ce);
	}
}

static void ce_f101_hstimer_suspend(struct device_t * dev)
{
}

static void ce_f101_hstimer_resume(struct device_t * dev)
{
}

static struct driver_t ce_f101_hstimer = {
	.name		= "ce-f101-hstimer",
	.probe		= ce_f101_hstimer_probe,
	.remove		= ce_f101_hstimer_remove,
	.suspend	= ce_f101_hstimer_suspend,
	.resume		= ce_f101_hstimer_resume,
};

static void ce_f101_hstimer_driver_init(void)
{
	register_driver(&ce_f101_hstimer);
}

static void ce_f101_hstimer_driver_exit(void)
{
	unregister_driver(&ce_f101_hstimer);
}

driver_initcall(ce_f101_hstimer_driver_init);
driver_exitcall(ce_f101_hstimer_driver_exit);
