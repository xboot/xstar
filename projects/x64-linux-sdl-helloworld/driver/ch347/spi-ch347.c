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

struct spi_ch347_pdata_t {
	mSPI_Master_Cfg cfg;
	uint64_t rate;
	int bits;
	int fd;
	unsigned char buf[SPI_MAX_LENGTH];
};

static const uint64_t ch347_spi_rate_tab[] = { 60000000, 30000000, 15000000, 7500000, 3750000, 1875000, 937500, 468750 };

static void ch347_spi_apply(struct spi_ch347_pdata_t * pdat)
{
	CH34X_SPIMasterInit(pdat->fd, &pdat->cfg);
	CH34X_SPISetDefaultCS(pdat->fd, 0);
}

static void ch347_spi_init(struct spi_ch347_pdata_t * pdat)
{
	xos_memset(&pdat->cfg, 0, sizeof(pdat->cfg));
	pdat->cfg.SPI_Clock = 30000000;
	pdat->cfg.SPI_OutDefaultData = 0xff;
	pdat->cfg.SPI_Mode = 0;
	pdat->cfg.SPI_CS0Init = 1;
	pdat->cfg.SPI_CS1Init = 1;
	pdat->cfg.SPI_DataBits = 0;
	pdat->cfg.SPI_BitOrder = 0;
	pdat->cfg.SPI_CS0Polar = 0;
	pdat->cfg.SPI_CS1Polar = 0;
	pdat->cfg.SPI_DataToDataDelay = 0;
	pdat->cfg.SPI_CsToDataDelay = 1;
	pdat->cfg.SPI_DataToCsDelay = 1;
	pdat->rate = 30000000;
	pdat->bits = 8;
	ch347_spi_apply(pdat);
}

static void ch347_spi_set_bits(struct spi_ch347_pdata_t * pdat, int bits)
{
	if(pdat->bits != bits)
	{
		pdat->bits = bits;
		pdat->cfg.SPI_DataBits = (pdat->bits == 16) ? 1 : 0;
		ch347_spi_apply(pdat);
	}
}

static void ch347_spi_set_rate_mode(struct spi_ch347_pdata_t * pdat, uint64_t rate, int mode)
{
	int changed = 0;

	if(pdat->rate != rate)
	{
		for(int i = 0; i < ARRAY_SIZE(ch347_spi_rate_tab); i++)
		{
			if(rate >= ch347_spi_rate_tab[i])
			{
				pdat->cfg.SPI_Clock = ch347_spi_rate_tab[i];
				pdat->rate = rate;
				changed = 1;
				break;
			}
		}
	}
	if(pdat->cfg.SPI_Mode != mode)
	{
		pdat->cfg.SPI_Mode = mode;
		changed = 1;
	}
	if(changed)
		ch347_spi_apply(pdat);
}

static int ch347_spi_xfer(struct spi_ch347_pdata_t * pdat, struct spi_msg_t * msg)
{
	unsigned char * tx;
	unsigned char * rx;
	int len, n, ret = 0;

	if(msg->len <= 0)
		return 0;
	tx = (unsigned char *)msg->txbuf;
	rx = (unsigned char *)msg->rxbuf;
	len = msg->len;
	while(len > 0)
	{
		n = (len > SPI_MAX_LENGTH) ? SPI_MAX_LENGTH : len;
		if(tx)
			xos_memcpy(pdat->buf, tx, n);
		else
			xos_memset(pdat->buf, pdat->cfg.SPI_OutDefaultData, n);
		if(CH34X_SPIWriteRead(pdat->fd, 0, 0, pdat->buf, n) != 0)
			break;
		if(rx)
			xos_memcpy(rx, pdat->buf, n);
		if(tx)
			tx += n;
		if(rx)
			rx += n;
		len -= n;
		ret += n;
	}
	return ret;
}

static int spi_ch347_transfer(struct spi_t * spi, struct spi_msg_t * msg)
{
	struct spi_ch347_pdata_t * pdat = (struct spi_ch347_pdata_t *)spi->priv;
	int ret;

	ch347_lock();
	{
		ch347_spi_set_bits(pdat, msg->bits);
		ch347_spi_set_rate_mode(pdat, (msg->speed > 0) ? msg->speed : 30000000, msg->mode);
		ret = ch347_spi_xfer(pdat, msg);
	}
	ch347_unlock();
	return ret;
}

static void spi_ch347_select(struct spi_t * spi, int cs)
{
	struct spi_ch347_pdata_t * pdat = (struct spi_ch347_pdata_t *)spi->priv;

	ch347_lock();
	{
		CH34X_SPIChangeCS(pdat->fd, cs, 1);
	}
	ch347_unlock();
}

static void spi_ch347_deselect(struct spi_t * spi, int cs)
{
	struct spi_ch347_pdata_t * pdat = (struct spi_ch347_pdata_t *)spi->priv;

	ch347_lock();
	{
		CH34X_SPIChangeCS(pdat->fd, cs, 0);
	}
	ch347_unlock();
}

static struct device_t * spi_ch347_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct spi_ch347_pdata_t * pdat;
	struct spi_t * spi;
	struct device_t * dev;
	int fd;

	fd = ch347_detect();
	if(fd <= 0)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct spi_ch347_pdata_t));
	if(!pdat)
		return NULL;

	spi = xos_mem_malloc(sizeof(struct spi_t));
	if(!spi)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->fd = fd;

	spi->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	spi->type = SPI_TYPE_SINGLE;
	spi->transfer = spi_ch347_transfer;
	spi->select = spi_ch347_select;
	spi->deselect = spi_ch347_deselect;
	spi->priv = pdat;

	ch347_spi_init(pdat);

	if(!(dev = register_spi(spi, drv)))
	{
		free_device_name(spi->name);
		xos_mem_free(spi->priv);
		xos_mem_free(spi);
		return NULL;
	}
	return dev;
}

static void spi_ch347_remove(struct device_t * dev)
{
	struct spi_t * spi = (struct spi_t *)dev->priv;

	if(spi)
	{
		unregister_spi(spi);
		free_device_name(spi->name);
		xos_mem_free(spi->priv);
		xos_mem_free(spi);
	}
}

static void spi_ch347_suspend(struct device_t * dev)
{
}

static void spi_ch347_resume(struct device_t * dev)
{
}

static struct driver_t spi_ch347 = {
	.name		= "spi-ch347",
	.probe		= spi_ch347_probe,
	.remove		= spi_ch347_remove,
	.suspend	= spi_ch347_suspend,
	.resume		= spi_ch347_resume,
};

static void spi_ch347_driver_init(void)
{
	register_driver(&spi_ch347);
}

static void spi_ch347_driver_exit(void)
{
	unregister_driver(&spi_ch347);
}

driver_initcall(spi_ch347_driver_init);
driver_exitcall(spi_ch347_driver_exit);
