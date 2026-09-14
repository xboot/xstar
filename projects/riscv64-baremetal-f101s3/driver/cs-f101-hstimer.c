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

struct cs_f101_hstimer_pdata_t {
	io_addr_t addr;
	char * clk;
	int reset;
};

static uint64_t cs_f101_hstimer_read(struct clocksource_t * cs)
{
	struct cs_f101_hstimer_pdata_t * pdat = (struct cs_f101_hstimer_pdata_t *)cs->priv;
	uint32_t lo = xos_io_read32(pdat->addr + HSTIMER_CUR_LO(1));
	uint32_t hi = xos_io_read32(pdat->addr + HSTIMER_CUR_HI(1));
	return (uint64_t)(0xffffffffffffffffULL - (((uint64_t)hi << 32) | lo));
}

static struct device_t * cs_f101_hstimer_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct cs_f101_hstimer_pdata_t * pdat;
	struct clocksource_t * cs;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);
	uint64_t rate;
	uint32_t val;

	if(!search_clk(clk))
		return NULL;

	rate = clk_get_rate(clk);
	if(rate == 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct cs_f101_hstimer_pdata_t));
	if(!pdat)
		return NULL;

	cs = xos_mem_malloc(sizeof(struct clocksource_t));
	if(!cs)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->clk = xos_strdup(clk);
	pdat->reset = dt_read_int(n, "reset", -1);

	clk_enable(pdat->clk);
	cs->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	cs->mask = CLOCKSOURCE_MASK(56);
	clocksource_calc_mult_shift(&cs->mult, &cs->shift, rate, 1000000000ULL, (uint32_t)XCLAMP((uint64_t)(cs->mask / rate), (uint64_t)1, (uint64_t)600));
	cs->read = cs_f101_hstimer_read;
	cs->priv = pdat;

	if(pdat->reset >= 0)
		reset_deassert(pdat->reset);
	xos_io_write32(pdat->addr + HSTIMER_IRQ_EN, xos_io_read32(pdat->addr + HSTIMER_IRQ_EN) & ~(1 << 1));
	xos_io_write32(pdat->addr + HSTIMER_IRQ_STA, 1 << 1);
	xos_io_write32(pdat->addr + HSTIMER_INTV_LO(1), 0xffffffff);
	xos_io_write32(pdat->addr + HSTIMER_INTV_HI(1), 0x00ffffff);
	val = xos_io_read32(pdat->addr + HSTIMER_CTRL(1));
	val &= ~((0x1 << 7) | (0x7 << 4) | (0x3 << 0));
	val |= (0x0 << 7) | (0x0 << 4) | (0x1 << 0);
	xos_io_write32(pdat->addr + HSTIMER_CTRL(1), val);

	if(!(dev = register_clocksource(cs, drv)))
	{
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(cs->name);
		xos_mem_free(cs->priv);
		xos_mem_free(cs);
		return NULL;
	}
	return dev;
}

static void cs_f101_hstimer_remove(struct device_t * dev)
{
	struct clocksource_t * cs = (struct clocksource_t *)dev->priv;
	struct cs_f101_hstimer_pdata_t * pdat = (struct cs_f101_hstimer_pdata_t *)cs->priv;

	if(cs)
	{
		unregister_clocksource(cs);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(cs->name);
		xos_mem_free(cs->priv);
		xos_mem_free(cs);
	}
}

static void cs_f101_hstimer_suspend(struct device_t * dev)
{
}

static void cs_f101_hstimer_resume(struct device_t * dev)
{
}

static struct driver_t cs_f101_hstimer = {
	.name		= "cs-f101-hstimer",
	.probe		= cs_f101_hstimer_probe,
	.remove		= cs_f101_hstimer_remove,
	.suspend	= cs_f101_hstimer_suspend,
	.resume		= cs_f101_hstimer_resume,
};

static void cs_f101_hstimer_driver_init(void)
{
	register_driver(&cs_f101_hstimer);
}

static void cs_f101_hstimer_driver_exit(void)
{
	unregister_driver(&cs_f101_hstimer);
}

driver_initcall(cs_f101_hstimer_driver_init);
driver_exitcall(cs_f101_hstimer_driver_exit);
