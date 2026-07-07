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

#include <ch347.h>

struct i2c_ch347_pdata_t {
	int fd;
};

static int i2c_ch347_xfer(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num)
{
	struct i2c_ch347_pdata_t * pdat = (struct i2c_ch347_pdata_t *)i2c->priv;
	unsigned char wbuf[256 + 1];
	int wlen = 0;
	void * rbuf = NULL;
	int rlen = 0;
	int ret;

	for(int i = 0; i < num; i++)
	{
		struct i2c_msg_t * pmsg = &msgs[i];
		if(wlen == 0)
		{
			wbuf[wlen++] = pmsg->addr << 1;
		}
		if(pmsg->flags & I2C_MODE_RD)
		{
			if(pmsg->len > 0)
			{
				rbuf = pmsg->buf;
				rlen = pmsg->len;
			}
		}
		else
		{
			if(pmsg->len > 0)
			{
				xos_memcpy(&wbuf[wlen], pmsg->buf, pmsg->len);
				wlen += pmsg->len;
			}
		}
	}
	ch347_lock();
	{
		ret = CH347StreamI2C(pdat->fd, wlen, wbuf, rlen, rbuf) ? num : 0;
	}
	ch347_unlock();
	return ret;
}

static struct device_t * i2c_ch347_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct i2c_ch347_pdata_t * pdat;
	struct i2c_t * i2c;
	struct device_t * dev;
	int fd;

	fd = ch347_detect();
	if(fd <= 0)
		return NULL;
	CH347I2C_Set(fd, 0x1);
	CH347I2C_SetStretch(fd, 1);
	CH347I2C_SetDriveMode(fd, 1);

	pdat = xos_mem_malloc(sizeof(struct i2c_ch347_pdata_t));
	if(!pdat)
		return NULL;

	i2c = xos_mem_malloc(sizeof(struct i2c_t));
	if(!i2c)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->fd = fd;

	i2c->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	i2c->xfer = i2c_ch347_xfer;
	i2c->priv = pdat;

	if(!(dev = register_i2c(i2c, drv)))
	{
		free_device_name(i2c->name);
		xos_mem_free(i2c->priv);
		xos_mem_free(i2c);
		return NULL;
	}
	return dev;
}

static void i2c_ch347_remove(struct device_t * dev)
{
	struct i2c_t * i2c = (struct i2c_t *)dev->priv;

	if(i2c)
	{
		unregister_i2c(i2c);
		free_device_name(i2c->name);
		xos_mem_free(i2c->priv);
		xos_mem_free(i2c);
	}
}

static void i2c_ch347_suspend(struct device_t * dev)
{
}

static void i2c_ch347_resume(struct device_t * dev)
{
}

static struct driver_t i2c_ch347 = {
	.name		= "i2c-ch347",
	.probe		= i2c_ch347_probe,
	.remove		= i2c_ch347_remove,
	.suspend	= i2c_ch347_suspend,
	.resume		= i2c_ch347_resume,
};

static void i2c_ch347_driver_init(void)
{
	register_driver(&i2c_ch347);
}

static void i2c_ch347_driver_exit(void)
{
	unregister_driver(&i2c_ch347);
}

driver_initcall(i2c_ch347_driver_init);
driver_exitcall(i2c_ch347_driver_exit);
