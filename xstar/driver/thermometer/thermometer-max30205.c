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
 * MAX30205 - high precision human body temperature sensor
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: MAX30205 sensor slave address (default: 0x48)
 *
 * Example device tree node:
 *   "thermometer-max30205": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 72
 *   }
 */

enum {
	MAX30205_REG_TEMP	= 0x00,
	MAX30205_REG_CFG	= 0x01,
	MAX30205_REG_THYST	= 0x02,
	MAX30205_REG_TOS	= 0x03,
};

struct thermometer_max30205_pdata_t {
	struct i2c_device_t * dev;
};

static int max30205_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int max30205_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int max30205_initial(struct i2c_device_t * dev)
{
	uint8_t cfg;

	if(!max30205_read(dev, MAX30205_REG_CFG, &cfg, 1))
		return 0;
	if(!max30205_write(dev, MAX30205_REG_CFG, (uint8_t[]){ 0 << 5 }, 1))
		return 0;
	return 1;
}

static int thermometer_max30205_get(struct thermometer_t * thermometer, int * temperature)
{
	struct thermometer_max30205_pdata_t * pdat = (struct thermometer_max30205_pdata_t *)thermometer->priv;
	uint8_t buf[2];

	if(max30205_read(pdat->dev, MAX30205_REG_TEMP, buf, 2))
	{
		int16_t raw = (int16_t)(((uint16_t)buf[0] << 8) | buf[1]);
		int temp = (((int)raw * 1000) >> 8) + 64 * 1000;
		if(temperature)
			*temperature = temp;
		return 1;
	}
	return 0;
}

static struct device_t * thermometer_max30205_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct thermometer_max30205_pdata_t * pdat;
	struct thermometer_t * t;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x48), 0);
	if(!i2cdev)
		return NULL;

	if(!max30205_initial(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct thermometer_max30205_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	t = xos_mem_malloc(sizeof(struct thermometer_t));
	if(!t)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct thermometer_max30205_pdata_t));
	pdat->dev = i2cdev;

	t->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	t->get = thermometer_max30205_get;
	t->priv = pdat;

	if(!(dev = register_thermometer(t, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(t->name);
		xos_mem_free(t->priv);
		xos_mem_free(t);
		return NULL;
	}
	return dev;
}

static void thermometer_max30205_remove(struct device_t * dev)
{
	struct thermometer_t * t = (struct thermometer_t *)dev->priv;
	struct thermometer_max30205_pdata_t * pdat = (struct thermometer_max30205_pdata_t *)t->priv;

	if(t)
	{
		unregister_thermometer(t);
		i2c_device_free(pdat->dev);
		free_device_name(t->name);
		xos_mem_free(t->priv);
		xos_mem_free(t);
	}
}

static void thermometer_max30205_suspend(struct device_t * dev)
{
}

static void thermometer_max30205_resume(struct device_t * dev)
{
}

static struct driver_t thermometer_max30205 = {
	.name		= "thermometer-max30205",
	.probe		= thermometer_max30205_probe,
	.remove		= thermometer_max30205_remove,
	.suspend	= thermometer_max30205_suspend,
	.resume		= thermometer_max30205_resume,
};

static void thermometer_max30205_driver_init(void)
{
	register_driver(&thermometer_max30205);
}

static void thermometer_max30205_driver_exit(void)
{
	unregister_driver(&thermometer_max30205);
}

driver_initcall(thermometer_max30205_driver_init);
driver_exitcall(thermometer_max30205_driver_exit);
