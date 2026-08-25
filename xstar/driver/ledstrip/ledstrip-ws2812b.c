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

#include <driver/spi/spi.h>
#include <driver/ledstrip/ledstrip.h>

/*
 * WS2812B - WS2812B LED Strip Driver
 *
 * Example:
 *   "ledstrip-ws2812b": {
 *       "spi-bus": "spi-f1c100s.0",
 *       "chip-select": 0,
 *       "mode": 3,
 *       "speed": 6400000,
 *       "count": 10
 *   }
 */

struct ledstrip_ws2812b_pdata_t {
	struct spi_device_t * spidev;
	int count;
	struct color_t * color;
	int buflen;
	unsigned char * buffer;
};

static void ledstrip_ws2812b_set_count(struct ledstrip_t * strip, int n)
{
	struct ledstrip_ws2812b_pdata_t * pdat = (struct ledstrip_ws2812b_pdata_t *)strip->priv;

	if((n != pdat->count) && (n > 0))
	{
		if(pdat->color)
			xos_mem_free(pdat->color);
		if(pdat->buffer)
			xos_mem_free(pdat->buffer);
		pdat->count = n;
		pdat->color = xos_mem_malloc(pdat->count * sizeof(struct color_t));
		xos_memset(pdat->color, 0, pdat->count * sizeof(struct color_t));
		pdat->buflen = 8 + n * 24 + 240;
		pdat->buffer = xos_mem_malloc(pdat->buflen);
		xos_memset(&pdat->buffer[0], 0, 8);
		xos_memset(&pdat->buffer[8], 0xc0, n * 24);
		xos_memset(&pdat->buffer[8 + n * 24], 0, 240);
	}
}

static int ledstrip_ws2812b_get_count(struct ledstrip_t * strip)
{
	struct ledstrip_ws2812b_pdata_t * pdat = (struct ledstrip_ws2812b_pdata_t *)strip->priv;
	return pdat->count;
}

static void ledstrip_ws2812b_set_color(struct ledstrip_t * strip, int i, struct color_t * c)
{
	struct ledstrip_ws2812b_pdata_t * pdat = (struct ledstrip_ws2812b_pdata_t *)strip->priv;
	unsigned char * p = &pdat->buffer[8 + i * 24];
	int n;

	xos_memcpy(&pdat->color[i], c, sizeof(struct color_t));
	for(n = 0; n < 8; n++)
		*p++ = (c->g & (0x80 >> n)) ? 0xfc: 0xc0;
	for(n = 0; n < 8; n++)
		*p++ = (c->r & (0x80 >> n)) ? 0xfc: 0xc0;
	for(n = 0; n < 8; n++)
		*p++ = (c->b & (0x80 >> n)) ? 0xfc: 0xc0;
}

static void ledstrip_ws2812b_get_color(struct ledstrip_t * strip, int i, struct color_t * c)
{
	struct ledstrip_ws2812b_pdata_t * pdat = (struct ledstrip_ws2812b_pdata_t *)strip->priv;
	xos_memcpy(c, &pdat->color[i], sizeof(struct color_t));
}

static void ledstrip_ws2812b_refresh(struct ledstrip_t * strip)
{
	struct ledstrip_ws2812b_pdata_t * pdat = (struct ledstrip_ws2812b_pdata_t *)strip->priv;
	spi_device_write_then_read(pdat->spidev, pdat->buffer, pdat->buflen, NULL, 0);
}

static struct device_t * ledstrip_ws2812b_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct ledstrip_ws2812b_pdata_t * pdat;
	struct ledstrip_t * strip;
	struct device_t * dev;
	struct spi_device_t * spidev;

	spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 3), 8, dt_read_int(n, "speed", 6400000));
	if(!spidev)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct ledstrip_ws2812b_pdata_t));
	if(!pdat)
		return NULL;

	strip = xos_mem_malloc(sizeof(struct ledstrip_t));
	if(!strip)
	{
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->spidev = spidev;
	pdat->count = 0;
	pdat->color = NULL;
	pdat->buflen = 0;
	pdat->buffer = NULL;

	strip->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	strip->set_count = ledstrip_ws2812b_set_count;
	strip->get_count = ledstrip_ws2812b_get_count;
	strip->set_color = ledstrip_ws2812b_set_color;
	strip->get_color = ledstrip_ws2812b_get_color;
	strip->refresh = ledstrip_ws2812b_refresh;
	strip->priv = pdat;

	ledstrip_ws2812b_set_count(strip, dt_read_int(n, "count", 1));
	ledstrip_ws2812b_refresh(strip);

	if(!(dev = register_ledstrip(strip, drv)))
	{
		free_device_name(strip->name);
		xos_mem_free(strip->priv);
		xos_mem_free(strip);
		return NULL;
	}
	return dev;
}

static void ledstrip_ws2812b_remove(struct device_t * dev)
{
	struct ledstrip_t * strip = (struct ledstrip_t *)dev->priv;
	struct ledstrip_ws2812b_pdata_t * pdat = (struct ledstrip_ws2812b_pdata_t *)strip->priv;

	if(strip)
	{
		unregister_ledstrip(strip);
		if(pdat->color)
			xos_mem_free(pdat->color);
		free_device_name(strip->name);
		xos_mem_free(strip->priv);
		xos_mem_free(strip);
	}
}

static void ledstrip_ws2812b_suspend(struct device_t * dev)
{
}

static void ledstrip_ws2812b_resume(struct device_t * dev)
{
}

static struct driver_t ledstrip_ws2812b = {
	.name		= "ledstrip-ws2812b",
	.probe		= ledstrip_ws2812b_probe,
	.remove		= ledstrip_ws2812b_remove,
	.suspend	= ledstrip_ws2812b_suspend,
	.resume		= ledstrip_ws2812b_resume,
};

static void ledstrip_ws2812b_driver_init(void)
{
	register_driver(&ledstrip_ws2812b);
}

static void ledstrip_ws2812b_driver_exit(void)
{
	unregister_driver(&ledstrip_ws2812b);
}

driver_initcall(ledstrip_ws2812b_driver_init);
driver_exitcall(ledstrip_ws2812b_driver_exit);
