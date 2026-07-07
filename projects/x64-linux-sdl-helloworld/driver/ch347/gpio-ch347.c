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

#include <ch347.h>

struct gpio_ch347_pdata_t {
	int fd;
	int base;
	int ngpio;
	int oirq;
	uint8_t dir;
	uint8_t dat;
};

static void gpio_ch347_set_cfg(struct gpiochip_t * chip, int offset, int cfg)
{
}

static int gpio_ch347_get_cfg(struct gpiochip_t * chip, int offset)
{
	return 0;
}

static void gpio_ch347_set_pull(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull)
{
}

static enum gpio_pull_t gpio_ch347_get_pull(struct gpiochip_t * chip, int offset)
{
	return GPIO_PULL_NONE;
}

static void gpio_ch347_set_drv(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv)
{
}

static enum gpio_drv_t gpio_ch347_get_drv(struct gpiochip_t * chip, int offset)
{
	return GPIO_DRV_WEAK;
}

static void gpio_ch347_set_rate(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate)
{
}

static enum gpio_rate_t gpio_ch347_get_rate(struct gpiochip_t * chip, int offset)
{
	return GPIO_RATE_SLOW;
}

static void gpio_ch347_set_dir(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir)
{
	struct gpio_ch347_pdata_t * pdat = (struct gpio_ch347_pdata_t *)chip->priv;

	if(offset >= chip->ngpio)
		return;

	ch347_lock();
	{
		switch(dir)
		{
		case GPIO_DIRECTION_INPUT:
			pdat->dir &= ~(1 << offset);
			break;
		case GPIO_DIRECTION_OUTPUT:
			pdat->dir |= (1 << offset);
			break;
		default:
			break;
		}
		CH347GPIO_Set(pdat->fd, 0xff, pdat->dir, pdat->dat);
	}
	ch347_unlock();
}

static enum gpio_direction_t gpio_ch347_get_dir(struct gpiochip_t * chip, int offset)
{
	struct gpio_ch347_pdata_t * pdat = (struct gpio_ch347_pdata_t *)chip->priv;
	uint8_t dir, dat;
	int ret;

	if(offset >= chip->ngpio)
		return GPIO_DIRECTION_INPUT;

	ch347_lock();
	{
		ret = CH347GPIO_Get(pdat->fd, &dir, &dat);
	}
	ch347_unlock();
	if(ret)
		return (dir & (1 << offset)) ? GPIO_DIRECTION_OUTPUT : GPIO_DIRECTION_INPUT;
	return GPIO_DIRECTION_INPUT;
}

static void gpio_ch347_set_value(struct gpiochip_t * chip, int offset, int value)
{
	struct gpio_ch347_pdata_t * pdat = (struct gpio_ch347_pdata_t *)chip->priv;

	if(offset >= chip->ngpio)
		return;

	ch347_lock();
	{
		if(value)
			pdat->dat |= (1 << offset);
		else
			pdat->dat &= ~(1 << offset);
		CH347GPIO_Set(pdat->fd, 0xff, pdat->dir, pdat->dat);
	}
	ch347_unlock();
}

static int gpio_ch347_get_value(struct gpiochip_t * chip, int offset)
{
	struct gpio_ch347_pdata_t * pdat = (struct gpio_ch347_pdata_t *)chip->priv;
	uint8_t dir, dat;
	int ret;

	if(offset >= chip->ngpio)
		return 0;

	ch347_lock();
	{
		ret = CH347GPIO_Get(pdat->fd, &dir, &dat);
	}
	ch347_unlock();
	if(ret)
		return !!(dat & (1 << offset));
	return 0;
}

static int gpio_ch347_to_irq(struct gpiochip_t * chip, int offset)
{
	struct gpio_ch347_pdata_t * pdat = (struct gpio_ch347_pdata_t *)chip->priv;

	if((offset >= chip->ngpio) || (pdat->oirq < 0))
		return -1;
	return pdat->oirq + offset;
}

static struct device_t * gpio_ch347_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct gpio_ch347_pdata_t * pdat;
	struct gpiochip_t * chip;
	struct device_t * dev;
	int fd;

	fd = ch347_detect();
	if(fd <= 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct gpio_ch347_pdata_t));
	if(!pdat)
		return NULL;

	chip = xos_mem_malloc(sizeof(struct gpiochip_t));
	if(!chip)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->fd = fd;
	pdat->base = dt_read_int(n, "gpio-base", 1024);
	pdat->ngpio = 8;
	pdat->oirq = dt_read_int(n, "interrupt-offset", -1);
	pdat->dir = 0xff;
	pdat->dat = 0x00;

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->ngpio = pdat->ngpio;
	chip->set_cfg = gpio_ch347_set_cfg;
	chip->get_cfg = gpio_ch347_get_cfg;
	chip->set_pull = gpio_ch347_set_pull;
	chip->get_pull = gpio_ch347_get_pull;
	chip->set_drv = gpio_ch347_set_drv;
	chip->get_drv = gpio_ch347_get_drv;
	chip->set_rate = gpio_ch347_set_rate;
	chip->get_rate = gpio_ch347_get_rate;
	chip->set_dir = gpio_ch347_set_dir;
	chip->get_dir = gpio_ch347_get_dir;
	chip->set_value = gpio_ch347_set_value;
	chip->get_value = gpio_ch347_get_value;
	chip->to_irq = gpio_ch347_to_irq;
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

static void gpio_ch347_remove(struct device_t * dev)
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

static void gpio_ch347_suspend(struct device_t * dev)
{
}

static void gpio_ch347_resume(struct device_t * dev)
{
}

static struct driver_t gpio_ch347 = {
	.name		= "gpio-ch347",
	.probe		= gpio_ch347_probe,
	.remove		= gpio_ch347_remove,
	.suspend	= gpio_ch347_suspend,
	.resume		= gpio_ch347_resume,
};

static void gpio_ch347_driver_init(void)
{
	register_driver(&gpio_ch347);
}

static void gpio_ch347_driver_exit(void)
{
	unregister_driver(&gpio_ch347);
}

driver_initcall(gpio_ch347_driver_init);
driver_exitcall(gpio_ch347_driver_exit);
