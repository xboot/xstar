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
#include <v821/reg-ccu.h>

struct clk_v821_pll_pdata_t {
	io_addr_t addr;
	char * parent;
	int channel;
};

static void clk_v821_pll_set_parent(struct clk_t * clk, const char * pname)
{
}

static const char * clk_v821_pll_get_parent(struct clk_t * clk)
{
	struct clk_v821_pll_pdata_t * pdat = (struct clk_v821_pll_pdata_t *)clk->priv;
	return pdat->parent;
}

static void clk_v821_pll_set_enable(struct clk_t * clk, int enable)
{
}

static int clk_v821_pll_get_enable(struct clk_t * clk)
{
	return TRUE;
}

static void clk_v821_pll_set_rate(struct clk_t * clk, uint64_t prate, uint64_t rate)
{
}

static uint64_t clk_v821_pll_get_rate(struct clk_t * clk, uint64_t prate)
{
	struct clk_v821_pll_pdata_t * pdat = (struct clk_v821_pll_pdata_t *)clk->priv;
	uint32_t r, n, m;
	uint64_t rate = 0;

	switch(pdat->channel)
	{
	case 0:
		r = xos_io_read32(pdat->addr + CCU_AON_PLL_CPU_CTRL_REG);
		n = (r >> 8) & 0xff;
		m = (r >> 2) & 0x3;
		rate = (uint64_t)(prate * (n + 1) / (m > 1 ? 4 : m + 1));
		break;

	case 1:
		r = xos_io_read32(pdat->addr + CCU_AON_PLL_PERI_CTRL0_REG);
		n = (r >> 8) & 0xff;
		m = (r >> 0) & 0x7;
		rate = (uint64_t)(prate * 2 * (n + 1)) / (m + 1);
		break;

	case 2:
		r = xos_io_read32(pdat->addr + CCU_AON_PLL_VIDEO_CTRL_REG);
		n = (r >> 8) & 0xff;
		m = (r >> 1) & 0x3;
		rate = (uint64_t)(prate * (n + 1)) / (m + 1);
		break;

	case 3:
		r = xos_io_read32(pdat->addr + CCU_AON_PLL_CSI_CTRL_REG);
		n = (r >> 8) & 0xff;
		m = (r >> 1) & 0x3;
		rate = (uint64_t)(prate * (n + 1)) / (1 << m);
		break;

	case 4:
		r = xos_io_read32(pdat->addr + CCU_AON_PLL_AUDIO_CTRL_REG);
		n = (r >> 8) & 0xff;
		m = (r >> 1) & 0x3;
		rate = (uint64_t)(prate * (n + 1)) / (m + 1);
		break;

	case 5:
		r = xos_io_read32(pdat->addr + CCU_AON_PLL_DDR_CTRL_REG);
		n = (r >> 8) & 0xff;
		m = (r >> 1) & 0x3;
		rate = (uint64_t)(prate * (n + 1)) / (m + 1);
		break;

	default:
		break;
	}

	return rate;
}

static struct device_t * clk_v821_pll_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct clk_v821_pll_pdata_t * pdat;
	struct clk_t * clk;
	struct device_t * dev;
	struct dtnode_t o;
	char * parent = dt_read_string(n, "parent", NULL);
	char * name = dt_read_string(n, "name", NULL);
	int channel = dt_read_int(n, "channel", -1);

	if(channel < 0 || channel > 5)
		return NULL;

	if(!parent || !name)
		return NULL;

	if(!search_clk(parent) || search_clk(name))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct clk_v821_pll_pdata_t));
	if(!pdat)
		return NULL;

	clk = xos_mem_malloc(sizeof(struct clk_t));
	if(!clk)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = V821_CCU_AON_BASE;
	pdat->parent = xos_strdup(parent);
	pdat->channel = channel;

	clk->name = xos_strdup(name);
	clk->count = 0;
	clk->set_parent = clk_v821_pll_set_parent;
	clk->get_parent = clk_v821_pll_get_parent;
	clk->set_enable = clk_v821_pll_set_enable;
	clk->get_enable = clk_v821_pll_get_enable;
	clk->set_rate = clk_v821_pll_set_rate;
	clk->get_rate = clk_v821_pll_get_rate;
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

static void clk_v821_pll_remove(struct device_t * dev)
{
	struct clk_t * clk = (struct clk_t *)dev->priv;
	struct clk_v821_pll_pdata_t * pdat = (struct clk_v821_pll_pdata_t *)clk->priv;

	if(clk)
	{
		unregister_clk(clk);
		xos_mem_free(pdat->parent);
		xos_mem_free(clk->name);
		xos_mem_free(clk->priv);
		xos_mem_free(clk);
	}
}

static void clk_v821_pll_suspend(struct device_t * dev)
{
}

static void clk_v821_pll_resume(struct device_t * dev)
{
}

static struct driver_t clk_v821_pll = {
	.name		= "clk-v821-pll",
	.probe		= clk_v821_pll_probe,
	.remove		= clk_v821_pll_remove,
	.suspend	= clk_v821_pll_suspend,
	.resume		= clk_v821_pll_resume,
};

static void clk_v821_pll_driver_init(void)
{
	register_driver(&clk_v821_pll);
}

static void clk_v821_pll_driver_exit(void)
{
	unregister_driver(&clk_v821_pll);
}

driver_initcall(clk_v821_pll_driver_init);
driver_exitcall(clk_v821_pll_driver_exit);
