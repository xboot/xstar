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
 * LCD - Fitipower JD9851 Lcd Driver
 *
 * Example:
 *	"fb-jd9851": {
 *		"spi-bus": "spi-f101.1",
 *		"chip-select": 0,
 *		"type": 0,
 *		"mode": 0,
 *		"speed": 100000000,
 *		"reset-gpio": 100,
 *		"reset-gpio-config": 1,
 *		"cd-gpio": 101,
 *		"cd-gpio-config": 1,
 *		"width": 240,
 *		"height": 240,
 *		"physical-width": 25,
 *		"physical-height": 25,
 *		"backlight": null
 *	}
 */

struct fb_jd9851_pdata_t {
	struct spi_device_t * dev;
	int rst;
	int rstcfg;
	int cd;
	int cdcfg;
	int width;
	int height;
	int pwidth;
	int pheight;
	uint8_t * txbuf;

	struct led_t * backlight;
	int brightness;
};

static void jd9851_write_command(struct fb_jd9851_pdata_t * pdat, uint8_t cmd)
{
	spi_device_select(pdat->dev);
	gpio_set_value(pdat->cd, 0);
	spi_device_write_then_read(pdat->dev, &cmd, 1, 0, 0);
	gpio_set_value(pdat->cd, 1);
	spi_device_deselect(pdat->dev);
}

static void jd9851_write_data(struct fb_jd9851_pdata_t * pdat, uint8_t dat)
{
	spi_device_select(pdat->dev);
	spi_device_write_then_read(pdat->dev, &dat, 1, 0, 0);
	spi_device_deselect(pdat->dev);
}

static void jd9851_set_window(struct fb_jd9851_pdata_t * pdat, int x, int y, int w, int h)
{
	jd9851_write_command(pdat, 0x2a);
	jd9851_write_data(pdat, (x >> 8) & 0xff);
	jd9851_write_data(pdat, (x >> 0) & 0xff);
	jd9851_write_data(pdat, ((x + w - 1) >> 8) & 0xff);
	jd9851_write_data(pdat, ((x + w - 1) >> 0) & 0xff);
	jd9851_write_command(pdat, 0x2b);
	jd9851_write_data(pdat, (y >> 8) & 0xff);
	jd9851_write_data(pdat, (y >> 0) & 0xff);
	jd9851_write_data(pdat, ((y + h - 1) >> 8) & 0xff);
	jd9851_write_data(pdat, ((y + h - 1) >> 0) & 0xff);
}

static void jd9851_init(struct fb_jd9851_pdata_t * pdat)
{
	jd9851_write_command(pdat, 0xdf);
	jd9851_write_data(pdat, 0x98);
	jd9851_write_data(pdat, 0x51);
	jd9851_write_data(pdat, 0xe9);
	jd9851_write_command(pdat, 0xde);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_command(pdat, 0xb7);
	jd9851_write_data(pdat, 0x31);
	jd9851_write_data(pdat, 0x88);
	jd9851_write_data(pdat, 0x31);
	jd9851_write_data(pdat, 0x10);
	jd9851_write_command(pdat, 0xc8);
	jd9851_write_data(pdat, 0x3f);
	jd9851_write_data(pdat, 0x3c);
	jd9851_write_data(pdat, 0x34);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2e);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x2c);
	jd9851_write_data(pdat, 0x2c);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x28);
	jd9851_write_data(pdat, 0x0e);
	jd9851_write_data(pdat, 0x3f);
	jd9851_write_data(pdat, 0x3c);
	jd9851_write_data(pdat, 0x34);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2e);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x2c);
	jd9851_write_data(pdat, 0x2c);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2d);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x2a);
	jd9851_write_data(pdat, 0x28);
	jd9851_write_data(pdat, 0x0e);
	jd9851_write_command(pdat, 0xb9);
	jd9851_write_data(pdat, 0x33);
	jd9851_write_data(pdat, 0x28);
	jd9851_write_data(pdat, 0xcc);
	jd9851_write_command(pdat, 0xbb);
	jd9851_write_data(pdat, 0x06);
	jd9851_write_data(pdat, 0x7a);
	jd9851_write_data(pdat, 0x30);
	jd9851_write_data(pdat, 0x30);
	jd9851_write_data(pdat, 0x6c);
	jd9851_write_data(pdat, 0x60);
	jd9851_write_data(pdat, 0x50);
	jd9851_write_data(pdat, 0x70);
	jd9851_write_command(pdat, 0xbc);
	jd9851_write_data(pdat, 0x38);
	jd9851_write_data(pdat, 0x3c);
	jd9851_write_command(pdat, 0xc0);
	jd9851_write_data(pdat, 0x31);
	jd9851_write_data(pdat, 0x20);
	jd9851_write_command(pdat, 0xc1);
	jd9851_write_data(pdat, 0x16);
	jd9851_write_command(pdat, 0xc3);
	jd9851_write_data(pdat, 0x08);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_data(pdat, 0x0a);
	jd9851_write_data(pdat, 0x10);
	jd9851_write_data(pdat, 0x08);
	jd9851_write_data(pdat, 0x54);
	jd9851_write_data(pdat, 0x45);
	jd9851_write_data(pdat, 0x71);
	jd9851_write_data(pdat, 0x2c);
	jd9851_write_command(pdat, 0xc4);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_data(pdat, 0xa0);
	jd9851_write_data(pdat, 0x79);
	jd9851_write_data(pdat, 0x0e);
	jd9851_write_data(pdat, 0x0a);
	jd9851_write_data(pdat, 0x16);
	jd9851_write_data(pdat, 0x79);
	jd9851_write_data(pdat, 0x0e);
	jd9851_write_data(pdat, 0x0a);
	jd9851_write_data(pdat, 0x16);
	jd9851_write_data(pdat, 0x79);
	jd9851_write_data(pdat, 0x0e);
	jd9851_write_data(pdat, 0x0a);
	jd9851_write_data(pdat, 0x16);
	jd9851_write_data(pdat, 0x82);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_data(pdat, 0x03);
	jd9851_write_command(pdat, 0xd0);
	jd9851_write_data(pdat, 0x04);
	jd9851_write_data(pdat, 0x0c);
	jd9851_write_data(pdat, 0x6b);
	jd9851_write_data(pdat, 0x0f);
	jd9851_write_data(pdat, 0x07);
	jd9851_write_data(pdat, 0x03);
	jd9851_write_command(pdat, 0xd7);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_command(pdat, 0xde);
	jd9851_write_data(pdat, 0x02);
	jd9851_write_command(pdat, 0xb8);
	jd9851_write_data(pdat, 0x19);
	jd9851_write_data(pdat, 0xa0);
	jd9851_write_data(pdat, 0x2f);
	jd9851_write_data(pdat, 0x04);
	jd9851_write_data(pdat, 0x33);
	jd9851_write_command(pdat, 0xc1);
	jd9851_write_data(pdat, 0x10);
	jd9851_write_data(pdat, 0x66);
	jd9851_write_data(pdat, 0x66);
	jd9851_write_data(pdat, 0x01);
	jd9851_write_command(pdat, 0xde);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_command(pdat, 0x11);
	jd9851_write_command(pdat, 0xde);
	jd9851_write_data(pdat, 0x02);
	jd9851_write_command(pdat, 0xc5);
	jd9851_write_data(pdat, 0x01);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_command(pdat, 0xca);
	jd9851_write_data(pdat, 0x10);
	jd9851_write_data(pdat, 0x20);
	jd9851_write_data(pdat, 0xf4);
	jd9851_write_command(pdat, 0xde);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_command(pdat, 0x21);
	jd9851_write_command(pdat, 0x3a);
	jd9851_write_data(pdat, 0x55);
	jd9851_write_command(pdat, 0x36);
	jd9851_write_data(pdat, 0x00);
	jd9851_write_command(pdat, 0x29);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_jd9851_pdata_t * pdat = (struct fb_jd9851_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_jd9851_pdata_t * pdat = (struct fb_jd9851_pdata_t *)fb->priv;
	return led_get_brightness(pdat->backlight);
}

static struct surface_t * fb_create(struct framebuffer_t * fb)
{
	return surface_alloc(fb->width, fb->height);
}

static void fb_destroy(struct framebuffer_t * fb, struct surface_t * s)
{
	surface_free(s);
}

static int fb_present(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l, void (*cb)(void *), void * data)
{
	struct fb_jd9851_pdata_t * pdat = (struct fb_jd9851_pdata_t *)fb->priv;

	if(l && (l->nitems > 0))
	{
		for(int i = 0; i < l->nitems; i++)
		{
			struct region_t * r = &l->items[i];
			uint8_t * q = pdat->txbuf;
			for(int y = 0; y < r->h; y++)
			{
				uint32_t * p = s->pixels + (r->y + y) * s->stride + (r->x << 2);
				for(int x = 0; x < r->w; x++)
				{
					uint32_t v = *p++;
					*q++ = ((v >> 16) & 0xf8) | ((v >> 13) & 0x07);
					*q++ = ((v >> 5) & 0xe0) | ((v >> 3) & 0x1f);
				}
			}
			jd9851_set_window(pdat, r->x, r->y, r->w, r->h);
			jd9851_write_command(pdat, 0x2c);
			spi_device_select(pdat->dev);
			spi_device_write_then_read(pdat->dev, pdat->txbuf, r->w * r->h * 2, 0, 0);
			spi_device_deselect(pdat->dev);
		}
	}
	return 0;
}

static void fb_wait(struct framebuffer_t * fb)
{
}

static struct device_t * fb_jd9851_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_jd9851_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct spi_device_t * spidev;
	int cd = dt_read_int(n, "cd-gpio", -1);

	if(!gpio_is_valid(cd))
		return NULL;

	spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 0), 8, dt_read_int(n, "speed", 0));
	if(!spidev)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_jd9851_pdata_t));
	if(!pdat)
	{
		spi_device_free(spidev);
		return NULL;
	}

	fb = xos_mem_malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		spi_device_free(spidev);
		xos_mem_free(pdat);
		return NULL;
	}

	pdat->dev = spidev;
	pdat->rst = dt_read_int(n, "reset-gpio", -1);
	pdat->rstcfg = dt_read_int(n, "reset-gpio-config", -1);
	pdat->cd = cd;
	pdat->cdcfg = dt_read_int(n, "cd-gpio-config", -1);
	pdat->width = dt_read_int(n, "width", 170);
	pdat->height = dt_read_int(n, "height", 320);
	pdat->pwidth = dt_read_int(n, "physical-width", 18);
	pdat->pheight = dt_read_int(n, "physical-height", 33);
	pdat->backlight = search_led(dt_read_string(n, "backlight", NULL));

	pdat->txbuf = xos_mem_malloc(pdat->width * pdat->height * 2);
	if(!pdat->txbuf)
	{
		spi_device_free(spidev);
		xos_mem_free(pdat);
		xos_mem_free(fb);
		return NULL;
	}

	fb->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	fb->width = pdat->width;
	fb->height = pdat->height;
	fb->pwidth = pdat->pwidth;
	fb->pheight = pdat->pheight;
	fb->setbl = fb_setbl;
	fb->getbl = fb_getbl;
	fb->create = fb_create;
	fb->destroy = fb_destroy;
	fb->present = fb_present;
	fb->wait = fb_wait;
	fb->priv = pdat;

	if(pdat->rst >= 0)
	{
		if(pdat->rst >= 0)
			gpio_set_cfg(pdat->rst, pdat->rstcfg);
		gpio_set_pull(pdat->rst, GPIO_PULL_UP);
		gpio_direction_output(pdat->rst, 0);
		mdelay(100);
		gpio_direction_output(pdat->rst, 1);
		mdelay(100);
	}
	if(pdat->cd >= 0)
	{
		if(pdat->cd >= 0)
			gpio_set_cfg(pdat->cd, pdat->cdcfg);
		gpio_set_pull(pdat->cd, GPIO_PULL_UP);
		gpio_direction_output(pdat->cd, 1);
	}
	jd9851_init(pdat);

	if(!(dev = register_framebuffer(fb, drv)))
	{
		spi_device_free(pdat->dev);
		xos_mem_free(pdat->txbuf);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
		return NULL;
	}
	return dev;
}

static void fb_jd9851_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_jd9851_pdata_t * pdat = (struct fb_jd9851_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		spi_device_free(pdat->dev);
		xos_mem_free(pdat->txbuf);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_jd9851_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_jd9851_pdata_t * pdat = (struct fb_jd9851_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_jd9851_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_jd9851_pdata_t * pdat = (struct fb_jd9851_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_jd9851 = {
	.name		= "fb-jd9851",
	.probe		= fb_jd9851_probe,
	.remove		= fb_jd9851_remove,
	.suspend	= fb_jd9851_suspend,
	.resume		= fb_jd9851_resume,
};

static void fb_jd9851_driver_init(void)
{
	register_driver(&fb_jd9851);
}

static void fb_jd9851_driver_exit(void)
{
	unregister_driver(&fb_jd9851);
}

driver_initcall(fb_jd9851_driver_init);
driver_exitcall(fb_jd9851_driver_exit);
