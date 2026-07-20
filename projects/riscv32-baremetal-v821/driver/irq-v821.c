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
#include <riscv32.h>

#define PLIC_PRIO(x)		(0x0000 + (x) * 4)
#define PLIC_IP(x)			(0x1000 + (x) * 4)
#define PLIC_MIE(x)			(0x2000 + (x) * 4)
#define PLIC_SIE(x)			(0x2080 + (x) * 4)
#define PLIC_CTRL			(0x1ffffc)
#define PLIC_MTH			(0x200000)
#define PLIC_MCLAIM			(0x200004)
#define PLIC_STH			(0x201000)
#define PLIC_SCLAIM			(0x201004)

struct irq_v821_pdata_t {
	io_addr_t addr;
	int base;
	int nirq;
};

static void irq_v821_enable(struct irqchip_t * chip, int offset)
{
	struct irq_v821_pdata_t * pdat = (struct irq_v821_pdata_t *)chip->priv;
	int irq = chip->base + offset;
	uint32_t val = xos_io_read32(pdat->addr + PLIC_MIE(irq >> 5));
	val |= (1 << (irq & 0x1f));
	xos_io_write32(pdat->addr + PLIC_MIE(irq >> 5), val);
}

static void irq_v821_disable(struct irqchip_t * chip, int offset)
{
	struct irq_v821_pdata_t * pdat = (struct irq_v821_pdata_t *)chip->priv;
	int irq = chip->base + offset;
	uint32_t val = xos_io_read32(pdat->addr + PLIC_MIE(irq >> 5));
	val &= ~(1 << (irq & 0x1f));
	xos_io_write32(pdat->addr + PLIC_MIE(irq >> 5), val);
}

static void irq_v821_settype(struct irqchip_t * chip, int offset, enum irq_type_t type)
{
}

static void irq_v821_dispatch(struct irqchip_t * chip)
{
	struct irq_v821_pdata_t * pdat = (struct irq_v821_pdata_t *)chip->priv;
	uint32_t irq = xos_io_read32(pdat->addr + PLIC_MCLAIM);
	int offset = irq + chip->base;

	if((offset > 0) && (offset < chip->nirq))
	{
		(chip->handler[offset].func)(chip->handler[offset].data);
		xos_io_write32(pdat->addr + PLIC_MCLAIM, offset);
	}
}

static void plic_init(struct irq_v821_pdata_t * pdat)
{
	uint32_t val;
	int i;

	/*
	 * Disable all interrupts
	 */
	for(i = 0; i < pdat->nirq; i++)
	{
		val = xos_io_read32(pdat->addr + PLIC_MIE(i >> 5));
		val &= ~(1 << (i & 0x1f));
		xos_io_write32(pdat->addr + PLIC_MIE(i >> 5), val);
	}
	/*
	 * Set all source priorities to one
	 */
	for(i = 0; i < pdat->nirq; i++)
	{
		xos_io_write32(pdat->addr + PLIC_PRIO(i), 1);
	}
	/*
	 * Clear pending bits
	 */
	for(i = 0; i < pdat->nirq; i++)
	{
		xos_io_write32(pdat->addr + PLIC_MCLAIM, i);
	}
}

static struct device_t * irq_v821_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct irq_v821_pdata_t * pdat;
	struct irqchip_t * chip;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	int base = dt_read_int(n, "interrupt-base", -1);
	int nirq = dt_read_int(n, "interrupt-count", -1);

	if((base < 0) || (nirq <= 0))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct irq_v821_pdata_t));
	if(!pdat)
		return NULL;

	chip = xos_mem_malloc(sizeof(struct irqchip_t));
	if(!chip)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->base = base;
	pdat->nirq = nirq;

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->nirq = pdat->nirq;
	chip->handler = xos_mem_malloc(sizeof(struct irq_handler_t) * pdat->nirq);
	chip->enable = irq_v821_enable;
	chip->disable = irq_v821_disable;
	chip->settype = irq_v821_settype;
	chip->dispatch = irq_v821_dispatch;
	chip->priv = pdat;

	plic_init(pdat);
	csr_set(mie, MIE_MEIE);
	csr_set(mstatus, MSTATUS_MIE);

	if(!(dev = register_irqchip(chip, drv)))
	{
		free_device_name(chip->name);
		xos_mem_free(chip->handler);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
		return NULL;
	}
	return dev;
}

static void irq_v821_remove(struct device_t * dev)
{
	struct irqchip_t * chip = (struct irqchip_t *)dev->priv;

	if(chip)
	{
		unregister_irqchip(chip);
		free_device_name(chip->name);
		xos_mem_free(chip->handler);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
	}
}

static void irq_v821_suspend(struct device_t * dev)
{
}

static void irq_v821_resume(struct device_t * dev)
{
}

static struct driver_t irq_v821 = {
	.name		= "irq-v821",
	.probe		= irq_v821_probe,
	.remove		= irq_v821_remove,
	.suspend	= irq_v821_suspend,
	.resume		= irq_v821_resume,
};

static void irq_v821_driver_init(void)
{
	register_driver(&irq_v821);
}

static void irq_v821_driver_exit(void)
{
	unregister_driver(&irq_v821);
}

driver_initcall(irq_v821_driver_init);
driver_exitcall(irq_v821_driver_exit);
