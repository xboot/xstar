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

struct reset_f133_pdata_t
{
	io_addr_t addr;
	int base;
	int nreset;
};

static void reset_f133_assert(struct resetchip_t * chip, int offset)
{
	struct reset_f133_pdata_t * pdat = (struct reset_f133_pdata_t *)chip->priv;
	uint32_t val;

	if(offset >= chip->nreset)
		return;

	val = xos_io_read32(pdat->addr);
	val &= ~(1 << offset);
	xos_io_write32(pdat->addr, val);
}

static void reset_f133_deassert(struct resetchip_t * chip, int offset)
{
	struct reset_f133_pdata_t * pdat = (struct reset_f133_pdata_t *)chip->priv;
	uint32_t val;

	if(offset >= chip->nreset)
		return;

	val = xos_io_read32(pdat->addr);
	val |= (1 << offset);
	xos_io_write32(pdat->addr, val);
}

static struct device_t * reset_f133_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct reset_f133_pdata_t * pdat;
	struct resetchip_t * chip;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	int base = dt_read_int(n, "reset-base", -1);
	int nreset = dt_read_int(n, "reset-count", -1);

	if((base < 0) || (nreset <= 0))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct reset_f133_pdata_t));
	if(!pdat)
		return NULL;

	chip = xos_mem_malloc(sizeof(struct resetchip_t));
	if(!chip)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->base = base;
	pdat->nreset = nreset;

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->nreset = pdat->nreset;
	chip->assert = reset_f133_assert;
	chip->deassert = reset_f133_deassert;
	chip->priv = pdat;

	if(!(dev = register_resetchip(chip, drv)))
	{
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
		return NULL;
	}
	return dev;
}

static void reset_f133_remove(struct device_t * dev)
{
	struct resetchip_t * chip = (struct resetchip_t *)dev->priv;

	if(chip)
	{
		unregister_resetchip(chip);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
	}
}

static void reset_f133_suspend(struct device_t * dev)
{
}

static void reset_f133_resume(struct device_t * dev)
{
}

static struct driver_t reset_f133 = {
	.name		= "reset-f133",
	.probe		= reset_f133_probe,
	.remove		= reset_f133_remove,
	.suspend	= reset_f133_suspend,
	.resume		= reset_f133_resume,
};

static void reset_f133_driver_init(void)
{
	register_driver(&reset_f133);
}

static void reset_f133_driver_exit(void)
{
	unregister_driver(&reset_f133);
}

driver_initcall(reset_f133_driver_init);
driver_exitcall(reset_f133_driver_exit);
