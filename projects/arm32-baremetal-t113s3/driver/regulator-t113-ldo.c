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

struct regulator_t113_ldo_pdata_t {
	io_addr_t addr;
	int channel;
};

static void regulator_t113_ldo_set_parent(struct regulator_t * supply, const char * pname)
{
}

static const char * regulator_t113_ldo_get_parent(struct regulator_t * supply)
{
	return NULL;
}

static void regulator_t113_ldo_set_enable(struct regulator_t * supply, int enable)
{
}

static int regulator_t113_ldo_get_enable(struct regulator_t * supply)
{
	return TRUE;
}

static uint8_t t113_ldo_vol_to_reg(int vol, int step, int min, int max)
{
	int reg;

	if(vol < min)
		vol = min;
	else if(vol > max)
		vol = max;

	reg = (vol - min + step - 1) / step;
	return (uint8_t)(reg & 0xff);
}

static int t113_ldo_reg_to_vol(uint8_t reg, int step, int min, int max)
{
	int vol = (int)reg * step + min;

	if(vol < min)
		vol = min;
	else if(vol > max)
		vol = max;
	return vol;
}

static void regulator_t113_ldo_set_voltage(struct regulator_t * supply, int voltage)
{
	struct regulator_t113_ldo_pdata_t * pdat = (struct regulator_t113_ldo_pdata_t *)supply->priv;
	uint32_t val;
	uint8_t v;

	switch(pdat->channel)
	{
	/* LDOA - 1.593V ~ 2.013V, 0.0135V/step, 0.2A */
	case 0:
		v = t113_ldo_vol_to_reg(voltage, 13500, 1593000, 2013000);
		val = xos_io_read32(pdat->addr);
		val &= ~(0xff << 0);
		val |= (v & 0x1f) << 0;
		xos_io_write32(pdat->addr, val);
		break;

	/* LDOB - 1.167V ~ 2.013V, 0.0135V/step, 0.4A */
	case 1:
		v = t113_ldo_vol_to_reg(voltage, 13500, 1167000, 2013000);
		val = xos_io_read32(pdat->addr);
		val &= ~(0xff << 8);
		val |= (v & 0x3f) << 8;
		xos_io_write32(pdat->addr, val);
		break;

	default:
		break;
	}
}

static int regulator_t113_ldo_get_voltage(struct regulator_t * supply)
{
	struct regulator_t113_ldo_pdata_t * pdat = (struct regulator_t113_ldo_pdata_t *)supply->priv;
	int voltage;
	uint8_t v;

	switch(pdat->channel)
	{
	/* LDOA - 1.593V ~ 2.013V, 0.0135V/step, 0.2A */
	case 0:
		v = (xos_io_read32(pdat->addr) >> 0) & 0x1f;
		voltage = t113_ldo_reg_to_vol(v, 13500, 1593000, 2013000);
		break;

	/* LDOB - 1.167V ~ 2.013V, 0.0135V/step, 0.4A */
	case 1:
		v = (xos_io_read32(pdat->addr) >> 8) & 0x3f;
		voltage = t113_ldo_reg_to_vol(v, 13500, 1167000, 2013000);
		break;

	default:
		voltage = 0;
		break;
	}
	return voltage;
}

static struct device_t * regulator_t113_ldo_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct regulator_t113_ldo_pdata_t * pdat;
	struct regulator_t * supply;
	struct device_t * dev;
	struct dtnode_t o;
	io_addr_t addr = dt_read_address(n);
	char * name = dt_read_string(n, "name", NULL);
	int channel = dt_read_int(n, "channel", -1);

	if(!name || search_regulator(name))
		return NULL;

	if(channel < 0 || channel > 1)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct regulator_t113_ldo_pdata_t));
	if(!pdat)
		return NULL;

	supply = xos_mem_malloc(sizeof(struct regulator_t));
	if(!supply)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->channel = channel;

	supply->name = xos_strdup(name);
	supply->count = 0;
	supply->set_parent = regulator_t113_ldo_set_parent;
	supply->get_parent = regulator_t113_ldo_get_parent;
	supply->set_enable = regulator_t113_ldo_set_enable;
	supply->get_enable = regulator_t113_ldo_get_enable;
	supply->set_voltage = regulator_t113_ldo_set_voltage;
	supply->get_voltage = regulator_t113_ldo_get_voltage;
	supply->priv = pdat;

	if(!(dev = register_regulator(supply, drv)))
	{
		xos_mem_free(supply->name);
		xos_mem_free(supply->priv);
		xos_mem_free(supply);
		return NULL;
	}
	if(dt_read_object(n, "default", &o))
	{
		char * s = supply->name;
		char * p;
		int v;
		int e;

		if((p = dt_read_string(&o, "parent", NULL)) && search_regulator(p))
			regulator_set_parent(s, p);
		if((v = dt_read_int(&o, "voltage", -1)) >= 0)
			regulator_set_voltage(s, v);
		if((e = dt_read_bool(&o, "enable", -1)) != -1)
		{
			if(e > 0)
				regulator_enable(s);
			else
				regulator_disable(s);
		}
	}
	return dev;
}

static void regulator_t113_ldo_remove(struct device_t * dev)
{
	struct regulator_t * supply = (struct regulator_t *)dev->priv;

	if(supply)
	{
		unregister_regulator(supply);
		xos_mem_free(supply->name);
		xos_mem_free(supply->priv);
		xos_mem_free(supply);
	}
}

static void regulator_t113_ldo_suspend(struct device_t * dev)
{
}

static void regulator_t113_ldo_resume(struct device_t * dev)
{
}

static struct driver_t regulator_t113_ldo = {
	.name		= "regulator-t113-ldo",
	.probe		= regulator_t113_ldo_probe,
	.remove		= regulator_t113_ldo_remove,
	.suspend	= regulator_t113_ldo_suspend,
	.resume		= regulator_t113_ldo_resume,
};

static void regulator_t113_ldo_driver_init(void)
{
	register_driver(&regulator_t113_ldo);
}

static void regulator_t113_ldo_driver_exit(void)
{
	unregister_driver(&regulator_t113_ldo);
}

driver_initcall(regulator_t113_ldo_driver_init);
driver_exitcall(regulator_t113_ldo_driver_exit);
