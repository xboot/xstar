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

#include <linux/linux.h>

struct gpio_v1_info_t {
	int valid;
	int dir;
	int handle;
};

struct gpio_v1_linux_pdata_t {
	int fd;
	int base;
	int ngpio;
	struct gpio_v1_info_t * info;
};

static void gpio_v1_linux_ready(int fd, int offset, int dir, int val, struct gpio_v1_info_t * info)
{
	if(info)
	{
		if((info->handle > 0) && (dir == info->dir))
			return;
		else
		{
			if(info->handle > 0)
			{
				linux_gpio_v1_close_handle(info->handle);
				info->handle = 0;
			}
			info->dir = dir;
			info->handle = linux_gpio_v1_open_handle(fd, offset, dir, val);
		}
	}
}

static void gpio_v1_linux_set_cfg(struct gpiochip_t * chip, int offset, int cfg)
{
}

static int gpio_v1_linux_get_cfg(struct gpiochip_t * chip, int offset)
{
	struct gpio_v1_linux_pdata_t * pdat = (struct gpio_v1_linux_pdata_t *)chip->priv;

	if((offset >= chip->ngpio))
		return 0;

	return pdat->info[offset].valid ? 1 : 0;
}

static void gpio_v1_linux_set_pull(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull)
{
}

static enum gpio_pull_t gpio_v1_linux_get_pull(struct gpiochip_t * chip, int offset)
{
	return GPIO_PULL_NONE;
}

static void gpio_v1_linux_set_drv(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv)
{
}

static enum gpio_drv_t gpio_v1_linux_get_drv(struct gpiochip_t * chip, int offset)
{
	return GPIO_DRV_WEAK;
}

static void gpio_v1_linux_set_rate(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate)
{
}

static enum gpio_rate_t gpio_v1_linux_get_rate(struct gpiochip_t * chip, int offset)
{
	return GPIO_RATE_SLOW;
}

static void gpio_v1_linux_set_dir(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir)
{
}

static enum gpio_direction_t gpio_v1_linux_get_dir(struct gpiochip_t * chip, int offset)
{
	struct gpio_v1_linux_pdata_t * pdat = (struct gpio_v1_linux_pdata_t *)chip->priv;

	if(offset >= chip->ngpio)
		return GPIO_DIRECTION_INPUT;

	return pdat->info[offset].dir ? GPIO_DIRECTION_OUTPUT : GPIO_DIRECTION_INPUT;
}

static void gpio_v1_linux_set_value(struct gpiochip_t * chip, int offset, int value)
{
	struct gpio_v1_linux_pdata_t * pdat = (struct gpio_v1_linux_pdata_t *)chip->priv;

	if((offset >= chip->ngpio) || !pdat->info[offset].valid)
		return;

	gpio_v1_linux_ready(pdat->fd, offset, 1, value, &pdat->info[offset]);
	linux_gpio_v1_set_value(pdat->info[offset].handle, offset, value);
}

static int gpio_v1_linux_get_value(struct gpiochip_t * chip, int offset)
{
	struct gpio_v1_linux_pdata_t * pdat = (struct gpio_v1_linux_pdata_t *)chip->priv;

	if((offset >= chip->ngpio) || !pdat->info[offset].valid)
		return 0;

	gpio_v1_linux_ready(pdat->fd, offset, 0, 0, &pdat->info[offset]);
	return linux_gpio_v1_get_value(pdat->info[offset].handle, offset);
}

static int gpio_v1_linux_to_irq(struct gpiochip_t * chip, int offset)
{
	return -1;
}

static struct device_t * gpio_v1_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct gpio_v1_linux_pdata_t * pdat;
	struct gpiochip_t * chip;
	struct device_t * dev;
	int base = dt_read_int(n, "gpio-base", -1);
	int ngpio;
	int fd = linux_gpio_v1_open(dt_read_string(n, "device", NULL), &ngpio);

	if((fd <= 0) || (base < 0) || (ngpio <= 0))
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct gpio_v1_linux_pdata_t));
	if(!pdat)
		return NULL;

	chip = xos_mem_malloc(sizeof(struct gpiochip_t));
	if(!chip)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->fd = fd;
	pdat->base = base;
	pdat->ngpio = ngpio;
	pdat->info = xos_mem_malloc(sizeof(struct gpio_v1_info_t) * pdat->ngpio);
	for(int i = 0; i < pdat->ngpio; i++)
	{
		struct gpio_v1_info_t * info = &pdat->info[i];
		info->valid = linux_gpio_v1_info(pdat->fd, i, &info->dir);
		info->handle = 0;
	}

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->ngpio = pdat->ngpio;
	chip->set_cfg = gpio_v1_linux_set_cfg;
	chip->get_cfg = gpio_v1_linux_get_cfg;
	chip->set_pull = gpio_v1_linux_set_pull;
	chip->get_pull = gpio_v1_linux_get_pull;
	chip->set_drv = gpio_v1_linux_set_drv;
	chip->get_drv = gpio_v1_linux_get_drv;
	chip->set_rate = gpio_v1_linux_set_rate;
	chip->get_rate = gpio_v1_linux_get_rate;
	chip->set_dir = gpio_v1_linux_set_dir;
	chip->get_dir = gpio_v1_linux_get_dir;
	chip->set_value = gpio_v1_linux_set_value;
	chip->get_value = gpio_v1_linux_get_value;
	chip->to_irq = gpio_v1_linux_to_irq;
	chip->priv = pdat;

	if(!(dev = register_gpiochip(chip, drv)))
	{
		linux_gpio_v1_close(pdat->fd);
		xos_mem_free(pdat->info);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
		return NULL;
	}
	return dev;
}

static void gpio_v1_linux_remove(struct device_t * dev)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)dev->priv;
	struct gpio_v1_linux_pdata_t * pdat = (struct gpio_v1_linux_pdata_t *)chip->priv;

	if(chip)
	{
		unregister_gpiochip(chip);
		linux_gpio_v1_close(pdat->fd);
		xos_mem_free(pdat->info);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
	}
}

static void gpio_v1_linux_suspend(struct device_t * dev)
{
}

static void gpio_v1_linux_resume(struct device_t * dev)
{
}

static struct driver_t gpio_v1_linux = {
	.name		= "gpio-v1-linux",
	.probe		= gpio_v1_linux_probe,
	.remove		= gpio_v1_linux_remove,
	.suspend	= gpio_v1_linux_suspend,
	.resume		= gpio_v1_linux_resume,
};

static void gpio_v1_linux_driver_init(void)
{
	register_driver(&gpio_v1_linux);
}

static void gpio_v1_linux_driver_exit(void)
{
	unregister_driver(&gpio_v1_linux);
}

driver_initcall(gpio_v1_linux_driver_init);
driver_exitcall(gpio_v1_linux_driver_exit);
