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

enum {
	WDT_CR		= 0x00,
	WDT_TORR	= 0x04,
	WDT_CCVR	= 0x08,
	WDT_CRR		= 0x0c,
	WDT_STAT	= 0x10,
	WDT_EOI		= 0x14,
};

struct wdg_rv1106_pdata_t {
	io_addr_t addr;
	char * clk;
};

static void wdg_rv1106_set(struct watchdog_t * watchdog, int timeout)
{
	struct wdg_rv1106_pdata_t * pdat = (struct wdg_rv1106_pdata_t *)watchdog->priv;
	uint64_t rate = clk_get_rate(pdat->clk);
	int maxtime = 0x80000000 / clk_get_rate(pdat->clk) + 1;

	if(timeout < 0)
		timeout = 0;
	if(timeout > maxtime)
		timeout = maxtime;

	if(timeout > 0)
	{
		uint32_t torr = 0, acc = 1;
		uint32_t count = (timeout * rate) / 0x10000;

		while(acc < count)
		{
			acc *= 2;
			torr++;
		}
		if(torr > 15)
			torr = 15;

		xos_io_write32(pdat->addr + WDT_CR, 0);
		xos_io_write32(pdat->addr + WDT_TORR, torr);
		xos_io_write32(pdat->addr + WDT_CRR, 0x76);
		xos_io_write32(pdat->addr + WDT_CR, (0x3 << 2) | (0x0 << 1) | (0x1 << 0));
	}
	else
	{
		xos_io_write32(pdat->addr + WDT_CRR, 0x76);
		xos_io_write32(pdat->addr + WDT_CR, 0x0);
	}
}

static int wdg_rv1106_get(struct watchdog_t * watchdog)
{
	struct wdg_rv1106_pdata_t * pdat = (struct wdg_rv1106_pdata_t *)watchdog->priv;
	return (int)(xos_io_read32(pdat->addr + WDT_CCVR) / clk_get_rate(pdat->clk));
}

static struct device_t * wdg_rv1106_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct wdg_rv1106_pdata_t * pdat;
	struct watchdog_t * wdg;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct wdg_rv1106_pdata_t));
	if(!pdat)
		return NULL;

	wdg = xos_mem_malloc(sizeof(struct watchdog_t));
	if(!wdg)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->clk = xos_strdup(clk);

	wdg->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	wdg->set = wdg_rv1106_set;
	wdg->get = wdg_rv1106_get;
	wdg->priv = pdat;

	clk_enable(pdat->clk);
	xos_io_write32(pdat->addr + WDT_CRR, 0x76);
	xos_io_write32(pdat->addr + WDT_CR, 0x0);

	if(!(dev = register_watchdog(wdg, drv)))
	{
		xos_io_write32(pdat->addr + WDT_CRR, 0x76);
		xos_io_write32(pdat->addr + WDT_CR, 0x0);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(wdg->name);
		xos_mem_free(wdg->priv);
		xos_mem_free(wdg);
		return NULL;
	}
	return dev;
}

static void wdg_rv1106_remove(struct device_t * dev)
{
	struct watchdog_t * wdg = (struct watchdog_t *)dev->priv;
	struct wdg_rv1106_pdata_t * pdat = (struct wdg_rv1106_pdata_t *)wdg->priv;

	if(wdg)
	{
		unregister_watchdog(wdg);
		xos_io_write32(pdat->addr + WDT_CRR, 0x76);
		xos_io_write32(pdat->addr + WDT_CR, 0x0);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(wdg->name);
		xos_mem_free(wdg->priv);
		xos_mem_free(wdg);
	}
}

static void wdg_rv1106_suspend(struct device_t * dev)
{
}

static void wdg_rv1106_resume(struct device_t * dev)
{
}

static struct driver_t wdg_rv1106 = {
	.name		= "wdg-rv1106",
	.probe		= wdg_rv1106_probe,
	.remove		= wdg_rv1106_remove,
	.suspend	= wdg_rv1106_suspend,
	.resume		= wdg_rv1106_resume,
};

static void wdg_rv1106_driver_init(void)
{
	register_driver(&wdg_rv1106);
}

static void wdg_rv1106_driver_exit(void)
{
	unregister_driver(&wdg_rv1106);
}

driver_initcall(wdg_rv1106_driver_init);
driver_exitcall(wdg_rv1106_driver_exit);
