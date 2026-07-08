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
	WDG_IRQ_EN		= 0x00,
	WDG_IRQ_STA		= 0x04,
	WDG_SOFT_RST	= 0x08,
	WDG_CTRL		= 0x10,
	WDG_CFG			= 0x14,
	WDG_MODE		= 0x18,
	WDG_OUTPUT_CFG	= 0x1c,
};

struct wdg_v821_rtc_pdata_t {
	io_addr_t addr;
	char * clk;
};

static void wdg_v821_rtc_set(struct watchdog_t * watchdog, int timeout)
{
	struct wdg_v821_rtc_pdata_t * pdat = (struct wdg_v821_rtc_pdata_t *)watchdog->priv;

	if(timeout < 0)
		timeout = 0;
	if(timeout > 0x1ff)
		timeout = 0x1ff;
	if(timeout > 0)
	{
		uint32_t val = xos_io_read32(pdat->addr + WDG_MODE) & 0xffff;
		val &= ~(0x3ff << 4);
		val |= ((timeout << 1) << 4) | (0x1 << 0);
		xos_io_write32(pdat->addr + WDG_MODE, (0x16aa << 16) | val);
		xos_io_write32(pdat->addr + WDG_CTRL, (0xa57 << 1) | (1 << 0));
	}
	else
	{
		xos_io_write32(pdat->addr + WDG_MODE, (0x16aa << 16) | (0 << 0));
		xos_io_write32(pdat->addr + WDG_CTRL, (0xa57 << 1) | (1 << 0));
	}
}

static int wdg_v821_rtc_get(struct watchdog_t * watchdog)
{
	return 0;
}

static struct device_t * wdg_v821_rtc_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct wdg_v821_rtc_pdata_t * pdat;
	struct watchdog_t * wdg;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct wdg_v821_rtc_pdata_t));
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
	wdg->set = wdg_v821_rtc_set;
	wdg->get = wdg_v821_rtc_get;
	wdg->priv = pdat;

	clk_enable(pdat->clk);
	xos_io_write32(pdat->addr + WDG_IRQ_EN, 0x0);
	xos_io_write32(pdat->addr + WDG_IRQ_STA, 0x1);
	xos_io_write32(pdat->addr + WDG_CFG, (0x16aa << 16) | (0x1 << 0));
	xos_io_write32(pdat->addr + WDG_MODE, (0x16aa << 16) | (0 << 0));
	xos_io_write32(pdat->addr + WDG_CTRL, (0xa57 << 1) | (1 << 0));

	if(!(dev = register_watchdog(wdg, drv)))
	{
		xos_io_write32(pdat->addr + WDG_MODE, (0x16aa << 16) | (0 << 0));
		xos_io_write32(pdat->addr + WDG_CTRL, (0xa57 << 1) | (1 << 0));
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(wdg->name);
		xos_mem_free(wdg->priv);
		xos_mem_free(wdg);
		return NULL;
	}
	return dev;
}

static void wdg_v821_rtc_remove(struct device_t * dev)
{
	struct watchdog_t * wdg = (struct watchdog_t *)dev->priv;
	struct wdg_v821_rtc_pdata_t * pdat = (struct wdg_v821_rtc_pdata_t *)wdg->priv;

	if(wdg)
	{
		unregister_watchdog(wdg);
		xos_io_write32(pdat->addr + WDG_MODE, 0x0);
		xos_io_write32(pdat->addr + WDG_CTRL, (0xa57 << 1) | (1 << 0));
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(wdg->name);
		xos_mem_free(wdg->priv);
		xos_mem_free(wdg);
	}
}

static void wdg_v821_rtc_suspend(struct device_t * dev)
{
}

static void wdg_v821_rtc_resume(struct device_t * dev)
{
}

static struct driver_t wdg_v821_rtc = {
	.name		= "wdg-v821-rtc",
	.probe		= wdg_v821_rtc_probe,
	.remove		= wdg_v821_rtc_remove,
	.suspend	= wdg_v821_rtc_suspend,
	.resume		= wdg_v821_rtc_resume,
};

static void wdg_v821_rtc_driver_init(void)
{
	register_driver(&wdg_v821_rtc);
}

static void wdg_v821_rtc_driver_exit(void)
{
	unregister_driver(&wdg_v821_rtc);
}

driver_initcall(wdg_v821_rtc_driver_init);
driver_exitcall(wdg_v821_rtc_driver_exit);
