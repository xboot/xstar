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
 * MCP23017 - The 16-bit I/O expander using i2c interface
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: the slave address (default: 0x20)
 *
 * Example device tree node:
 *   "gpio-mcp23017:0": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 32,
 *       "gpio-base": 512
 *   }
 */

enum {
	IODIRA		= 0x00,
	IODIRB		= 0x01,
	IPOLA		= 0x02,
	IPOLB		= 0x03,
	GPINTENA	= 0x04,
	GPINTENB	= 0x05,
	DEFVALA		= 0x06,
	DEFVALB		= 0x07,
	INTCONA		= 0x08,
	INTCONB		= 0x09,
	IOCON		= 0x0A,
	GPPUA		= 0x0C,
	GPPUB		= 0x0D,
	INTFA		= 0x0E,
	INTFB		= 0x0F,
	INTCAPA		= 0x10,
	INTCAPB		= 0x11,
	GPIOA		= 0x12,
	GPIOB		= 0x13,
	OLATA		= 0x14,
	OLATB		= 0x15,
};

struct gpio_mcp23017_pdata_t {
	struct i2c_device_t * dev;
	int base;
	int ngpio;
	int oirq;
};

static int mcp23017_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
{
	struct i2c_msg_t msgs[2];

	msgs[0].addr = dev->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &reg;

	msgs[1].addr = dev->addr;
	msgs[1].flags = I2C_MODE_RD;
	msgs[1].len = len;
	msgs[1].buf = buf;

	if(i2c_transfer(dev->i2c, msgs, 2) != 2)
		return 0;
	return 1;
}

static int mcp23017_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
{
	struct i2c_msg_t msg;
	uint8_t mbuf[256];

	if(len > sizeof(mbuf) - 1)
		len = sizeof(mbuf) - 1;
	mbuf[0] = reg;
	xos_memcpy(&mbuf[1], buf, len);

	msg.addr = dev->addr;
	msg.flags = 0;
	msg.len = len + 1;
	msg.buf = &mbuf[0];

	if(i2c_transfer(dev->i2c, &msg, 1) != 1)
		return 0;
	return 1;
}

static int mcp23017_detect(struct i2c_device_t * dev)
{
	uint8_t val;

	if(!mcp23017_read(dev, IOCON, &val, 1))
		return 0;
	mcp23017_write(dev, IOCON, (uint8_t[]){ 0x40 }, 1);
	mcp23017_write(dev, GPPUA, (uint8_t[]){ 0xff }, 1);
	mcp23017_write(dev, GPPUB, (uint8_t[]){ 0xff }, 1);
	return 1;
}

static void gpio_mcp23017_set_cfg(struct gpiochip_t * chip, int offset, int cfg)
{
}

static int gpio_mcp23017_get_cfg(struct gpiochip_t * chip, int offset)
{
	return 0;
}

static void gpio_mcp23017_set_pull(struct gpiochip_t * chip, int offset, enum gpio_pull_t pull)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;
	uint8_t reg, val;

	if(offset >= chip->ngpio)
		return;

	reg = (offset < 8) ? GPPUA : GPPUB;
	mcp23017_read(pdat->dev, reg, &val, 1);
	if(pull == GPIO_PULL_UP)
		val |= (1 << (offset & 0x7));
	else
		val &= ~(1 << (offset & 0x7));
	mcp23017_write(pdat->dev, reg, &val, 1);
}

static enum gpio_pull_t gpio_mcp23017_get_pull(struct gpiochip_t * chip, int offset)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;
	uint8_t reg, val;

	if(offset >= chip->ngpio)
		return 0;

	reg = (offset < 8) ? GPPUA : GPPUB;
	mcp23017_read(pdat->dev, reg, &val, 1);
	if(val & (1 << (offset & 0x7)))
		return GPIO_PULL_UP;
	return GPIO_PULL_NONE;
}

static void gpio_mcp23017_set_drv(struct gpiochip_t * chip, int offset, enum gpio_drv_t drv)
{
}

static enum gpio_drv_t gpio_mcp23017_get_drv(struct gpiochip_t * chip, int offset)
{
	return GPIO_DRV_WEAK;
}

static void gpio_mcp23017_set_rate(struct gpiochip_t * chip, int offset, enum gpio_rate_t rate)
{
}

static enum gpio_rate_t gpio_mcp23017_get_rate(struct gpiochip_t * chip, int offset)
{
	return GPIO_RATE_SLOW;
}

static void gpio_mcp23017_set_dir(struct gpiochip_t * chip, int offset, enum gpio_direction_t dir)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;
	uint8_t reg, val;

	if(offset >= chip->ngpio)
		return;

	reg = (offset < 8) ? IODIRA : IODIRB;
	mcp23017_read(pdat->dev, reg, &val, 1);
	if(dir == GPIO_DIRECTION_INPUT)
		val |= (1 << (offset & 0x7));
	else
		val &= ~(1 << (offset & 0x7));
	mcp23017_write(pdat->dev, reg, &val, 1);
}

static enum gpio_direction_t gpio_mcp23017_get_dir(struct gpiochip_t * chip, int offset)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;
	uint8_t reg, val;

	if(offset >= chip->ngpio)
		return 0;

	reg = (offset < 8) ? IODIRA : IODIRB;
	mcp23017_read(pdat->dev, reg, &val, 1);
	if(val & (1 << (offset & 0x7)))
		return GPIO_DIRECTION_INPUT;
	return GPIO_DIRECTION_OUTPUT;
}

static void gpio_mcp23017_set_value(struct gpiochip_t * chip, int offset, int value)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;
	uint8_t reg, val;

	if(offset >= chip->ngpio)
		return;

	reg = (offset < 8) ? OLATA : OLATB;
	mcp23017_read(pdat->dev, reg, &val, 1);
	if(value)
		val |= (1 << (offset & 0x7));
	else
		val &= ~(1 << (offset & 0x7));
	mcp23017_write(pdat->dev, reg, &val, 1);
}

static int gpio_mcp23017_get_value(struct gpiochip_t * chip, int offset)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;
	uint8_t reg, val;

	if(offset >= chip->ngpio)
		return 0;

	reg = (offset < 8) ? GPIOA : GPIOB;
	mcp23017_read(pdat->dev, reg, &val, 1);
	return !!(val & (1 << (offset & 0x7)));
}

static int gpio_mcp23017_to_irq(struct gpiochip_t * chip, int offset)
{
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;

	if((offset >= chip->ngpio) || (pdat->oirq < 0))
		return -1;
	return pdat->oirq + offset;
}

static struct device_t * gpio_mcp23017_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct gpio_mcp23017_pdata_t * pdat;
	struct gpiochip_t * chip;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x20), 0);
	if(!i2cdev)
		return NULL;

	if(!mcp23017_detect(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct gpio_mcp23017_pdata_t));
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
	pdat->ngpio = 16;
	pdat->oirq = dt_read_int(n, "interrupt-offset", -1);

	chip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	chip->base = pdat->base;
	chip->ngpio = pdat->ngpio;
	chip->set_cfg = gpio_mcp23017_set_cfg;
	chip->get_cfg = gpio_mcp23017_get_cfg;
	chip->set_pull = gpio_mcp23017_set_pull;
	chip->get_pull = gpio_mcp23017_get_pull;
	chip->set_drv = gpio_mcp23017_set_drv;
	chip->get_drv = gpio_mcp23017_get_drv;
	chip->set_rate = gpio_mcp23017_set_rate;
	chip->get_rate = gpio_mcp23017_get_rate;
	chip->set_dir = gpio_mcp23017_set_dir;
	chip->get_dir = gpio_mcp23017_get_dir;
	chip->set_value = gpio_mcp23017_set_value;
	chip->get_value = gpio_mcp23017_get_value;
	chip->to_irq = gpio_mcp23017_to_irq;
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

static void gpio_mcp23017_remove(struct device_t * dev)
{
	struct gpiochip_t * chip = (struct gpiochip_t *)dev->priv;
	struct gpio_mcp23017_pdata_t * pdat = (struct gpio_mcp23017_pdata_t *)chip->priv;

	if(chip)
	{
		unregister_gpiochip(chip);
		i2c_device_free(pdat->dev);
		free_device_name(chip->name);
		xos_mem_free(chip->priv);
		xos_mem_free(chip);
	}
}

static void gpio_mcp23017_suspend(struct device_t * dev)
{
}

static void gpio_mcp23017_resume(struct device_t * dev)
{
}

static struct driver_t gpio_mcp23017 = {
	.name		= "gpio-mcp23017",
	.probe		= gpio_mcp23017_probe,
	.remove		= gpio_mcp23017_remove,
	.suspend	= gpio_mcp23017_suspend,
	.resume		= gpio_mcp23017_resume,
};

static void gpio_mcp23017_driver_init(void)
{
	register_driver(&gpio_mcp23017);
}

static void gpio_mcp23017_driver_exit(void)
{
	unregister_driver(&gpio_mcp23017);
}

driver_initcall(gpio_mcp23017_driver_init);
driver_exitcall(gpio_mcp23017_driver_exit);
