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
	GPADC_SR_CON		= 0x00,
	GPADC_CTRL			= 0x04,
	GPADC_CS_EN			= 0x08,
	GPADC_FIFO_INTC		= 0x0c,
	GPADC_FIFO_INTS		= 0x10,
	GPADC_FIFO_DATA		= 0x14,
	GPADC_CDATA			= 0x18,
	GPADC_DATAL_INTC	= 0x20,
	GPADC_DATAH_INTC	= 0x24,
	GPADC_DATA_INTC		= 0x28,
	GPADC_DATAL_INTS	= 0x30,
	GPADC_DATAH_INTS	= 0x34,
	GPADC_DATA_INTS		= 0x38,
	GPADC_CH0_CMP_DATA	= 0x40,
	GPADC_CH1_CMP_DATA	= 0x44,
	GPADC_CH0_DATA		= 0x80,
	GPADC_CH1_DATA		= 0x84,
};

struct adc_f133_pdata_t
{
	io_addr_t virt;
	char * clk;
	int reset;
};

static int32_t adc_f133_read(struct adc_t * adc, int channel)
{
	struct adc_f133_pdata_t * pdat = (struct adc_f133_pdata_t *)adc->priv;
	uint32_t val;

	val = xos_io_read32(pdat->virt + GPADC_CS_EN);
	val |= (1 << channel);
	xos_io_write32(pdat->virt + GPADC_CS_EN, val);

	val = xos_io_read32(pdat->virt + GPADC_CTRL);
	val &= ~(0x3 << 18);
	val |= (0x2 << 18);
	xos_io_write32(pdat->virt + GPADC_CTRL, val);

	xos_io_write32(pdat->virt + GPADC_DATA_INTC, 0);
	xos_io_write32(pdat->virt + GPADC_DATA_INTS, 1);

	val = xos_io_read32(pdat->virt + GPADC_CTRL);
	val |= (1 << 16);
	xos_io_write32(pdat->virt + GPADC_CTRL, val);

	while((xos_io_read32(pdat->virt + GPADC_DATA_INTS) & (1 << channel)) == 0);
	return xos_io_read32(pdat->virt + GPADC_CH0_DATA + (channel << 2));
}

static struct device_t * adc_f133_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct adc_f133_pdata_t * pdat;
	struct adc_t * adc;
	struct device_t * dev;
	io_addr_t virt = dt_read_address(n);
	char * clk = dt_read_string(n, "clock-name", NULL);

	if(!search_clk(clk))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct adc_f133_pdata_t));
	if(!pdat)
		return NULL;

	adc = xos_mem_malloc(sizeof(struct adc_t));
	if(!adc)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->virt = virt;
	pdat->clk = xos_strdup(clk);
	pdat->reset = dt_read_int(n, "reset", -1);

	adc->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	adc->vreference = dt_read_int(n, "reference-voltage", 1800000);
	adc->resolution = 12;
	adc->nchannel = 1;
	adc->read = adc_f133_read;
	adc->priv = pdat;

	clk_enable(pdat->clk);
	if(pdat->reset >= 0)
	{
		reset_assert(pdat->reset);
		udelay(1);
		reset_deassert(pdat->reset);
		udelay(1);
	}
	xos_io_write32(pdat->virt + GPADC_SR_CON, 0x01df002f);
	xos_io_write32(pdat->virt + GPADC_CTRL, 0);

	if(!(dev = register_adc(adc, drv)))
	{
		xos_io_write32(pdat->virt + GPADC_CTRL, 0);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(adc->name);
		xos_mem_free(adc->priv);
		xos_mem_free(adc);
		return NULL;
	}
	return dev;
}

static void adc_f133_remove(struct device_t * dev)
{
	struct adc_t * adc = (struct adc_t *)dev->priv;
	struct adc_f133_pdata_t * pdat = (struct adc_f133_pdata_t *)adc->priv;

	if(adc)
	{
		unregister_adc(adc);
		xos_io_write32(pdat->virt + GPADC_CTRL, 0);
		clk_disable(pdat->clk);
		xos_mem_free(pdat->clk);
		free_device_name(adc->name);
		xos_mem_free(adc->priv);
		xos_mem_free(adc);
	}
}

static void adc_f133_suspend(struct device_t * dev)
{
}

static void adc_f133_resume(struct device_t * dev)
{
}

static struct driver_t adc_f133 = {
	.name		= "adc-f133",
	.probe		= adc_f133_probe,
	.remove		= adc_f133_remove,
	.suspend	= adc_f133_suspend,
	.resume		= adc_f133_resume,
};

static void adc_f133_driver_init(void)
{
	register_driver(&adc_f133);
}

static void adc_f133_driver_exit(void)
{
	unregister_driver(&adc_f133);
}

driver_initcall(adc_f133_driver_init);
driver_exitcall(adc_f133_driver_exit);
