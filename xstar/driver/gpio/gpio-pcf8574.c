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

/*
 * PCF8574 - The 8-bit I/O expander using i2c interface
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: the slave address (default: 0x20)
 *
 * Example device tree node:
 *   "gpio-pcf8574:0": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 32,
 *       "gpio-base": 512,
 *       "latch-state": 255
 *   }
 */

struct gpio_pcf8574_pdata_t {
	struct i2c_device_t * dev;
	int base;
	int ngpio;
	int oirq;
	uint8_t latch;
};

static int pcf8574_read(struct i2c_device_t * dev, uint8_t * val)
{
	struct i2c_msg_t msgs;

	msgs.addr = dev->addr;
	msgs.flags = I2C_MODE_RD;
	msgs.len = 1;
	msgs.buf = val;
	if(i2c_transfer(dev->i2c, &msgs, 1) != 1)
		return 0;
	return 1;
}

static int pcf8574_write(struct i2c_device_t * dev, uint8_t val)
{
	struct i2c_msg_t msg;

	msg.addr = dev->addr;
	msg.flags = 0;
	msg.len = 1;
	msg.buf = &val;
	if(i2c_transfer(dev->i2c, &msg, 1) != 1)
		return 0;
	return 1;
}

static int pcf8574_detect(struct i2c_device_t * dev)
{
	uint8_t latch;

	if(!pcf8574_read(dev, &latch))
		return 0;
	return 1;
}

static void gpio_pcf8574_set_cfg(struct gpiochip_t * chip, int offset, int cfg)
{
}

static int gpio_pcf8574_get_cfg(struct gpiochip_t * chip, int offset)
{
	return 0;
}

static void gpio_pcf8574_set_pull(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull)
{
}

static enum gpio_pull_t gpio_pcf8574_get_pull(struct gpiochip_t * chip, int offset)
{
	return GPIO_PULL_NONE;
}

static void gpio_pcf8574_set_drv(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv)
{
}

static enum gpio_drv_t gpio_pcf8574_get_drv(struct gpiochip_t * chip, int offset)
{
	return GPIO_DRV_WEAK;
}

static void gpio_pcf8574_set_rate(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate)
{
}

static enum gpio_rate_t gpio_pcf8574_get_rate(struct gpiochip_t * chip, int offset)
{
	return GPIO_RATE_SLOW;
}

static void gpio_pcf8574_set_dir(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir)
{
}

static enum gpio_direction_t gpio_pcf8574_get_dir(struct gpiochip_t * chip, int offset)
{
	return GPIO_DIRECTION_INPUT;
}

static void gpio_pcf8574_set_value(struct gpiochip_t * chip, int offset, int value)
{
	struct gpio_pcf8574_pdata_t * pdat = (struct gpio_pcf8574_pdata_t *)chip->priv;

	if(offset >= chip->ngpio)
		return;
	if(value)
		pdat->latch |= (1 << offset);
	else
		pdat->latch &= ~(1 << offset);
	pcf8574_write(pdat->dev, pdat->latch);
}

static int gpio_pcf8574_get_value(struct gpiochip_t * chip, int offset)
{
	struct gpio_pcf8574_pdata_t * pdat = (struct gpio_pcf8574_pdata_t *)chip->priv;
	uint8_t val;

	if(offset >= chip->ngpio)
		return 0;
	if(!pcf8574_read(pdat->dev, &val))
		return 0;
	return !!(val & (1 << offset));
}

static int gpio_pcf8574_to_irq(struct gpiochip_t * chip, int offset)
{
	struct gpio_pcf8574_pdata_t * pdat = (struct gpio_pcf8574_pdata_t *)chip->priv;

	if((offset >= chip->ngpio) || (pdat->oirq < 0))
		return -1;
	return pdat->oirq + offset;
}

static struct device_t * gpio_pcf8574_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct gpio_pcf8574_pdata_t * pdat;
	struct gpiochip_t * chip;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x20), 0);
	if(!i2cdev)
		return NULL;

	if(!pcf8574_detect(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct gpio_pcf8574_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	chip = xos_mem_malloc(sizeof(struct gpiochip_t));
	if(!chip)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->dev = i2cdev;
	pdat->base = dt_read_int(n, "gpio-base", 512);
	pdat->ngpio = 8;
	pdat->oirq = dt_read_int(n, "interrupt-offset", -1);
	pdat->latch = dt_read_u8(n, "latch-state", 0xff);

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->ngpio = pdat->ngpio;
	chip->set_cfg = gpio_pcf8574_set_cfg;
	chip->get_cfg = gpio_pcf8574_get_cfg;
	chip->set_pull = gpio_pcf8574_set_pull;
	chip->get_pull = gpio_pcf8574_get_pull;
	chip->set_drv = gpio_pcf8574_set_drv;
	chip->get_drv = gpio_pcf8574_get_drv;
	chip->set_rate = gpio_pcf8574_set_rate;
	chip->get_rate = gpio_pcf8574_get_rate;
	chip->set_dir = gpio_pcf8574_set_dir;
	chip->get_dir = gpio_pcf8574_get_dir;
	chip->set_value = gpio_pcf8574_set_value;
	chip->get_value = gpio_pcf8574_get_value;
	chip->to_irq = gpio_pcf8574_to_irq;
	chip->priv = pdat;

	if(!(dev = register_gpiochip(chip, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
		return NULL;
	}
	return dev;
}

static void gpio_pcf8574_remove(struct device_t * dev)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)dev->priv;
	struct gpio_pcf8574_pdata_t * pdat = (struct gpio_pcf8574_pdata_t *)chip->priv;

	if(chip)
	{
		unregister_gpiochip(chip);
		i2c_device_free(pdat->dev);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
	}
}

static void gpio_pcf8574_suspend(struct device_t * dev)
{
}

static void gpio_pcf8574_resume(struct device_t * dev)
{
}

static struct driver_t gpio_pcf8574 = {
	.name		= "gpio-pcf8574",
	.probe		= gpio_pcf8574_probe,
	.remove		= gpio_pcf8574_remove,
	.suspend	= gpio_pcf8574_suspend,
	.resume		= gpio_pcf8574_resume,
};

static void gpio_pcf8574_driver_init(void)
{
	register_driver(&gpio_pcf8574);
}

static void gpio_pcf8574_driver_exit(void)
{
	unregister_driver(&gpio_pcf8574);
}

driver_initcall(gpio_pcf8574_driver_init);
driver_exitcall(gpio_pcf8574_driver_exit);
