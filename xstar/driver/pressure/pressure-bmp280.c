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
 * BMP280 - Digital Pressure and Temperature Sensor
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: BMP280 sensor slave address (default: 0x77)
 *
 * Example device tree node:
 *   "pressure-bmp280": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 119
 *   }
 */

enum {
	BMP280_CHIPID_REG			= 0xD0,
	BMP280_RESET_REG			= 0xE0,
	BMP280_STATUS_REG			= 0xF3,
	BMP280_CTRLMEAS_REG			= 0xF4,
	BMP280_CONFIG_REG			= 0xF5,
	BMP280_PRESSURE_MSB_REG		= 0xF7,
	BMP280_PRESSURE_LSB_REG		= 0xF8,
	BMP280_PRESSURE_XLSB_REG	= 0xF9,
	BMP280_TEMPERATURE_MSB_REG	= 0xFA,
	BMP280_TEMPERATURE_LSB_REG	= 0xFB,
	BMP280_TEMPERATURE_XLSB_REG	= 0xFC,

	BMP280_DIG_T1_LSB_REG		= 0x88,
	BMP280_DIG_T1_MSB_REG		= 0x89,
	BMP280_DIG_T2_LSB_REG		= 0x8A,
	BMP280_DIG_T2_MSB_REG		= 0x8B,
	BMP280_DIG_T3_LSB_REG		= 0x8C,
	BMP280_DIG_T3_MSB_REG		= 0x8D,
	BMP280_DIG_P1_LSB_REG		= 0x8E,
	BMP280_DIG_P1_MSB_REG		= 0x8F,
	BMP280_DIG_P2_LSB_REG		= 0x90,
	BMP280_DIG_P2_MSB_REG		= 0x91,
	BMP280_DIG_P3_LSB_REG		= 0x92,
	BMP280_DIG_P3_MSB_REG		= 0x93,
	BMP280_DIG_P4_LSB_REG		= 0x94,
	BMP280_DIG_P4_MSB_REG		= 0x95,
	BMP280_DIG_P5_LSB_REG		= 0x96,
	BMP280_DIG_P5_MSB_REG		= 0x97,
	BMP280_DIG_P6_LSB_REG		= 0x98,
	BMP280_DIG_P6_MSB_REG		= 0x99,
	BMP280_DIG_P7_LSB_REG		= 0x9A,
	BMP280_DIG_P7_MSB_REG		= 0x9B,
	BMP280_DIG_P8_LSB_REG		= 0x9C,
	BMP280_DIG_P8_MSB_REG		= 0x9D,
	BMP280_DIG_P9_LSB_REG		= 0x9E,
	BMP280_DIG_P9_MSB_REG		= 0x9F,
};

struct pressure_bmp280_pdata_t {
	struct i2c_device_t * dev;
	struct {
		uint16_t t1;
		int16_t t2;
		int16_t t3;
		uint16_t p1;
		int16_t p2;
		int16_t p3;
		int16_t p4;
		int16_t p5;
		int16_t p6;
		int16_t p7;
		int16_t p8;
		int16_t p9;
		int32_t fine;
	} calib;
};

static int bmp280_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int bmp280_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int bmp280_initial(struct i2c_device_t * dev)
{
	uint8_t id;

	if(!bmp280_read(dev, BMP280_CHIPID_REG, &id, 1) || (id != 0x58))
		return 0;
	bmp280_write(dev, BMP280_CTRLMEAS_REG, (uint8_t[]){ ((0x04 << 2) | (0x05 << 5) | 0x03) }, 1);
	bmp280_write(dev, BMP280_CONFIG_REG, (uint8_t[]){ (0x5 << 2) }, 1);
	return 1;
}

static int bmp280_read_calib(struct pressure_bmp280_pdata_t * pdat)
{
	if(bmp280_read(pdat->dev, BMP280_DIG_T1_LSB_REG, (uint8_t *)&pdat->calib, 24))
	{
		pdat->calib.t1 = le16_to_cpu(pdat->calib.t1);
		pdat->calib.t2 = le16_to_cpu(pdat->calib.t2);
		pdat->calib.t3 = le16_to_cpu(pdat->calib.t3);
		pdat->calib.p1 = le16_to_cpu(pdat->calib.p1);
		pdat->calib.p2 = le16_to_cpu(pdat->calib.p2);
		pdat->calib.p3 = le16_to_cpu(pdat->calib.p3);
		pdat->calib.p4 = le16_to_cpu(pdat->calib.p4);
		pdat->calib.p5 = le16_to_cpu(pdat->calib.p5);
		pdat->calib.p6 = le16_to_cpu(pdat->calib.p6);
		pdat->calib.p7 = le16_to_cpu(pdat->calib.p7);
		pdat->calib.p8 = le16_to_cpu(pdat->calib.p8);
		pdat->calib.p9 = le16_to_cpu(pdat->calib.p9);
		return 1;
	}
	return 0;
}

static inline uint32_t bmp280_compensate_pressure(struct pressure_bmp280_pdata_t * pdat, int32_t praw)
{
	int64_t var1 = ((int64_t)pdat->calib.fine) - 128000;
	int64_t var2 = var1 * var1 * (int64_t)pdat->calib.p6;
	var2 = var2 + ((var1 * (int64_t)pdat->calib.p5) << 17);
	var2 = var2 + (((int64_t)pdat->calib.p4) << 35);
	var1 = ((var1 * var1 * (int64_t)pdat->calib.p3) >> 8) + ((var1 * (int64_t)pdat->calib.p2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)pdat->calib.p1) >> 33;
	if(var1 == 0)
		return 0;
	int64_t p = 1048576 - praw;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)pdat->calib.p9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)pdat->calib.p8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)pdat->calib.p7) << 4);
	return (uint32_t)p;
}

static inline int32_t bmp280_compensate_temperature(struct pressure_bmp280_pdata_t * pdat, int32_t traw)
{
	int32_t var1 = ((((traw >> 3) - ((int32_t)pdat->calib.t1 << 1))) * ((int32_t)pdat->calib.t2)) >> 11;
	int32_t var2 = (((((traw >> 4) - ((int32_t)pdat->calib.t1)) * ((traw >> 4) - ((int32_t)pdat->calib.t1))) >> 12) * ((int32_t)pdat->calib.t3)) >> 14;
	pdat->calib.fine = var1 + var2;
	int32_t t = (pdat->calib.fine * 5 + 128) >> 8;
	return t;
}

static int pressure_bmp280_get(struct pressure_t * p, int * pressure, int * temperature)
{
	struct pressure_bmp280_pdata_t * pdat = (struct pressure_bmp280_pdata_t *)p->priv;
	uint8_t buf[6];

	if(bmp280_read(pdat->dev, BMP280_PRESSURE_MSB_REG, buf, 6))
	{
		int32_t traw = (int32_t)((((uint32_t)buf[3]) << 12) | (((uint32_t)buf[4]) << 4) | ((uint32_t)buf[5] >> 4));
		int32_t praw = (int32_t)((((uint32_t)buf[0]) << 12) | (((uint32_t)buf[1]) << 4) | ((uint32_t)buf[2] >> 4));
		int celsius = bmp280_compensate_temperature(pdat, traw) * 1000 / 100;
		int pa = bmp280_compensate_pressure(pdat, praw) / 256;
		if(pressure)
			*pressure = pa;
		if(temperature)
			*temperature = celsius;
		return 1;
	}
	return 0;
}

static struct device_t * pressure_bmp280_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct pressure_bmp280_pdata_t * pdat;
	struct pressure_t * p;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x77), 0);
	if(!i2cdev)
		return NULL;

	if(!bmp280_initial(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct pressure_bmp280_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	p = xos_mem_malloc(sizeof(struct pressure_t));
	if(!p)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct pressure_bmp280_pdata_t));
	pdat->dev = i2cdev;
	bmp280_read_calib(pdat);

	p->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	p->get = pressure_bmp280_get;
	p->priv = pdat;

	if(!(dev = register_pressure(p, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(p->name);
		xos_mem_free(p->priv);
		xos_mem_free(p);
		return NULL;
	}
	return dev;
}

static void pressure_bmp280_remove(struct device_t * dev)
{
	struct pressure_t * p = (struct pressure_t *)dev->priv;
	struct pressure_bmp280_pdata_t * pdat = (struct pressure_bmp280_pdata_t *)p->priv;

	if(p)
	{
		unregister_pressure(p);
		i2c_device_free(pdat->dev);
		free_device_name(p->name);
		xos_mem_free(p->priv);
		xos_mem_free(p);
	}
}

static void pressure_bmp280_suspend(struct device_t * dev)
{
}

static void pressure_bmp280_resume(struct device_t * dev)
{
}

static struct driver_t pressure_bmp280 = {
	.name		= "pressure-bmp280",
	.probe		= pressure_bmp280_probe,
	.remove		= pressure_bmp280_remove,
	.suspend	= pressure_bmp280_suspend,
	.resume		= pressure_bmp280_resume,
};

static void pressure_bmp280_driver_init(void)
{
	register_driver(&pressure_bmp280);
}

static void pressure_bmp280_driver_exit(void)
{
	unregister_driver(&pressure_bmp280);
}

driver_initcall(pressure_bmp280_driver_init);
driver_exitcall(pressure_bmp280_driver_exit);
