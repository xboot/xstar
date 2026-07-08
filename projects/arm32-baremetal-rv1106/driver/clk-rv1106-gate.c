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

struct clk_rv1106_gate_pdata_t {
	io_addr_t addr;
	char * parent;
	int shift;
	int invert;
};

static void clk_rv1106_gate_set_parent(struct clk_t * clk, const char * pname)
{
}

static const char * clk_rv1106_gate_get_parent(struct clk_t * clk)
{
	struct clk_rv1106_gate_pdata_t * pdat = (struct clk_rv1106_gate_pdata_t *)clk->priv;
	return pdat->parent;
}

static void clk_rv1106_gate_set_enable(struct clk_t * clk, int enable)
{
	struct clk_rv1106_gate_pdata_t * pdat = (struct clk_rv1106_gate_pdata_t *)clk->priv;
	uint32_t val;

	if(enable)
		val = (pdat->invert ? 0x0 : 0x1) << pdat->shift;
	else
		val = (pdat->invert ? 0x1 : 0x0) << pdat->shift;
	val |= 0x1 << (pdat->shift + 16);
	xos_io_write32(pdat->addr, val);
}

static int clk_rv1106_gate_get_enable(struct clk_t * clk)
{
	struct clk_rv1106_gate_pdata_t * pdat = (struct clk_rv1106_gate_pdata_t *)clk->priv;

	if(xos_io_read32(pdat->addr) & (0x1 << pdat->shift))
		return pdat->invert ? FALSE : TRUE;
	return pdat->invert ? TRUE : FALSE;
}

static void clk_rv1106_gate_set_rate(struct clk_t * clk, uint64_t prate, uint64_t rate)
{
}

static uint64_t clk_rv1106_gate_get_rate(struct clk_t * clk, uint64_t prate)
{
	return prate;
}

static struct device_t * clk_rv1106_gate_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clk_rv1106_gate_pdata_t * pdat;
	struct clk_t * clk;
	struct device_t * dev;
	struct dtnode_t o;
	io_addr_t addr = dt_read_address(n);
	char * parent = dt_read_string(n, "parent", NULL);
	char * name = dt_read_string(n, "name", NULL);
	int shift = dt_read_int(n, "shift", -1);

	if(!parent || !name || (shift < 0))
		return NULL;

	if(!search_clk(parent) || search_clk(name))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct clk_rv1106_gate_pdata_t));
	if(!pdat)
		return NULL;

	clk = xos_mem_malloc(sizeof(struct clk_t));
	if(!clk)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->parent = xos_strdup(parent);
	pdat->shift = shift;
	pdat->invert = dt_read_bool(n, "invert", 0);

	clk->name = xos_strdup(name);
	clk->count = 0;
	clk->set_parent = clk_rv1106_gate_set_parent;
	clk->get_parent = clk_rv1106_gate_get_parent;
	clk->set_enable = clk_rv1106_gate_set_enable;
	clk->get_enable = clk_rv1106_gate_get_enable;
	clk->set_rate = clk_rv1106_gate_set_rate;
	clk->get_rate = clk_rv1106_gate_get_rate;
	clk->priv = pdat;

	if(!(dev = register_clk(clk, drv)))
	{
		xos_mem_free(pdat->parent);
		xos_mem_free(clk->name);
		xos_mem_free(clk->priv);
		xos_mem_free(clk);
		return NULL;
	}
	if(dt_read_object(n, "default", &o))
	{
		char * c = clk->name;
		char * p;
		uint64_t r;
		int e;

		if((p = dt_read_string(&o, "parent", NULL)) && search_clk(p))
			clk_set_parent(c, p);
		if((r = (uint64_t)dt_read_long(&o, "rate", 0)) > 0)
			clk_set_rate(c, r);
		if((e = dt_read_bool(&o, "enable", -1)) != -1)
		{
			if(e > 0)
				clk_enable(c);
			else
				clk_disable(c);
		}
	}
	return dev;
}

static void clk_rv1106_gate_remove(struct device_t * dev)
{
	struct clk_t * clk = (struct clk_t *)dev->priv;
	struct clk_rv1106_gate_pdata_t * pdat = (struct clk_rv1106_gate_pdata_t *)clk->priv;

	if(clk)
	{
		unregister_clk(clk);
		xos_mem_free(pdat->parent);
		xos_mem_free(clk->name);
		xos_mem_free(clk->priv);
		xos_mem_free(clk);
	}
}

static void clk_rv1106_gate_suspend(struct device_t * dev)
{
}

static void clk_rv1106_gate_resume(struct device_t * dev)
{
}

static struct driver_t clk_rv1106_gate = {
	.name		= "clk-rv1106-gate",
	.probe		= clk_rv1106_gate_probe,
	.remove		= clk_rv1106_gate_remove,
	.suspend	= clk_rv1106_gate_suspend,
	.resume		= clk_rv1106_gate_resume,
};

static void clk_rv1106_gate_driver_init(void)
{
	register_driver(&clk_rv1106_gate);
}

static void clk_rv1106_gate_driver_exit(void)
{
	unregister_driver(&clk_rv1106_gate);
}

driver_initcall(clk_rv1106_gate_driver_init);
driver_exitcall(clk_rv1106_gate_driver_exit);
