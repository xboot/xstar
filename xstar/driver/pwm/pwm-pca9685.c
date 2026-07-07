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
 * PCA9685 - 16-channel PWM output with 12-bit resolution
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 * - channel: pwm channel number to be controlled
 *
 * Optional properties:
 * - slave-address: PCA9685 slave address (default: 0x40)
 * - frequency: pwm frequency from 24hz to 1526hz (default: 50hz)
 *
 * Example device tree node:
 *   "pwm-pca9685": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 64,
 *       "frequency": 50,
 *       "channel": 0
 *   }
 */

enum {
	PCA9685_REG_MODE1			= 0x00,
	PCA9685_REG_MODE2			= 0x01,
	PCA9685_REG_SUBADR1			= 0x02,
	PCA9685_REG_SUBADR2			= 0x03,
	PCA9685_REG_SUBADR3			= 0x04,
	PCA9685_REG_ALLCALLADR		= 0x05,
	PCA9685_REG_PWM0_ON_L		= 0x06,
	PCA9685_REG_PWM0_ON_H		= 0x07,
	PCA9685_REG_PWM0_OFF_L		= 0x08,
	PCA9685_REG_PWM0_OFF_H		= 0x09,
	PCA9685_REG_ALL_PWM_ON_L	= 0xfa,
	PCA9685_REG_ALL_PWM_ON_H	= 0xfb,
	PCA9685_REG_ALL_PWM_OFF_L	= 0xfc,
	PCA9685_REG_ALL_PWM_OFF_H	= 0xfd,
	PCA9685_REG_PRESCALE		= 0xfe,
};

struct pwm_pca9685_pdata_t {
	struct i2c_device_t * dev;
	int channel;

	int enable;
	int duty;
	int period;
	int polarity;
};

static int pca9685_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int pca9685_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int pca9685_initial(struct i2c_device_t * dev, int frequency)
{
	uint8_t prescale = XROUND(25000000 / (4096 * frequency)) - 1;
	uint8_t val;

	/*
	 * Prescale
	 */
	if(!pca9685_read(dev, PCA9685_REG_PRESCALE, &val, 1))
		return 0;
	if(val != prescale)
	{
		pca9685_write(dev, PCA9685_REG_MODE1, (uint8_t[]){ 1 << 4 }, 1);
		pca9685_write(dev, PCA9685_REG_PRESCALE, (uint8_t[]){ prescale }, 1);
		pca9685_write(dev, PCA9685_REG_MODE1, (uint8_t[]){ 0 << 4 }, 1);
		udelay(500);
	}

	/*
	 * Mode1
	 */
	if(!pca9685_read(dev, PCA9685_REG_MODE1, &val, 1))
		return 0;
	if(val != 0x0)
		pca9685_write(dev, PCA9685_REG_MODE1, (uint8_t[]){ 0x0 }, 1);

	/*
	 * Mode2
	 */
	if(!pca9685_read(dev, PCA9685_REG_MODE2, &val, 1))
		return 0;
	if(val != 0x4)
		pca9685_write(dev, PCA9685_REG_MODE2, (uint8_t[]){ 0x4 }, 1);

	return 1;
}

static void pca9685_refresh(struct pwm_pca9685_pdata_t * pdat)
{
	unsigned int duty;

	if(pdat->enable)
	{
		if(pdat->polarity)
			duty = (pdat->duty * 4096ULL) / pdat->period;
		else
			duty = ((pdat->period - pdat->duty) * 4096ULL) / pdat->period;
	}
	else
	{
		if(pdat->polarity)
			duty = 0;
		else
			duty = 4096;
	}

	if(duty == 0)
	{
		pca9685_write(pdat->dev, PCA9685_REG_PWM0_OFF_H + 4 * pdat->channel, (uint8_t[]){ 1 << 4 }, 1);
		return;
	}
	else if(duty >= 4096)
	{
		pca9685_write(pdat->dev, PCA9685_REG_PWM0_ON_H + 4 * pdat->channel, (uint8_t[]){ 1 << 4 }, 1);
		pca9685_write(pdat->dev, PCA9685_REG_PWM0_OFF_H + 4 * pdat->channel, (uint8_t[]){ 0 }, 1);
		return;
	}

	unsigned int on = pdat->channel * 4096 / 16;
	unsigned int off = (on + duty) % 4096;
	pca9685_write(pdat->dev, PCA9685_REG_PWM0_ON_L + 4 * pdat->channel, (uint8_t[]){ on & 0xff }, 1);
	pca9685_write(pdat->dev, PCA9685_REG_PWM0_ON_H + 4 * pdat->channel, (uint8_t[]){ (on >> 8) & 0xf }, 1);
	pca9685_write(pdat->dev, PCA9685_REG_PWM0_OFF_L + 4 * pdat->channel, (uint8_t[]){ off & 0xff }, 1);
	pca9685_write(pdat->dev, PCA9685_REG_PWM0_OFF_H + 4 * pdat->channel, (uint8_t[]){ (off >> 8) & 0xf }, 1);
}

static void pwm_pca9685_config(struct pwm_t * pwm, int duty, int period, int polarity)
{
	struct pwm_pca9685_pdata_t * pdat = (struct pwm_pca9685_pdata_t *)pwm->priv;

	if(pdat->duty != duty || pdat->period != period || pdat->polarity != polarity)
	{
		if(pdat->duty != duty)
			pdat->duty = duty;
		if(pdat->period != period)
			pdat->period = period;
		if(pdat->polarity != polarity)
			pdat->polarity = polarity;
		pca9685_refresh(pdat);
	}
}

static void pwm_pca9685_enable(struct pwm_t * pwm)
{
	struct pwm_pca9685_pdata_t * pdat = (struct pwm_pca9685_pdata_t *)pwm->priv;

	if(pdat->enable != 1)
	{
		pdat->enable = 1;
		pca9685_refresh(pdat);
	}
}

static void pwm_pca9685_disable(struct pwm_t * pwm)
{
	struct pwm_pca9685_pdata_t * pdat = (struct pwm_pca9685_pdata_t *)pwm->priv;

	if(pdat->enable != 0)
	{
		pdat->enable = 0;
		pca9685_refresh(pdat);
	}
}

static struct device_t * pwm_pca9685_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct pwm_pca9685_pdata_t * pdat;
	struct pwm_t * pwm;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;
	int frequency = dt_read_int(n, "frequency", 50);
	int channel = dt_read_int(n, "channel", -1);

	if((frequency < 24) || (frequency > 1526))
		return NULL;

	if((channel < 0) || (channel > 15))
		return NULL;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x40), 0);
	if(!i2cdev)
		return NULL;

	if(!pca9685_initial(i2cdev, frequency))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct pwm_pca9685_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pwm = xos_mem_malloc(sizeof(struct pwm_t));
	if(!pwm)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct pwm_pca9685_pdata_t));
	pdat->dev = i2cdev;
	pdat->channel = channel;
	pdat->enable = -1;
	pdat->duty = -1;
	pdat->period = -1;
	pdat->polarity = -1;

	pwm->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	pwm->config = pwm_pca9685_config;
	pwm->enable = pwm_pca9685_enable;
	pwm->disable = pwm_pca9685_disable;
	pwm->priv = pdat;

	if(!(dev = register_pwm(pwm, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(pwm->name);
		xos_mem_free(pwm->priv);
		xos_mem_free(pwm);
		return NULL;
	}
	return dev;
}

static void pwm_pca9685_remove(struct device_t * dev)
{
	struct pwm_t * pwm = (struct pwm_t *)dev->priv;
	struct pwm_pca9685_pdata_t * pdat = (struct pwm_pca9685_pdata_t *)pwm->priv;

	if(pwm)
	{
		unregister_pwm(pwm);
		i2c_device_free(pdat->dev);
		free_device_name(pwm->name);
		xos_mem_free(pwm->priv);
		xos_mem_free(pwm);
	}
}

static void pwm_pca9685_suspend(struct device_t * dev)
{
}

static void pwm_pca9685_resume(struct device_t * dev)
{
}

static struct driver_t pwm_pca9685 = {
	.name		= "pwm-pca9685",
	.probe		= pwm_pca9685_probe,
	.remove		= pwm_pca9685_remove,
	.suspend	= pwm_pca9685_suspend,
	.resume		= pwm_pca9685_resume,
};

static void pwm_pca9685_driver_init(void)
{
	register_driver(&pwm_pca9685);
}

static void pwm_pca9685_driver_exit(void)
{
	unregister_driver(&pwm_pca9685);
}

driver_initcall(pwm_pca9685_driver_init);
driver_exitcall(pwm_pca9685_driver_exit);
