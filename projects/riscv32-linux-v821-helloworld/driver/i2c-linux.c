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

#include <linux/linux.h>

struct i2c_linux_pdata_t {
	struct mutex_t lock;
	int fd;
};

static int i2c_linux_xfer(struct i2c_t * i2c, struct i2c_msg_t * msgs, int num)
{
	struct i2c_linux_pdata_t * pdat = (struct i2c_linux_pdata_t *)i2c->priv;
	struct linux_i2c_msg_t msg[num];
	int ret;

	for(int i = 0; i < num; i++)
	{
		msg[i].addr = msgs[i].addr;
		msg[i].flags = msgs[i].flags;
		msg[i].len = msgs[i].len;
		msg[i].buf = msgs[i].buf;
	}
	xos_mutex_lock(&pdat->lock);
	{
		ret = linux_i2c_xfer(pdat->fd, msg, num);
	}
	xos_mutex_unlock(&pdat->lock);
	return ret;
}

static struct device_t * i2c_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct i2c_linux_pdata_t * pdat;
	struct i2c_t * i2c;
	struct device_t * dev;
	char * path = dt_read_string(n, "device", NULL);
	int fd;

	if(!linux_i2c_exist(path))
		return NULL;

	fd = linux_i2c_open(path);
	if(fd <= 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct i2c_linux_pdata_t));
	if(!pdat)
		return NULL;

	i2c = xos_mem_malloc(sizeof(struct i2c_t));
	if(!i2c)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	xos_mutex_init(&pdat->lock);
	pdat->fd = fd;

	i2c->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	i2c->xfer = i2c_linux_xfer;
	i2c->priv = pdat;

	if(!(dev = register_i2c(i2c, drv)))
	{
		xos_mutex_exit(&pdat->lock);
		linux_i2c_close(pdat->fd);
		free_device_name(i2c->name);
		xos_mem_free(i2c->priv);
		xos_mem_free(i2c);
		return NULL;
	}
	return dev;
}

static void i2c_linux_remove(struct device_t * dev)
{
	struct i2c_t * i2c = (struct i2c_t *)dev->priv;
	struct i2c_linux_pdata_t * pdat = (struct i2c_linux_pdata_t *)i2c->priv;

	if(i2c)
	{
		unregister_i2c(i2c);
		xos_mutex_exit(&pdat->lock);
		linux_i2c_close(pdat->fd);
		free_device_name(i2c->name);
		xos_mem_free(i2c->priv);
		xos_mem_free(i2c);
	}
}

static void i2c_linux_suspend(struct device_t * dev)
{
}

static void i2c_linux_resume(struct device_t * dev)
{
}

static struct driver_t i2c_linux = {
	.name		= "i2c-linux",
	.probe		= i2c_linux_probe,
	.remove		= i2c_linux_remove,
	.suspend	= i2c_linux_suspend,
	.resume		= i2c_linux_resume,
};

static void i2c_linux_driver_init(void)
{
	register_driver(&i2c_linux);
}

static void i2c_linux_driver_exit(void)
{
	unregister_driver(&i2c_linux);
}

driver_initcall(i2c_linux_driver_init);
driver_exitcall(i2c_linux_driver_exit);
