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
 * AHT20 - Digital Humidity and Temperature Sensor
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: AHT20 sensor slave address (default: 0x38)
 *
 * Example device tree node:
 *   "hygrometer-aht20": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 56
 *   }
 */

enum {
	AHT20_REG_STATUS	= 0x00,
	AHT20_REG_MEASURE	= 0xac,
	AHT20_REG_RESET		= 0xba,
	AHT20_REG_INIT		= 0xbe,
};

struct hygrometer_aht20_pdata_t {
	struct i2c_device_t * dev;
	int humidity;
	int temperature;
};

static int aht20_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int aht20_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int aht20_initial(struct i2c_device_t * dev)
{
	int trycnt = 3;
	uint8_t status;

	mdelay(5);
	if(!aht20_read(dev, AHT20_REG_STATUS, &status, 1))
		return 0;
	while(--trycnt > 0)
	{
		int cnt = 3;
		do {
			aht20_write(dev, AHT20_REG_INIT, (uint8_t[]){ 0x08, 0x00 }, 2);
			mdelay(10);
			aht20_read(dev, AHT20_REG_STATUS, &status, 1);
		} while(!(status & (1 << 3)) && (--cnt > 0));
		if(cnt > 0)
			return 1;
		else
		{
			aht20_write(dev, AHT20_REG_RESET, (uint8_t[]){ 0x00 }, 1);
			mdelay(100);
		}
	}
	return 0;
}

static int hygrometer_aht20_get(struct hygrometer_t * hygrometer, int * humidity, int * temperature)
{
	struct hygrometer_aht20_pdata_t * pdat = (struct hygrometer_aht20_pdata_t *)hygrometer->priv;
	uint8_t buf[7];
	int32_t tmp;

	if(aht20_write(pdat->dev, AHT20_REG_MEASURE, (uint8_t[]){ 0x33, 0x00 }, 2))
	{
		mdelay(80);
		if(aht20_read(pdat->dev, AHT20_REG_STATUS, buf, 7))
		{
			tmp = buf[1];
			tmp = tmp << 8;
			tmp += buf[2];
			tmp = tmp << 8;
			tmp += buf[3];
			tmp = tmp >> 4;
			pdat->humidity = (int)(tmp * 100.0f / 1048576.0f);

			tmp = buf[3] & 0x0f;
			tmp = tmp << 8;
			tmp += buf[4];
			tmp = tmp << 8;
			tmp += buf[5];
			pdat->temperature = (int)(tmp * 200000.0f / 1048576.0f - 50000.0f);
		}
	}
	if(humidity)
		*humidity = pdat->humidity;
	if(temperature)
		*temperature = pdat->temperature;
	return 1;
}

static struct device_t * hygrometer_aht20_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct hygrometer_aht20_pdata_t * pdat;
	struct hygrometer_t * h;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x38), 0);
	if(!i2cdev)
		return NULL;

	if(!aht20_initial(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct hygrometer_aht20_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	h = xos_mem_malloc(sizeof(struct hygrometer_t));
	if(!h)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct hygrometer_aht20_pdata_t));
	pdat->dev = i2cdev;
	pdat->humidity = 0;
	pdat->temperature = 0;

	h->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	h->get = hygrometer_aht20_get;
	h->priv = pdat;

	if(!(dev = register_hygrometer(h, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(h->name);
		xos_mem_free(h->priv);
		xos_mem_free(h);
		return NULL;
	}
	return dev;
}

static void hygrometer_aht20_remove(struct device_t * dev)
{
	struct hygrometer_t * h = (struct hygrometer_t *)dev->priv;
	struct hygrometer_aht20_pdata_t * pdat = (struct hygrometer_aht20_pdata_t *)h->priv;

	if(h)
	{
		unregister_hygrometer(h);
		i2c_device_free(pdat->dev);
		free_device_name(h->name);
		xos_mem_free(h->priv);
		xos_mem_free(h);
	}
}

static void hygrometer_aht20_suspend(struct device_t * dev)
{
}

static void hygrometer_aht20_resume(struct device_t * dev)
{
}

static struct driver_t hygrometer_aht20 = {
	.name		= "hygrometer-aht20",
	.probe		= hygrometer_aht20_probe,
	.remove		= hygrometer_aht20_remove,
	.suspend	= hygrometer_aht20_suspend,
	.resume		= hygrometer_aht20_resume,
};

static void hygrometer_aht20_driver_init(void)
{
	register_driver(&hygrometer_aht20);
}

static void hygrometer_aht20_driver_exit(void)
{
	unregister_driver(&hygrometer_aht20);
}

driver_initcall(hygrometer_aht20_driver_init);
driver_exitcall(hygrometer_aht20_driver_exit);
