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

#include <driver/gpio/gpio.h>

static ssize_t gpiochip_read_base(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	return xos_sprintf(buf, "%d", chip->base);
}

static ssize_t gpiochip_read_ngpio(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	return xos_sprintf(buf, "%d", chip->ngpio);
}

static ssize_t gpiochip_read_config(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
			len += xos_sprintf((char *)(p + len), "[%d] 0x%x\r\n", i, gpio_get_cfg(i));
	}
	return len;
}

static ssize_t gpiochip_read_pull(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * s, * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
		{
			switch(gpio_get_pull(i))
			{
			case GPIO_PULL_UP:
				s = "up";
				break;
			case GPIO_PULL_DOWN:
				s = "down";
				break;
			case GPIO_PULL_NONE:
			default:
				s = "none";
				break;
			}
			len += xos_sprintf((char *)(p + len), "[%d] %s\r\n", i, s);
		}
	}
	return len;
}

static ssize_t gpiochip_read_strength(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * s, * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
		{
			switch(gpio_get_drv(i))
			{
			case GPIO_DRV_WEAKER:
				s = "weaker";
				break;
			case GPIO_DRV_STRONGER:
				s = "stronger";
				break;
			case GPIO_DRV_STRONG:
				s = "strong";
				break;
			case GPIO_DRV_WEAK:
			default:
				s = "weak";
				break;
			}
			len += xos_sprintf((char *)(p + len), "[%d] %s\r\n", i, s);
		}
	}
	return len;
}

static ssize_t gpiochip_read_rate(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * s, * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
		{
			switch(gpio_get_rate(i))
			{
			case GPIO_RATE_FAST:
				s = "fast";
				break;
			case GPIO_RATE_SLOW:
			default:
				s = "slow";
				break;
			}
			len += xos_sprintf((char *)(p + len), "[%d] %s\r\n", i, s);
		}
	}
	return len;
}

static ssize_t gpiochip_read_direction(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * s, * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
		{
			switch(gpio_get_direction(i))
			{
			case GPIO_DIRECTION_OUTPUT:
				s = "output";
				break;
			case GPIO_DIRECTION_INPUT:
			default:
				s = "input";
				break;
			}
			len += xos_sprintf((char *)(p + len), "[%d] %s\r\n", i, s);
		}
	}
	return len;
}

static ssize_t gpiochip_read_value(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
			len += xos_sprintf((char *)(p + len), "[%d] %d\r\n", i, gpio_get_value(i));
	}
	return len;
}

static ssize_t gpiochip_read_interrupt(struct kobj_t * kobj, void * buf, size_t size)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)kobj->priv;
	char * p = buf;
	int len = 0;
	int i;

	for(i = chip->base; i < chip->base + chip->ngpio; i++)
	{
		if(gpio_is_valid(i))
			len += xos_sprintf((char *)(p + len), "[%d] %d\r\n", i, gpio_to_irq(i));
	}
	return len;
}

struct gpiochip_t * search_gpiochip(int gpio)
{
	struct device_t * pos, * n;
	struct gpiochip_t * chip;

	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_GPIOCHIP], head)
	{
		chip = (struct gpiochip_t *)(pos->priv);
		if((gpio >= chip->base) && (gpio < (chip->base + chip->ngpio)))
			return chip;
	}
	return NULL;
}

struct device_t * register_gpiochip(struct gpiochip_t * chip, struct driver_t * drv)
{
	struct device_t * dev;

	if(!chip || !chip->name)
		return NULL;

	if(chip->base < 0 || chip->ngpio <= 0)
		return NULL;

	if(!chip->set_cfg || !chip->get_cfg ||
		!chip->set_pull || !chip->get_pull ||
		!chip->set_drv || !chip->get_drv ||
		!chip->set_rate || !chip->get_rate ||
		!chip->set_dir || !chip->get_dir ||
		!chip->set_value || !chip->get_value ||
		!chip->to_irq)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(chip->name);
	dev->type = DEVICE_TYPE_GPIOCHIP;
	dev->driver = drv;
	dev->priv = chip;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "base", gpiochip_read_base, NULL, chip);
	kobj_add_regular(dev->kobj, "ngpio", gpiochip_read_ngpio, NULL, chip);
	kobj_add_regular(dev->kobj, "config", gpiochip_read_config, NULL, chip);
	kobj_add_regular(dev->kobj, "pull", gpiochip_read_pull, NULL, chip);
	kobj_add_regular(dev->kobj, "strength", gpiochip_read_strength, NULL, chip);
	kobj_add_regular(dev->kobj, "rate", gpiochip_read_rate, NULL, chip);
	kobj_add_regular(dev->kobj, "direction", gpiochip_read_direction, NULL, chip);
	kobj_add_regular(dev->kobj, "value", gpiochip_read_value, NULL, chip);
	kobj_add_regular(dev->kobj, "interrupt", gpiochip_read_interrupt, NULL, chip);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_gpiochip(struct gpiochip_t * chip)
{
	struct device_t * dev;

	if(chip && chip->name && (chip->base >= 0) && (chip->ngpio > 0))
	{
		dev = search_device(chip->name, DEVICE_TYPE_GPIOCHIP);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

int gpio_is_valid(int gpio)
{
	return search_gpiochip(gpio) ? 1 : 0;
}

void gpio_set_cfg(int gpio, int cfg)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		chip->set_cfg(chip, gpio - chip->base, cfg);
}

int gpio_get_cfg(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->get_cfg(chip, gpio - chip->base);
	return 0;
}

void gpio_set_pull(int gpio, enum gpio_pull_t pull)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		chip->set_pull(chip, gpio - chip->base, pull);
}

enum gpio_pull_t gpio_get_pull(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->get_pull(chip, gpio - chip->base);
	return GPIO_PULL_NONE;
}

void gpio_set_drv(int gpio, enum gpio_drv_t drv)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		chip->set_drv(chip, gpio - chip->base, drv);
}

enum gpio_drv_t gpio_get_drv(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->get_drv(chip, gpio - chip->base);
	return GPIO_DRV_WEAK;
}

void gpio_set_rate(int gpio, enum gpio_rate_t rate)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		chip->set_rate(chip, gpio - chip->base, rate);
}

enum gpio_rate_t gpio_get_rate(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->get_rate(chip, gpio - chip->base);
	return GPIO_RATE_SLOW;
}

void gpio_set_direction(int gpio, enum gpio_direction_t dir)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		chip->set_dir(chip, gpio - chip->base, dir);
}

enum gpio_direction_t gpio_get_direction(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->get_dir(chip, gpio - chip->base);
	return GPIO_DIRECTION_INPUT;
}

void gpio_set_value(int gpio, int value)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		chip->set_value(chip, gpio - chip->base, value);
}

int gpio_get_value(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->get_value(chip, gpio - chip->base);
	return 0;
}

void gpio_direction_output(int gpio, int value)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
	{
		chip->set_dir(chip, gpio - chip->base, GPIO_DIRECTION_OUTPUT);
		chip->set_value(chip, gpio - chip->base, value);
	}
}

int gpio_direction_input(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
	{
		chip->set_dir(chip, gpio - chip->base, GPIO_DIRECTION_INPUT);
		return chip->get_value(chip, gpio - chip->base);
	}
	return 0;
}

int gpio_to_irq(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
		return chip->to_irq(chip, gpio - chip->base);
	return -1;
}

struct gpiodesc_t * gpiod_alloc(int gpio)
{
	struct gpiochip_t * chip = search_gpiochip(gpio);

	if(chip)
	{
		struct gpiodesc_t * desc = xos_mem_malloc(sizeof(struct gpiodesc_t));
		if(desc)
		{
			desc->chip = chip;
			desc->offset = gpio - chip->base;
			return desc;
		}
	}
	return NULL;
}

void gpiod_free(struct gpiodesc_t * desc)
{
	if(desc)
		xos_mem_free(desc);
}
