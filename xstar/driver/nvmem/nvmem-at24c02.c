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

#include <kernel/time/delay.h>
#include <driver/i2c/i2c.h>
#include <driver/nvmem/nvmem.h>

/*
 * at24c02 - 2kbit(256x8) i2c serial eeprom
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: i2c slave address (default: 0x50)
 *
 * Example:
 *   "nvmem-at24c02": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 80
 *   }
 */

struct nvmem_at24c02_pdata_t {
	struct i2c_device_t * dev;
	int capacity;
};

static int at24c02_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int at24c02_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int nvmem_at24c02_capacity(struct nvmem_t * m)
{
	struct nvmem_at24c02_pdata_t * pdat = (struct nvmem_at24c02_pdata_t *)m->priv;
	return pdat->capacity;
}

static int nvmem_at24c02_read(struct nvmem_t * m, void * buf, int offset, int count)
{
	struct nvmem_at24c02_pdata_t * pdat = (struct nvmem_at24c02_pdata_t *)m->priv;
	uint8_t * pbuf = buf;
	int length = 0;

	while(count > 0)
	{
		int chunk = XMIN(count, 32);
		if(!at24c02_read(pdat->dev, offset, pbuf, chunk))
			break;
		count -= chunk;
		pbuf += chunk;
		offset += chunk;
		length += chunk;
	}
	return length;
}

static int nvmem_at24c02_write(struct nvmem_t * m, void * buf, int offset, int count)
{
	struct nvmem_at24c02_pdata_t * pdat = (struct nvmem_at24c02_pdata_t *)m->priv;
	uint8_t * pbuf = buf;
	int length = 0;

	while(count > 0)
	{
		int chunk = XMIN(count, 8 - (offset & 0x7));
		if(!at24c02_write(pdat->dev, offset, pbuf, chunk))
			break;
		count -= chunk;
		pbuf += chunk;
		length += chunk;
		offset += chunk;
		mdelay(5);
	}
	return length;
}

static struct device_t * nvmem_at24c02_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct nvmem_at24c02_pdata_t * pdat;
	struct nvmem_t * m;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;
	uint8_t val;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x50), 0);
	if(!i2cdev)
		return NULL;

	if(!at24c02_read(i2cdev, 0, &val, 1))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct nvmem_at24c02_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	m = xos_mem_malloc(sizeof(struct nvmem_t));
	if(!m)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->dev = i2cdev;
	pdat->capacity = 256;

	m->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	m->capacity = nvmem_at24c02_capacity;
	m->read = nvmem_at24c02_read;
	m->write = nvmem_at24c02_write;
	m->priv = pdat;

	if(!(dev = register_nvmem(m, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(m->name);
		xos_mem_free(m->priv);
		xos_mem_free(m);
		return NULL;
	}
	return dev;
}

static void nvmem_at24c02_remove(struct device_t * dev)
{
	struct nvmem_t * m = (struct nvmem_t *)dev->priv;
	struct nvmem_at24c02_pdata_t * pdat = (struct nvmem_at24c02_pdata_t *)m->priv;

	if(m)
	{
		unregister_nvmem(m);
		i2c_device_free(pdat->dev);
		free_device_name(m->name);
		xos_mem_free(m->priv);
		xos_mem_free(m);
	}
}

static void nvmem_at24c02_suspend(struct device_t * dev)
{
}

static void nvmem_at24c02_resume(struct device_t * dev)
{
}

static struct driver_t nvmem_at24c02 = {
	.name		= "nvmem-at24c02",
	.probe		= nvmem_at24c02_probe,
	.remove		= nvmem_at24c02_remove,
	.suspend	= nvmem_at24c02_suspend,
	.resume		= nvmem_at24c02_resume,
};

static void nvmem_at24c02_driver_init(void)
{
	register_driver(&nvmem_at24c02);
}

static void nvmem_at24c02_driver_exit(void)
{
	unregister_driver(&nvmem_at24c02);
}

driver_initcall(nvmem_at24c02_driver_init);
driver_exitcall(nvmem_at24c02_driver_exit);
