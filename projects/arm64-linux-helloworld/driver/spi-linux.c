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

struct spi_linux_pdata_t {
	struct mutex_t lock;
	int fd;
	int mode;
};

static int spi_linux_transfer(struct spi_t * spi, struct spi_msg_t * msg)
{
	struct spi_linux_pdata_t * pdat = (struct spi_linux_pdata_t *)spi->priv;
	struct linux_spi_msg_t tmsg;
	int ret;

	if(pdat->mode != msg->mode)
	{
		xos_mutex_lock(&pdat->lock);
		{
			pdat->mode = msg->mode;
			linux_spi_mode(pdat->fd, pdat->mode);
		}
		xos_mutex_unlock(&pdat->lock);
	}

	tmsg.txbuf = msg->txbuf;
	tmsg.rxbuf = msg->rxbuf;
	tmsg.len = msg->len;
	tmsg.type = msg->type;
	tmsg.bits = msg->bits;
	tmsg.speed = msg->speed;
	xos_mutex_lock(&pdat->lock);
	{
		ret = linux_spi_transfer(pdat->fd, &tmsg);
	}
	xos_mutex_unlock(&pdat->lock);
	return ret;
}

static void spi_linux_select(struct spi_t * spi, int cs)
{
	struct spi_linux_pdata_t * pdat = (struct spi_linux_pdata_t *)spi->priv;

	xos_mutex_lock(&pdat->lock);
	{
		linux_spi_select(pdat->fd, cs);
	}
	xos_mutex_unlock(&pdat->lock);
}

static void spi_linux_deselect(struct spi_t * spi, int cs)
{
	struct spi_linux_pdata_t * pdat = (struct spi_linux_pdata_t *)spi->priv;

	xos_mutex_lock(&pdat->lock);
	{
		linux_spi_deselect(pdat->fd, cs);
	}
	xos_mutex_unlock(&pdat->lock);
}

static struct device_t * spi_linux_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct spi_linux_pdata_t * pdat;
	struct spi_t * spi;
	struct device_t * dev;
	char * path = dt_read_string(n, "device", NULL);
	int fd;

	if(!linux_spi_exist(path))
		return NULL;

	fd = linux_spi_open(path);
	if(fd <= 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct spi_linux_pdata_t));
	if(!pdat)
		return NULL;

	spi = xos_mem_malloc(sizeof(struct spi_t));
	if(!spi)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	xos_mutex_init(&pdat->lock);
	pdat->fd = fd;
	pdat->mode = -1;

	spi->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	spi->type = SPI_TYPE_SINGLE | SPI_TYPE_DUAL | SPI_TYPE_QUAD | SPI_TYPE_OCTAL;
	spi->transfer = spi_linux_transfer;
	spi->select = spi_linux_select;
	spi->deselect = spi_linux_deselect;
	spi->priv = pdat;
	if(!(dev = register_spi(spi, drv)))
	{
		xos_mutex_exit(&pdat->lock);
		linux_spi_close(pdat->fd);
		free_device_name(spi->name);
		xos_mem_free(spi->priv);
		xos_mem_free(spi);
		return NULL;
	}
	return dev;
}

static void spi_linux_remove(struct device_t * dev)
{
	struct spi_t * spi = (struct spi_t *)dev->priv;
	struct spi_linux_pdata_t * pdat = (struct spi_linux_pdata_t *)spi->priv;

	if(spi)
	{
		unregister_spi(spi);
		xos_mutex_exit(&pdat->lock);
		linux_spi_close(pdat->fd);
		free_device_name(spi->name);
		xos_mem_free(spi->priv);
		xos_mem_free(spi);
	}
}

static void spi_linux_suspend(struct device_t * dev)
{
}

static void spi_linux_resume(struct device_t * dev)
{
}

static struct driver_t spi_linux = {
	.name		= "spi-linux",
	.probe		= spi_linux_probe,
	.remove		= spi_linux_remove,
	.suspend	= spi_linux_suspend,
	.resume		= spi_linux_resume,
};

static void spi_linux_driver_init(void)
{
	register_driver(&spi_linux);
}

static void spi_linux_driver_exit(void)
{
	unregister_driver(&spi_linux);
}

driver_initcall(spi_linux_driver_init);
driver_exitcall(spi_linux_driver_exit);
