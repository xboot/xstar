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

#define HWSPINLOCK_F101_STATUS	(0x010)
#define HWSPINLOCK_F101_LOCK(x)	(0x100 + (x) * 4)

struct hwspinlock_f101_pdata_t {
	io_addr_t addr;
	int base;
	int nlock;
};

static int hwspinlock_f101_trylock(struct hwspinlock_t * hsl, int lock)
{
	struct hwspinlock_f101_pdata_t * pdat = (struct hwspinlock_f101_pdata_t *)hsl->priv;
	return (xos_io_read32(pdat->addr + HWSPINLOCK_F101_LOCK(lock)) == 0) ? 1 : 0;
}

static void hwspinlock_f101_lock(struct hwspinlock_t * hsl, int lock)
{
	struct hwspinlock_f101_pdata_t * pdat = (struct hwspinlock_f101_pdata_t *)hsl->priv;
	while(xos_io_read32(pdat->addr + HWSPINLOCK_F101_LOCK(lock)) != 0);
}

static void hwspinlock_f101_unlock(struct hwspinlock_t * hsl, int lock)
{
	struct hwspinlock_f101_pdata_t * pdat = (struct hwspinlock_f101_pdata_t *)hsl->priv;
	xos_io_write32(pdat->addr + HWSPINLOCK_F101_LOCK(lock), 0);
}

static struct device_t * hwspinlock_f101_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct hwspinlock_f101_pdata_t * pdat;
	struct hwspinlock_t * hsl;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	int base = dt_read_int(n, "hwspinlock-base", -1);
	int nlock = dt_read_int(n, "hwspinlock-count", -1);

	if((base < 0) || (nlock <= 0))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct hwspinlock_f101_pdata_t));
	if(!pdat)
		return NULL;

	hsl = xos_mem_malloc(sizeof(struct hwspinlock_t));
	if(!hsl)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->base = base;
	pdat->nlock = nlock;

	hsl->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	hsl->base = pdat->base;
	hsl->nlock = pdat->nlock;
	hsl->trylock = hwspinlock_f101_trylock;
	hsl->lock = hwspinlock_f101_lock;
	hsl->unlock = hwspinlock_f101_unlock;
	hsl->priv = pdat;

	if(!(dev = register_hwspinlock(hsl, drv)))
	{
		free_device_name(hsl->name);
		xos_mem_free(hsl);
		xos_mem_free(pdat);
		return NULL;
	}
	return dev;
}

static void hwspinlock_f101_remove(struct device_t * dev)
{
	struct hwspinlock_t * hsl = (struct hwspinlock_t *)dev->priv;

	if(hsl)
	{
		unregister_hwspinlock(hsl);
		free_device_name(hsl->name);
		xos_mem_free(hsl->priv);
		xos_mem_free(hsl);
	}
}

static void hwspinlock_f101_suspend(struct device_t * dev)
{
}

static void hwspinlock_f101_resume(struct device_t * dev)
{
}

static struct driver_t hwspinlock_f101 = {
	.name		= "hwspinlock-f101",
	.probe		= hwspinlock_f101_probe,
	.remove		= hwspinlock_f101_remove,
	.suspend	= hwspinlock_f101_suspend,
	.resume		= hwspinlock_f101_resume,
};

static void hwspinlock_f101_driver_init(void)
{
	register_driver(&hwspinlock_f101);
}

static void hwspinlock_f101_driver_exit(void)
{
	unregister_driver(&hwspinlock_f101);
}

driver_initcall(hwspinlock_f101_driver_init);
driver_exitcall(hwspinlock_f101_driver_exit);
