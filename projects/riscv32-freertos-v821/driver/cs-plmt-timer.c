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

#define PLMT_MTIME			(0x0000)
#define PLMT_MTIMECMP(cpu)	(0x0008 + ((cpu) * 8))

struct cs_plmt_timer_pdata_t {
	io_addr_t addr;
	char * clk;
};

static uint64_t cs_plmt_timer_read(struct clocksource_t * cs)
{
	struct cs_plmt_timer_pdata_t * pdat = (struct cs_plmt_timer_pdata_t *)cs->priv;
	return xos_io_read64(pdat->addr + PLMT_MTIME);
}

static struct device_t * cs_plmt_timer_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct cs_plmt_timer_pdata_t * pdat;
	struct clocksource_t * cs;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct cs_plmt_timer_pdata_t));
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

	clk_enable(pdat->clk);
	clocksource_calc_mult_shift(&cs->mult, &cs->shift, clk_get_rate(pdat->clk), 1000000000ULL, 10);
	cs->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	cs->mask = CLOCKSOURCE_MASK(64);
	cs->read = cs_plmt_timer_read;
	cs->priv = pdat;

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

static void cs_plmt_timer_remove(struct device_t * dev)
{
	struct clocksource_t * cs = (struct clocksource_t *)dev->priv;
	struct cs_plmt_timer_pdata_t * pdat = (struct cs_plmt_timer_pdata_t *)cs->priv;

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

static void cs_plmt_timer_suspend(struct device_t * dev)
{
}

static void cs_plmt_timer_resume(struct device_t * dev)
{
}

static struct driver_t cs_plmt_timer = {
	.name		= "cs-plmt-timer",
	.probe		= cs_plmt_timer_probe,
	.remove		= cs_plmt_timer_remove,
	.suspend	= cs_plmt_timer_suspend,
	.resume		= cs_plmt_timer_resume,
};

static void cs_plmt_timer_driver_init(void)
{
	register_driver(&cs_plmt_timer);
}

static void cs_plmt_timer_driver_exit(void)
{
	unregister_driver(&cs_plmt_timer);
}

driver_initcall(cs_plmt_timer_driver_init);
driver_exitcall(cs_plmt_timer_driver_exit);
