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
	GPIO_SWPORT_DR_L	= 0x00,
	GPIO_SWPORT_DR_H	= 0x04,
	GPIO_SWPORT_DDR_L	= 0x08,
	GPIO_SWPORT_DDR_H	= 0x0c,
	GPIO_INT_EN_L		= 0x10,
	GPIO_INT_EN_H		= 0x14,
	GPIO_INT_MASK_L		= 0x18,
	GPIO_INT_MASK_H		= 0x1c,
	GPIO_INT_TYPE_L		= 0x20,
	GPIO_INT_TYPE_H		= 0x24,
	GPIO_INT_POLARITY_L	= 0x28,
	GPIO_INT_POLARITY_H	= 0x2c,
	GPIO_INT_BOTHEDGE_L	= 0x30,
	GPIO_INT_BOTHEDGE_H	= 0x34,
	GPIO_DEBOUNCE_L		= 0x38,
	GPIO_DEBOUNCE_H		= 0x3c,
	GPIO_DBCLK_DIV_EN_L	= 0x40,
	GPIO_DBCLK_DIV_EN_H	= 0x44,
	GPIO_DBCLK_DIV_CON	= 0x48,
	GPIO_INT_STATUS		= 0x50,
	GPIO_INT_RAWSTATUS	= 0x58,
	GPIO_PORT_EOI_L		= 0x60,
	GPIO_PORT_EOI_H		= 0x64,
	GPIO_EXT_PORT		= 0x70,
	GPIO_VER_ID			= 0x78,
};

struct gpio_rk3506_pdata_t
{
	io_addr_t addr;
	io_addr_t ioc0;
	io_addr_t ioc1;
	io_addr_t ioc2;
	io_addr_t ioc3;
	io_addr_t ioc4;
	io_addr_t rmio;
	int base;
	int ngpio;
	int oirq;
};

static inline void gpio_write32(io_addr_t reg, uint32_t val)
{
	xos_io_write32(reg, (val & 0x0000ffff) | 0xffff0000);
	xos_io_write32(reg + 0x4, (val >> 16) | 0xffff0000);
}

static inline uint32_t gpio_read32(io_addr_t reg)
{
	return (xos_io_read32(reg + 0x4) << 16) | (xos_io_read32(reg) & 0x0000ffff);
}

/*
 * Rockchip matrix IO cfg = ((RK3506_RMIO_FUNC & 0xff) << 4) | 0x7
 */
static void gpio_rk3506_set_rmio(struct gpio_rk3506_pdata_t * pdat, int offset, int cfg)
{
	io_addr_t addr;
	uint32_t val;

	if((cfg & 0xf) == 0x7)
	{
		switch(offset)
		{
		case 0 ... 23:
			addr = pdat->rmio + 0x00000080 + ((offset - 0) << 2);
			break;
		case 41 ... 43:
			addr = pdat->rmio + 0x000000e0 + ((offset - 41) << 2);
			break;
		case 50 ... 51:
			addr = pdat->rmio + 0x000000ec + ((offset - 50) << 2);
			break;
		case 57 ... 59:
			addr = pdat->rmio + 0x000000f4 + ((offset - 57) << 2);
			break;
		default:
			return;
		}
		val = 0x00ff0000 | ((cfg >> 4) & 0xff);
		xos_io_write32(addr, val);
	}
}

static void gpio_rk3506_set_cfg(struct gpiochip_t * chip, int offset, int cfg)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	io_addr_t addr;
	uint32_t val;

	if(offset >= chip->ngpio)
		return;

	if(pdat->base < 32)
	{
		if(offset < 24)
			addr = pdat->ioc0 + 0x00000000 + ((offset >> 2) << 2);
		else if(offset == 24)
		{
			addr = pdat->ioc0 + 0x00000830;
			val = (((0x3 << 16) | (cfg & 0x3)) << 0);
			xos_io_write32(addr, val);
			return;
		}
		else
			return;
	}
	else if(pdat->base < 64)
		addr = pdat->ioc1 + 0x00000020 + ((offset >> 2) << 2);
	else if(pdat->base < 96)
		addr = pdat->ioc2 + 0x00000040 + ((offset >> 2) << 2);
	else if(pdat->base < 128)
		addr = pdat->ioc3 + 0x00000060 + ((offset >> 2) << 2);
	else if(pdat->base < 160)
		addr = pdat->ioc4 + 0x00000080 + ((offset >> 2) << 2);
	else
		return;
	val = (((0xf << 16) | (cfg & 0xf)) << ((offset & 0x3) << 2));
	xos_io_write32(addr, val);
	gpio_rk3506_set_rmio(pdat, offset, cfg);
}

static int gpio_rk3506_get_cfg(struct gpiochip_t * chip, int offset)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	io_addr_t addr;

	if(offset >= chip->ngpio)
		return 0;

	if(pdat->base < 32)
	{
		if(offset < 24)
			addr = pdat->ioc0 + 0x00000000 + ((offset >> 2) << 2);
		else if(offset == 24)
		{
			addr = pdat->ioc0 + 0x00000830;
			return (xos_io_read32(addr) >> 0) & 0x3;
		}
		else
			return 0;
	}
	else if(pdat->base < 64)
		addr = pdat->ioc1 + 0x00000020 + ((offset >> 2) << 2);
	else if(pdat->base < 96)
		addr = pdat->ioc2 + 0x00000040 + ((offset >> 2) << 2);
	else if(pdat->base < 128)
		addr = pdat->ioc3 + 0x00000060 + ((offset >> 2) << 2);
	else if(pdat->base < 160)
		addr = pdat->ioc4 + 0x00000080 + ((offset >> 2) << 2);
	else
		return 0;
	return (xos_io_read32(addr) >> ((offset & 0x3) << 2)) & 0xf;
}

static void gpio_rk3506_set_pull(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	io_addr_t addr;
	uint32_t val, v = 0;

	if(offset >= chip->ngpio)
		return;

	if(pdat->base < 32)
	{
		if(offset < 24)
			addr = pdat->ioc0 + 0x00000200 + ((offset >> 3) << 2);
		else if(offset == 24)
		{
			addr = pdat->ioc0 + 0x00000830;
			val = (((0x3 << 16) | (pull == GPIO_PULL_UP) ? 1 : 0) << 5);
			xos_io_write32(addr, val);
			return;
		}
		else
			return;
	}
	else if(pdat->base < 64)
		addr = pdat->ioc1 + 0x00000210 + ((offset >> 3) << 2);
	else if(pdat->base < 96)
		addr = pdat->ioc2 + 0x00000220 + ((offset >> 3) << 2);
	else if(pdat->base < 128)
		addr = pdat->ioc3 + 0x00000230 + ((offset >> 3) << 2);
	else if(pdat->base < 160)
		addr = pdat->ioc4 + 0x00000240 + ((offset >> 3) << 2);
	else
		return;
	switch(pull)
	{
	case GPIO_PULL_UP:
		v = 0x1;
		break;
	case GPIO_PULL_DOWN:
		v = 0x2;
		break;
	case GPIO_PULL_NONE:
		v = 0x0;
		break;
	default:
		break;
	}
	val = (((0x3 << 16) | (v & 0x3)) << ((offset & 0x7) << 1));
	xos_io_write32(addr, val);
}

static enum gpio_pull_t gpio_rk3506_get_pull(struct gpiochip_t * chip, int offset)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	io_addr_t addr;
	uint32_t v = 0;

	if(offset >= chip->ngpio)
		return GPIO_PULL_NONE;

	if(pdat->base < 32)
	{
		if(offset < 24)
			addr = pdat->ioc0 + 0x00000200 + ((offset >> 3) << 2);
		else if(offset == 24)
		{
			addr = pdat->ioc0 + 0x00000830;
			return (xos_io_read32(addr) & (1 << 5)) ? GPIO_PULL_UP : GPIO_PULL_NONE;
		}
		else
			return GPIO_PULL_NONE;
	}
	else if(pdat->base < 64)
		addr = pdat->ioc1 + 0x00000210 + ((offset >> 3) << 2);
	else if(pdat->base < 96)
		addr = pdat->ioc2 + 0x00000220 + ((offset >> 3) << 2);
	else if(pdat->base < 128)
		addr = pdat->ioc3 + 0x00000230 + ((offset >> 3) << 2);
	else if(pdat->base < 160)
		addr = pdat->ioc4 + 0x00000240 + ((offset >> 3) << 2);
	else
		return GPIO_PULL_NONE;
	v = (xos_io_read32(addr) >> ((offset & 0x7) << 1)) & 0x3;
	switch(v)
	{
	case 0x0:
		return GPIO_PULL_NONE;
	case 0x1:
		return GPIO_PULL_UP;
	case 0x2:
		return GPIO_PULL_DOWN;
	default:
		break;
	}
	return GPIO_PULL_NONE;
}

static void gpio_rk3506_set_drv(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	io_addr_t addr;
	uint32_t val, v = 0;

	if(offset >= chip->ngpio)
		return;

	if(pdat->base < 32)
	{
		if(offset < 24)
			addr = pdat->ioc0 + 0x00000100 + ((offset >> 1) << 2);
		else if(offset == 24)
		{
			addr = pdat->ioc0 + 0x00000830;
			switch(drv)
			{
			case GPIO_DRV_WEAK:
				v = 0x0;
				break;
			case GPIO_DRV_WEAKER:
				v = 0x1;
				break;
			case GPIO_DRV_STRONGER:
				v = 0x2;
				break;
			case GPIO_DRV_STRONG:
				v = 0x3;
				break;
			default:
				break;
			}
			val = (((0x3 << 16) | (v & 0x3)) << 3);
			xos_io_write32(addr, val);
			return;
		}
		else
			return;
	}
	else if(pdat->base < 64)
		addr = pdat->ioc1 + 0x00000140 + ((offset >> 1) << 2);
	else if(pdat->base < 96)
		addr = pdat->ioc2 + 0x00000180 + ((offset >> 1) << 2);
	else if(pdat->base < 128)
		addr = pdat->ioc3 + 0x000001c0 + ((offset >> 1) << 2);
	else if(pdat->base < 160)
		addr = pdat->ioc4 + 0x00000200 + ((offset >> 1) << 2);
	else
		return;
	switch(drv)
	{
	case GPIO_DRV_WEAK:
		v = 0x0;
		break;
	case GPIO_DRV_WEAKER:
		v = 0x5;
		break;
	case GPIO_DRV_STRONGER:
		v = 0xa;
		break;
	case GPIO_DRV_STRONG:
		v = 0xf;
		break;
	default:
		break;
	}
	val = (((0xff << 16) | (v & 0xff)) << ((offset & 0x1) << 3));
	xos_io_write32(addr, val);
}

static enum gpio_drv_t gpio_rk3506_get_drv(struct gpiochip_t * chip, int offset)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	io_addr_t addr;
	uint32_t v = 0;

	if(offset >= chip->ngpio)
		return GPIO_DRV_WEAK;

	if(pdat->base < 32)
	{
		if(offset < 24)
			addr = pdat->ioc0 + 0x00000100 + ((offset >> 1) << 2);
		else if(offset == 24)
		{
			addr = pdat->ioc0 + 0x00000830;
			v = (xos_io_read32(addr) >> 3) & 0x3;
			switch(v)
			{
			case 0x0:
				return GPIO_DRV_WEAK;
			case 0x1:
				return GPIO_DRV_WEAKER;
			case 0x2:
				return GPIO_DRV_STRONGER;
			case 0x3:
				return GPIO_DRV_STRONG;
			default:
				break;
			}
			return GPIO_DRV_WEAK;
		}
		else
			return GPIO_DRV_WEAK;
	}
	else if(pdat->base < 64)
		addr = pdat->ioc1 + 0x00000140 + ((offset >> 1) << 2);
	else if(pdat->base < 96)
		addr = pdat->ioc2 + 0x00000180 + ((offset >> 1) << 2);
	else if(pdat->base < 128)
		addr = pdat->ioc3 + 0x000001c0 + ((offset >> 1) << 2);
	else if(pdat->base < 160)
		addr = pdat->ioc4 + 0x00000200 + ((offset >> 1) << 2);
	else
		return GPIO_DRV_WEAK;
	v = (xos_io_read32(addr) >> ((offset & 0x1) << 3)) & 0xff;
	switch(v)
	{
	case 0x0 ... 0x3:
		return GPIO_DRV_WEAK;
	case 0x4 ... 0x7:
		return GPIO_DRV_WEAKER;
	case 0x8 ... 0xb:
		return GPIO_DRV_STRONGER;
	case 0xc ... 0xf:
		return GPIO_DRV_STRONG;
	default:
		break;
	}
	return GPIO_DRV_WEAK;
}

static void gpio_rk3506_set_rate(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate)
{
}

static enum gpio_rate_t gpio_rk3506_get_rate(struct gpiochip_t * chip, int offset)
{
	return GPIO_RATE_SLOW;
}

static void gpio_rk3506_set_dir(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	uint32_t val;

	if(offset >= chip->ngpio)
		return;

	switch(dir)
	{
	case GPIO_DIRECTION_INPUT:
		val = gpio_read32(pdat->addr + GPIO_SWPORT_DDR_L);
		val &= ~(1 << offset);
		gpio_write32(pdat->addr + GPIO_SWPORT_DDR_L, val);
		break;

	case GPIO_DIRECTION_OUTPUT:
		val = gpio_read32(pdat->addr + GPIO_SWPORT_DDR_L);
		val |= 1 << offset;
		gpio_write32(pdat->addr + GPIO_SWPORT_DDR_L, val);
		break;

	default:
		break;
	}
}

static enum gpio_direction_t gpio_rk3506_get_dir(struct gpiochip_t * chip, int offset)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	uint32_t val;

	if(offset >= chip->ngpio)
		return GPIO_DIRECTION_INPUT;

	val = gpio_read32(pdat->addr + GPIO_SWPORT_DDR_L);
	if((val & (1 << offset)))
		return GPIO_DIRECTION_OUTPUT;
	return GPIO_DIRECTION_INPUT;
}

static void gpio_rk3506_set_value(struct gpiochip_t * chip, int offset, int value)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	uint32_t val;

	if(offset >= chip->ngpio)
		return;

	val = gpio_read32(pdat->addr + GPIO_SWPORT_DR_L);
	if(value)
		val |= (1 << offset);
	else
		val &= ~(1 << offset);
	gpio_write32(pdat->addr + GPIO_SWPORT_DR_L, val);
}

static int gpio_rk3506_get_value(struct gpiochip_t * chip, int offset)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;
	uint32_t val;

	if(offset >= chip->ngpio)
		return 0;

	val = gpio_read32(pdat->addr + GPIO_EXT_PORT);
	return (val & (1 << offset)) ? 1 : 0;
}

static int gpio_rk3506_to_irq(struct gpiochip_t * chip, int offset)
{
	struct gpio_rk3506_pdata_t * pdat = (struct gpio_rk3506_pdata_t *)chip->priv;

	if((offset >= chip->ngpio) || (pdat->oirq < 0))
		return -1;
	return pdat->oirq + offset;
}

static struct device_t * gpio_rk3506_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct gpio_rk3506_pdata_t * pdat;
	struct gpiochip_t * chip;
	struct device_t * dev;
	io_addr_t addr = dt_read_address(n);
	int base = dt_read_int(n, "gpio-base", -1);
	int ngpio = dt_read_int(n, "gpio-count", -1);

	if((base < 0) || (ngpio <= 0))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct gpio_rk3506_pdata_t));
	if(!pdat)
		return NULL;

	chip = xos_mem_malloc(sizeof(struct gpiochip_t));
	if(!chip)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->addr = addr;
	pdat->ioc0 = 0xff950000;
	pdat->ioc1 = 0xff660000;
	pdat->ioc2 = 0xff4d8000;
	pdat->ioc3 = 0xff4d8000;
	pdat->ioc4 = 0xff4d8000;
	pdat->rmio = 0xff910000;

	pdat->base = base;
	pdat->ngpio = ngpio;
	pdat->oirq = dt_read_int(n, "interrupt-offset", -1);

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->ngpio = pdat->ngpio;
	chip->set_cfg = gpio_rk3506_set_cfg;
	chip->get_cfg = gpio_rk3506_get_cfg;
	chip->set_pull = gpio_rk3506_set_pull;
	chip->get_pull = gpio_rk3506_get_pull;
	chip->set_drv = gpio_rk3506_set_drv;
	chip->get_drv = gpio_rk3506_get_drv;
	chip->set_rate = gpio_rk3506_set_rate;
	chip->get_rate = gpio_rk3506_get_rate;
	chip->set_dir = gpio_rk3506_set_dir;
	chip->get_dir = gpio_rk3506_get_dir;
	chip->set_value = gpio_rk3506_set_value;
	chip->get_value = gpio_rk3506_get_value;
	chip->to_irq = gpio_rk3506_to_irq;
	chip->priv = pdat;

	if(!(dev = register_gpiochip(chip, drv)))
	{
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
		return NULL;
	}
	return dev;
}

static void gpio_rk3506_remove(struct device_t * dev)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)dev->priv;

	if(chip)
	{
		unregister_gpiochip(chip);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
	}
}

static void gpio_rk3506_suspend(struct device_t * dev)
{
}

static void gpio_rk3506_resume(struct device_t * dev)
{
}

static struct driver_t gpio_rk3506 = {
	.name		= "gpio-rk3506",
	.probe		= gpio_rk3506_probe,
	.remove		= gpio_rk3506_remove,
	.suspend	= gpio_rk3506_suspend,
	.resume		= gpio_rk3506_resume,
};

static void gpio_rk3506_driver_init(void)
{
	register_driver(&gpio_rk3506);
}

static void gpio_rk3506_driver_exit(void)
{
	unregister_driver(&gpio_rk3506);
}

driver_initcall(gpio_rk3506_driver_init);
driver_exitcall(gpio_rk3506_driver_exit);
