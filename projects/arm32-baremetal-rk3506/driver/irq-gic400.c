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
#include <arm32.h>

enum {
	DIST_CTRL 			= 0x1000,
	DIST_CTR 			= 0x1004,
	DIST_ENABLE_SET 	= 0x1100,
	DIST_ENABLE_CLEAR 	= 0x1180,
	DIST_PENDING_SET 	= 0x1200,
	DIST_PENDING_CLEAR	= 0x1280,
	DIST_ACTIVE_BIT		= 0x1300,
	DIST_PRI			= 0x1400,
	DIST_TARGET			= 0x1800,
	DIST_CONFIG			= 0x1c00,
	DIST_SOFTINT		= 0x1f00,

	CPU_CTRL 			= 0x2000,
	CPU_PRIMASK 		= 0x2004,
	CPU_BINPOINT 		= 0x2008,
	CPU_INTACK 			= 0x200c,
	CPU_EOI 			= 0x2010,
	CPU_RUNNINGPRI 		= 0x2014,
	CPU_HIGHPRI 		= 0x2018,
};

struct irq_gic400_pdata_t {
	io_addr_t addr;
	int base;
	int nirq;
};

static void irq_gic400_enable(struct irqchip_t * chip, int offset)
{
	struct irq_gic400_pdata_t * pdat = (struct irq_gic400_pdata_t *)chip->priv;
	int irq = chip->base + offset;
	xos_io_write32(pdat->addr + DIST_ENABLE_SET + (irq / 32) * 4, 1 << (irq % 32));
}

static void irq_gic400_disable(struct irqchip_t * chip, int offset)
{
	struct irq_gic400_pdata_t * pdat = (struct irq_gic400_pdata_t *)chip->priv;
	int irq = chip->base + offset;
	xos_io_write32(pdat->addr + DIST_ENABLE_CLEAR + (irq / 32) * 4, 1 << (irq % 32));
}

static void irq_gic400_settype(struct irqchip_t * chip, int offset, enum irq_type_t type)
{
}

static void irq_gic400_dispatch(struct irqchip_t * chip)
{
	struct irq_gic400_pdata_t * pdat = (struct irq_gic400_pdata_t *)chip->priv;
	int irq = xos_io_read32(pdat->addr + CPU_INTACK) & 0x3ff;
	int offset = irq - chip->base;

	if((offset >= 0) && (offset < chip->nirq))
	{
		(chip->handler[offset].func)(chip->handler[offset].data);
		xos_io_write32(pdat->addr + CPU_EOI, irq);
	}
}

static void gic400_dist_init(io_addr_t addr)
{
	uint32_t gic_irqs;
	uint32_t cpumask;
	int i;

	xos_io_write32(addr + DIST_CTRL, 0x0);

	/*
	 * Find out how many interrupts are supported.
	 * The GIC only supports up to 1020 interrupt sources.
	 */
	gic_irqs = xos_io_read32(addr + DIST_CTR) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;
	if(gic_irqs > 1020)
		gic_irqs = 1020;

	/*
	 * Set all global interrupts to this CPU only.
	 */
	cpumask = 1 << smp_processor_id();
	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	for(i = 32; i < gic_irqs; i += 4)
		xos_io_write32(addr + DIST_TARGET + i * 4 / 4, cpumask);

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for(i = 32; i < gic_irqs; i += 16)
		xos_io_write32(addr + DIST_CONFIG + i * 4 / 16, 0);

	/*
	 * Set priority on all global interrupts.
	 */
	for(i = 32; i < gic_irqs; i += 4)
		xos_io_write32(addr + DIST_PRI + i * 4 / 4, 0xa0a0a0a0);

	/*
	 * Disable all interrupts, leave the SGI and PPI alone
	 * as these enables are banked registers.
	 */
	for(i = 32; i < gic_irqs; i += 32)
		xos_io_write32(addr + DIST_ENABLE_CLEAR + i * 4 / 32, 0xffffffff);

	xos_io_write32(addr + DIST_CTRL, 0x1);
}

static void gic400_cpu_init(io_addr_t addr)
{
	int i;

	/*
	 * Deal with the banked SGI and PPI interrupts - enable all
	 * SGI interrupts, ensure all PPI interrupts are disabled.
	 */
	xos_io_write32(addr + DIST_ENABLE_CLEAR, 0xffff0000);
	xos_io_write32(addr + DIST_ENABLE_SET, 0x0000ffff);

	/*
	 * Set priority on SGI and PPI interrupts
	 */
	for(i = 0; i < 32; i += 4)
		xos_io_write32(addr + DIST_PRI + i * 4 / 4, 0xa0a0a0a0);

	xos_io_write32(addr + CPU_PRIMASK, 0xf0);
	xos_io_write32(addr + CPU_CTRL, 0x1);
}

static struct device_t * irq_gic400_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct irq_gic400_pdata_t * pdat;
	struct irqchip_t * chip;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	int base = dt_read_int(n, "interrupt-base", -1);
	int nirq = dt_read_int(n, "interrupt-count", -1);

	if((base < 0) || (nirq <= 0))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct irq_gic400_pdata_t));
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
	chip->enable = irq_gic400_enable;
	chip->disable = irq_gic400_disable;
	chip->settype = irq_gic400_settype;
	chip->dispatch = irq_gic400_dispatch;
	chip->priv = pdat;

	gic400_dist_init(pdat->addr);
	gic400_cpu_init(pdat->addr);
	arm32_interrupt_enable();

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

static void irq_gic400_remove(struct device_t * dev)
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

static void irq_gic400_suspend(struct device_t * dev)
{
}

static void irq_gic400_resume(struct device_t * dev)
{
}

static struct driver_t irq_gic400 = {
	.name		= "irq-gic400",
	.probe		= irq_gic400_probe,
	.remove		= irq_gic400_remove,
	.suspend	= irq_gic400_suspend,
	.resume		= irq_gic400_resume,
};

static void irq_gic400_driver_init(void)
{
	register_driver(&irq_gic400);
}

static void irq_gic400_driver_exit(void)
{
	unregister_driver(&irq_gic400);
}

driver_initcall(irq_gic400_driver_init);
driver_exitcall(irq_gic400_driver_exit);
