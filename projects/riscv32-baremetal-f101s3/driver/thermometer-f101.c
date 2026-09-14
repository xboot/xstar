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

/*
 * F101 - Thermal Sensor Controller (THS)
 *
 * Required properties:
 * - clock-name: ths bus gate clock name
 *
 * Optional properties:
 * - reset: ths reset id
 *
 * Example device tree node:
 *   "thermometer-f101@0x02009400": {
 *       "clock-name": "ths",
 *       "reset": 688
 *   }
 */

enum {
	THS_CTRL			= 0x00,
	THS_EN				= 0x04,
	THS_PER				= 0x08,
	THS_DATA_INTC		= 0x10,
	THS_DATA_INTS		= 0x20,
	THS_FILTER			= 0x30,
	THS_CALIB			= 0xa0,
	THS_DATA			= 0xc0,
};

struct thermometer_f101_pdata_t {
	io_addr_t addr;
	char * clk;
	int reset;
};

enum {
	SID_PRCTL		= 0x03006000 + 0x00,
	SID_PR_ADDR		= 0x03006000 + 0x04,
	SID_PRKEY		= 0x03006000 + 0x08,
	SID_RDKEY		= 0x03006000 + 0x0C,
	EFUSE_HV_SWITCH	= 0x03090000 + 0x204,
};

static uint32_t efuse_read(uint32_t offset)
{
	uint32_t val;

	xos_io_write32(SID_PR_ADDR, (offset >> 2));
	val = xos_io_read32(SID_PRCTL);
	val &= ~((0xffff << 16) | 0x3);
	val |= (0xadbf << 16) | 0x2;
	xos_io_write32(SID_PRCTL, val);
	while(xos_io_read32(SID_PRCTL) & 0x2);
	val &= ~((0xffff << 16) | 0x3);
	xos_io_write32(SID_PRCTL, val);
	val = xos_io_read32(SID_RDKEY);
	return val;
}

static void thermometer_f101_calibrate(struct thermometer_f101_pdata_t * pdat)
{
	uint32_t val = efuse_read(0x14);
	uint8_t cal[4];

	cal[0] = (uint8_t)(val >> 0);
	cal[1] = (uint8_t)(val >> 8);
	cal[2] = (uint8_t)(val >> 16);
	cal[3] = (uint8_t)(val >> 24);
	int ft_temp = (int)(((cal[1] << 8) | cal[0]) & 0xfff);
	if(ft_temp == 0)
		return;
	int reg = (int)(((cal[2] << 4) | (cal[1] >> 4)) & 0xfff);
	int delta = (ft_temp * 100 - (reg - 2822) * -66) / -66;
	int cdata = 0x800 - delta;
	if(cdata & ~0xfff)
		return;

	val = xos_io_read32(pdat->addr + THS_CALIB);
	val = (val & ~0xfffU) | ((uint32_t)cdata & 0xfffU);
	xos_io_write32(pdat->addr + THS_CALIB, val);
}

static int thermometer_f101_get(struct thermometer_t * thermometer, int * temperature)
{
	struct thermometer_f101_pdata_t * pdat = (struct thermometer_f101_pdata_t *)thermometer->priv;
	ktime_t timeout = ktime_add_ms(ktime_get(), 100);
	uint32_t val;

	xos_io_write32(pdat->addr + THS_DATA_INTS, 1 << 0);
	do {
		val = xos_io_read32(pdat->addr + THS_DATA_INTS) & (1 << 0);
	} while((val == 0) && ktime_before(ktime_get(), timeout));

	if(val == 0)
		return 0;
	val = xos_io_read32(pdat->addr + THS_DATA) & 0xfff;
	if(val == 0)
		return 0;
	if(temperature)
	{
		if(val > 1840)
			*temperature = (2822 - (int)val) * 10000 / 151;
		else
			*temperature = (2835 - (int)val) * 25000 / 383;
	}

	return 1;
}

static struct device_t * thermometer_f101_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct thermometer_f101_pdata_t * pdat;
	struct thermometer_t * t;
	struct device_t * dev;
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct thermometer_f101_pdata_t));
	if(!pdat)
		return NULL;

	t = xos_mem_malloc(sizeof(struct thermometer_t));
	if(!t)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = dt_read_address(n);
	pdat->clk = xos_strdup(clk);
	pdat->reset = dt_read_int(n, "reset", -1);

	t->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	t->get = thermometer_f101_get;
	t->priv = pdat;

	clk_enable(pdat->clk);
	if(pdat->reset >= 0)
	{
		reset_assert(pdat->reset);
		udelay(1);
		reset_deassert(pdat->reset);
		udelay(1);
	}
	xos_io_write32(pdat->addr + THS_CTRL, 0x01df002f);
	xos_io_write32(pdat->addr + THS_PER, 5 << 12);
	xos_io_write32(pdat->addr + THS_FILTER, (1 << 2) | 0x3);
	xos_io_write32(pdat->addr + THS_DATA_INTS, 1 << 0);
	xos_io_write32(pdat->addr + THS_EN, 1 << 0);
	thermometer_f101_calibrate(pdat);

	if(!(dev = register_thermometer(t, drv)))
	{
		xos_io_write32(pdat->addr + THS_EN, 0);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(t->name);
		xos_mem_free(t->priv);
		xos_mem_free(t);
		return NULL;
	}
	return dev;
}

static void thermometer_f101_remove(struct device_t * dev)
{
	struct thermometer_t * t = (struct thermometer_t *)dev->priv;
	struct thermometer_f101_pdata_t * pdat = (struct thermometer_f101_pdata_t *)t->priv;

	if(t)
	{
		unregister_thermometer(t);
		xos_io_write32(pdat->addr + THS_EN, 0);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(t->name);
		xos_mem_free(t->priv);
		xos_mem_free(t);
	}
}

static void thermometer_f101_suspend(struct device_t * dev)
{
}

static void thermometer_f101_resume(struct device_t * dev)
{
}

static struct driver_t thermometer_f101 = {
	.name		= "thermometer-f101",
	.probe		= thermometer_f101_probe,
	.remove		= thermometer_f101_remove,
	.suspend	= thermometer_f101_suspend,
	.resume		= thermometer_f101_resume,
};

static void thermometer_f101_driver_init(void)
{
	register_driver(&thermometer_f101);
}

static void thermometer_f101_driver_exit(void)
{
	unregister_driver(&thermometer_f101);
}

driver_initcall(thermometer_f101_driver_init);
driver_exitcall(thermometer_f101_driver_exit);
