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
 * ADS1015 - 12-bit adc with single-ended or differential
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: ADS1015 slave address (default: 0x48)
 * - gain: programmable gain amplifier (0-5)
 * - rate: data rate setting (0-6)
 * - differential: input mode configuration
 *   true  = differential mode (AIN0-AIN1, AIN0-AIN3, AIN1-AIN3, AIN2-AIN3)
 *   false = single-ended mode (AIN0-GND, AIN1-GND, AIN2-GND, AIN3-GND)
 *
 * Example device tree node:
 *   "adc-ads1015": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 72,
 *       "gain": 1,
 *       "rate": 4,
 *       "differential": false
 *   }
 */

enum {
	ADS1015_REG_CONVERT		= 0x00,
	ADS1015_REG_CONFIG		= 0x01,
	ADS1015_REG_LOTHRESH	= 0x02,
	ADS1015_REG_HITHRESH	= 0x03,
};

enum {
	ADS1015_PGA_6_144V  	= 0x00,	/* +/-6.144V range = Gain 2/3 */
	ADS1015_PGA_4_096V		= 0x01,	/* +/-4.096V range = Gain   1 */
	ADS1015_PGA_2_048V		= 0x02,	/* +/-2.048V range = Gain   2 */
	ADS1015_PGA_1_024V		= 0x03,	/* +/-1.024V range = Gain   4 */
	ADS1015_PGA_0_512V		= 0x04,	/* +/-0.512V range = Gain   8 */
	ADS1015_PGA_0_256V		= 0x05,	/* +/-0.256V range = Gain  16 */
};

enum {
	ADS1015_SPS_128   		= 0x00,	/*  128 samples per second */
	ADS1015_SPS_250   		= 0x01,	/*  250 samples per second */
	ADS1015_SPS_490   		= 0x02,	/*  490 samples per second */
	ADS1015_SPS_920   		= 0x03,	/*  920 samples per second */
	ADS1015_SPS_1600   		= 0x04,	/* 1600 samples per second */
	ADS1015_SPS_2400   		= 0x05,	/* 2400 samples per second */
	ADS1015_SPS_3300   		= 0x06,	/* 3300 samples per second */
};

struct adc_ads1015_pdata_t {
	struct i2c_device_t * dev;
	int gain;
	int rate;
	int diff;
	int delay;
};

static int ads1015_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int ads1015_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int ads1015_compute_delay(int rate)
{
	static const int dr[] = {
		128, 250, 490, 920, 1600, 2400, 3300, 3300,
	};
	int us = (1000000U + dr[rate] - 1) / dr[rate];
	if(us < 1000)
		us = 1000;
	return us + 200;
}

static int ads1015_compute_vreference(int gain)
{
	static const int vref[] = {
		6144000, 4096000, 2048000, 1024000, 512000, 256000, 256000, 256000,
	};
	return vref[gain];
}

static int ads1015_initial(struct i2c_device_t * dev)
{
	uint8_t buf[2];

	if(!ads1015_read(dev, ADS1015_REG_CONFIG, buf, 2))
		return 0;
	return 1;
}

static int32_t adc_ads1015_read(struct adc_t * adc, int channel)
{
	struct adc_ads1015_pdata_t * pdat = (struct adc_ads1015_pdata_t *)adc->priv;
	uint8_t buf[2];

	uint16_t cfg = 0;
	cfg |= 0 << 15;
	cfg |= ((pdat->diff ? 0 : 1) << 14) | ((channel & 0x3) << 12);
	cfg |= pdat->gain << 9;
	cfg |= 0 << 8;
	cfg |= pdat->rate << 5;
	cfg |= 0x3 << 0;
	ads1015_write(pdat->dev, ADS1015_REG_CONFIG, (uint8_t[]){ (cfg >> 8) & 0xff, (cfg >> 0) & 0xff }, 2);

	udelay(pdat->delay);
	ads1015_read(pdat->dev, ADS1015_REG_CONVERT, buf, 2);
	int16_t raw = ((int16_t)(((uint16_t)buf[0] << 8) | buf[1])) >> 4;

	return (int32_t)raw;
}

static struct device_t * adc_ads1015_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct adc_ads1015_pdata_t * pdat;
	struct adc_t * adc;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x48), 0);
	if(!i2cdev)
		return NULL;

	if(!ads1015_initial(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct adc_ads1015_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	adc = xos_mem_malloc(sizeof(struct adc_t));
	if(!adc)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct adc_ads1015_pdata_t));
	pdat->dev = i2cdev;
	pdat->gain =  dt_read_int(n, "gain", ADS1015_PGA_4_096V) & 0x7;
	pdat->rate =  dt_read_int(n, "rate", ADS1015_SPS_1600) & 0x7;
	pdat->diff =  dt_read_bool(n, "differential", 0);
	pdat->delay = ads1015_compute_delay(pdat->rate);

	adc->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	adc->vreference = ads1015_compute_vreference(pdat->gain);
	adc->resolution = 11;
	adc->nchannel = 4;
	adc->read = adc_ads1015_read;
	adc->priv = pdat;

	if(!(dev = register_adc(adc, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(adc->name);
		xos_mem_free(adc->priv);
		xos_mem_free(adc);
		return NULL;
	}
	return dev;
}

static void adc_ads1015_remove(struct device_t * dev)
{
	struct adc_t * adc = (struct adc_t *)dev->priv;
	struct adc_ads1015_pdata_t * pdat = (struct adc_ads1015_pdata_t *)adc->priv;

	if(adc)
	{
		unregister_adc(adc);
		i2c_device_free(pdat->dev);
		free_device_name(adc->name);
		xos_mem_free(adc->priv);
		xos_mem_free(adc);
	}
}

static void adc_ads1015_suspend(struct device_t * dev)
{
}

static void adc_ads1015_resume(struct device_t * dev)
{
}

static struct driver_t adc_ads1015 = {
	.name		= "adc-ads1015",
	.probe		= adc_ads1015_probe,
	.remove		= adc_ads1015_remove,
	.suspend	= adc_ads1015_suspend,
	.resume		= adc_ads1015_resume,
};

static void adc_ads1015_driver_init(void)
{
	register_driver(&adc_ads1015);
}

static void adc_ads1015_driver_exit(void)
{
	unregister_driver(&adc_ads1015);
}

driver_initcall(adc_ads1015_driver_init);
driver_exitcall(adc_ads1015_driver_exit);
