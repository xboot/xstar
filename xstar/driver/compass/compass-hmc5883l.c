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
 * HMC5883L - Triple-Axis Digital Compass (Magnetometer)
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: HMC5883L compass slave address (default: 0x1e)
 *
 * Example device tree node:
 *   "compass-hmc5883l": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 30
 *   }
 */

enum {
	REG_CFGA 	= 0x00,
	REG_CFGB	= 0x01,
	REG_MODE	= 0x02,
	REG_DATAXH 	= 0x03,
	REG_DATAXL 	= 0x04,
	REG_DATAZH 	= 0x05,
	REG_DATAZL	= 0x06,
	REG_DATAYH 	= 0x07,
	REG_DATAYL 	= 0x08,
	REG_STATUS	= 0x09,
	REG_IDA		= 0x0a,
	REG_IDB		= 0x0b,
	REG_IDC		= 0x0c,
};

struct compass_hmc5883l_pdata_t {
	struct i2c_device_t * dev;
};

static int hmc5883l_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int hmc5883l_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int hmc5883l_initial(struct i2c_device_t * dev)
{
	uint8_t id[3];

	if(!hmc5883l_read(dev, REG_IDA, id, 3) || (id[0] != 0x48) || (id[1] != 0x34) || (id[2] != 0x33))
		return 0;
	hmc5883l_write(dev, REG_CFGA, (uint8_t[]){ 0x70 }, 1);
	hmc5883l_write(dev, REG_CFGB, (uint8_t[]){ 0x20 }, 1);
	hmc5883l_write(dev, REG_MODE, (uint8_t[]){ 0x00 }, 1);
	return 1;
}

static int compass_hmc5883l_get(struct compass_t * c, int * x, int * y, int * z)
{
	struct compass_hmc5883l_pdata_t * pdat = (struct compass_hmc5883l_pdata_t *)c->priv;
	uint8_t status, buf[6];
	int16_t tx, ty, tz;

	if(hmc5883l_read(pdat->dev, REG_STATUS, &status, 1) && (status & (1 << 0)))
	{
		hmc5883l_read(pdat->dev, REG_DATAXH, buf, 6);
		tx = (buf[0] << 8) | (buf[1] << 0);
		ty = (buf[4] << 8) | (buf[5] << 0);
		tz = (buf[2] << 8) | (buf[3] << 0);

		*x = (int64_t)tx * 1000000 / 1090;
		*y = (int64_t)ty * 1000000 / 1090;
		*z = (int64_t)tz * 1000000 / 1090;
		return 1;
	}
	return 0;
}

static struct device_t * compass_hmc5883l_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct compass_hmc5883l_pdata_t * pdat;
	struct compass_t * c;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x1e), 0);
	if(!i2cdev)
		return NULL;

	if(!hmc5883l_initial(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct compass_hmc5883l_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	c = xos_mem_malloc(sizeof(struct compass_t));
	if(!c)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->dev = i2cdev;

	c->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	c->ox = 0;
	c->oy = 0;
	c->oz = 0;
	c->get = compass_hmc5883l_get;
	c->priv = pdat;

	if(!(dev = register_compass(c, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(c->name);
		xos_mem_free(c->priv);
		xos_mem_free(c);
		return NULL;
	}
	return dev;
}

static void compass_hmc5883l_remove(struct device_t * dev)
{
	struct compass_t * c = (struct compass_t *)dev->priv;
	struct compass_hmc5883l_pdata_t * pdat = (struct compass_hmc5883l_pdata_t *)c->priv;

	if(c)
	{
		unregister_compass(c);
		i2c_device_free(pdat->dev);
		free_device_name(c->name);
		xos_mem_free(c->priv);
		xos_mem_free(c);
	}
}

static void compass_hmc5883l_suspend(struct device_t * dev)
{
}

static void compass_hmc5883l_resume(struct device_t * dev)
{
}

static struct driver_t compass_hmc5883l = {
	.name		= "compass-hmc5883l",
	.probe		= compass_hmc5883l_probe,
	.remove		= compass_hmc5883l_remove,
	.suspend	= compass_hmc5883l_suspend,
	.resume		= compass_hmc5883l_resume,
};

static void compass_hmc5883l_driver_init(void)
{
	register_driver(&compass_hmc5883l);
}

static void compass_hmc5883l_driver_exit(void)
{
	unregister_driver(&compass_hmc5883l);
}

driver_initcall(compass_hmc5883l_driver_init);
driver_exitcall(compass_hmc5883l_driver_exit);
