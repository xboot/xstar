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
 * GC9107 - GalaxyCore lcd single chip driver
 *
 * Example:
 *   "fb-gc9107": {
 *	     "spi-bus": "spi-v821.1",
 *	     "chip-select": 0,
 *	     "type": 0,
 *	     "mode": 0,
 *	     "speed": 50000000,
 *	     "reset-gpio": 111,
 *	     "reset-gpio-config": 1,
 *	     "cd-gpio": 101,
 *	     "cd-gpio-config": 1,
 *	     "width": 128,
 *	     "height": 128,
 *	     "physical-width": 23,
 *	     "physical-height": 23,
 *	     "backlight": null
 *   }
 */

struct fb_gc9107_pdata_t {
	struct spi_device_t * dev;
	int rst;
	int rstcfg;
	int cd;
	int cdcfg;
	int width;
	int height;
	int pwidth;
	int pheight;

	struct led_t * backlight;
	int brightness;
};

static void gc9107_write_command(struct fb_gc9107_pdata_t * pdat, uint8_t cmd)
{
	spi_device_select(pdat->dev);
	gpio_set_value(pdat->cd, 0);
	spi_device_write_then_read(pdat->dev, &cmd, 1, 0, 0);
	gpio_set_value(pdat->cd, 1);
	spi_device_deselect(pdat->dev);
}

static void gc9107_write_data(struct fb_gc9107_pdata_t * pdat, uint8_t dat)
{
	spi_device_select(pdat->dev);
	spi_device_write_then_read(pdat->dev, &dat, 1, 0, 0);
	spi_device_deselect(pdat->dev);
}

static void gc9107_set_window(struct fb_gc9107_pdata_t * pdat, int x, int y, int w, int h)
{
	gc9107_write_command(pdat, 0x2a);
	gc9107_write_data(pdat, (x >> 8) & 0xff);
	gc9107_write_data(pdat, (x >> 0) & 0xff);
	gc9107_write_data(pdat, ((x + w - 1) >> 8) & 0xff);
	gc9107_write_data(pdat, ((x + w - 1) >> 0) & 0xff);
	gc9107_write_command(pdat, 0x2b);
	gc9107_write_data(pdat, (y >> 8) & 0xff);
	gc9107_write_data(pdat, (y >> 0) & 0xff);
	gc9107_write_data(pdat, ((y + h - 1) >> 8) & 0xff);
	gc9107_write_data(pdat, ((y + h - 1) >> 0) & 0xff);
}

static void gc9107_init(struct fb_gc9107_pdata_t * pdat)
{
	gc9107_write_command(pdat, 0xef);
	gc9107_write_command(pdat, 0xeb);
	gc9107_write_data(pdat, 0x14);
	gc9107_write_command(pdat, 0xfe);
	gc9107_write_command(pdat, 0xef);
	gc9107_write_command(pdat, 0xeb);
	gc9107_write_data(pdat, 0x14);
	gc9107_write_command(pdat, 0x84);
	gc9107_write_data(pdat, 0x40);
	gc9107_write_command(pdat, 0x85);
	gc9107_write_data(pdat, 0xff);
	gc9107_write_command(pdat, 0x86);
	gc9107_write_data(pdat, 0xff);
	gc9107_write_command(pdat, 0x87);
	gc9107_write_data(pdat, 0xff);
	gc9107_write_command(pdat, 0x88);
	gc9107_write_data(pdat, 0x0a);
	gc9107_write_command(pdat, 0x89);
	gc9107_write_data(pdat, 0x21);
	gc9107_write_command(pdat, 0x8a);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_command(pdat, 0x8b);
	gc9107_write_data(pdat, 0x80);
	gc9107_write_command(pdat, 0x8c);
	gc9107_write_data(pdat, 0x01);
	gc9107_write_command(pdat, 0x8d);
	gc9107_write_data(pdat, 0x01);
	gc9107_write_command(pdat, 0x8e);
	gc9107_write_data(pdat, 0xff);
	gc9107_write_command(pdat, 0x8f);
	gc9107_write_data(pdat, 0xff);
	gc9107_write_command(pdat, 0xb6);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x20);
	gc9107_write_command(pdat, 0x3a);
	gc9107_write_data(pdat, 0x05);
	gc9107_write_command(pdat, 0x90);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_command(pdat, 0xbd);
	gc9107_write_data(pdat, 0x06);
	gc9107_write_command(pdat, 0xbc);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_command(pdat, 0xff);
	gc9107_write_data(pdat, 0x60);
	gc9107_write_data(pdat, 0x01);
	gc9107_write_data(pdat, 0x04);
	gc9107_write_command(pdat, 0xc3);
	gc9107_write_data(pdat, 0x13);
	gc9107_write_command(pdat, 0xc4);
	gc9107_write_data(pdat, 0x13);
	gc9107_write_command(pdat, 0xc9);
	gc9107_write_data(pdat, 0x22);
	gc9107_write_command(pdat, 0xbe);
	gc9107_write_data(pdat, 0x11);
	gc9107_write_command(pdat, 0xe1);
	gc9107_write_data(pdat, 0x10);
	gc9107_write_data(pdat, 0x0e);
	gc9107_write_command(pdat, 0xdf);
	gc9107_write_data(pdat, 0x21);
	gc9107_write_data(pdat, 0x0c);
	gc9107_write_data(pdat, 0x02);
	gc9107_write_command(pdat, 0xf0);
	gc9107_write_data(pdat, 0x45);
	gc9107_write_data(pdat, 0x09);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x26);
	gc9107_write_data(pdat, 0x2a);
	gc9107_write_command(pdat, 0xf1);
	gc9107_write_data(pdat, 0x43);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x72);
	gc9107_write_data(pdat, 0x36);
	gc9107_write_data(pdat, 0x37);
	gc9107_write_data(pdat, 0x6f);
	gc9107_write_command(pdat, 0xf2);
	gc9107_write_data(pdat, 0x45);
	gc9107_write_data(pdat, 0x09);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x26);
	gc9107_write_data(pdat, 0x2a);
	gc9107_write_command(pdat, 0xf3);
	gc9107_write_data(pdat, 0x43);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x72);
	gc9107_write_data(pdat, 0x36);
	gc9107_write_data(pdat, 0x37);
	gc9107_write_data(pdat, 0x6f);
	gc9107_write_command(pdat, 0xed);
	gc9107_write_data(pdat, 0x1b);
	gc9107_write_data(pdat, 0x0b);
	gc9107_write_command(pdat, 0xae);
	gc9107_write_data(pdat, 0x77);
	gc9107_write_command(pdat, 0xcd);
	gc9107_write_data(pdat, 0x63);
	gc9107_write_command(pdat, 0x70);
	gc9107_write_data(pdat, 0x07);
	gc9107_write_data(pdat, 0x07);
	gc9107_write_data(pdat, 0x04);
	gc9107_write_data(pdat, 0x0e);
	gc9107_write_data(pdat, 0x0f);
	gc9107_write_data(pdat, 0x09);
	gc9107_write_data(pdat, 0x07);
	gc9107_write_data(pdat, 0x08);
	gc9107_write_data(pdat, 0x03);
	gc9107_write_command(pdat, 0xe8);
	gc9107_write_data(pdat, 0x34);
	gc9107_write_command(pdat, 0x62);
	gc9107_write_data(pdat, 0x18);
	gc9107_write_data(pdat, 0x0d);
	gc9107_write_data(pdat, 0x71);
	gc9107_write_data(pdat, 0xed);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x18);
	gc9107_write_data(pdat, 0x0f);
	gc9107_write_data(pdat, 0x71);
	gc9107_write_data(pdat, 0xef);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_command(pdat, 0x63);
	gc9107_write_data(pdat, 0x18);
	gc9107_write_data(pdat, 0x11);
	gc9107_write_data(pdat, 0x71);
	gc9107_write_data(pdat, 0xf1);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x18);
	gc9107_write_data(pdat, 0x13);
	gc9107_write_data(pdat, 0x71);
	gc9107_write_data(pdat, 0xf3);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_data(pdat, 0x70);
	gc9107_write_command(pdat, 0x64);
	gc9107_write_data(pdat, 0x28);
	gc9107_write_data(pdat, 0x29);
	gc9107_write_data(pdat, 0xf1);
	gc9107_write_data(pdat, 0x01);
	gc9107_write_data(pdat, 0xf1);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x07);
	gc9107_write_command(pdat, 0x66);
	gc9107_write_data(pdat, 0x3c);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0xcd);
	gc9107_write_data(pdat, 0x67);
	gc9107_write_data(pdat, 0x45);
	gc9107_write_data(pdat, 0x45);
	gc9107_write_data(pdat, 0x10);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_command(pdat, 0x67);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x3c);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x01);
	gc9107_write_data(pdat, 0x54);
	gc9107_write_data(pdat, 0x10);
	gc9107_write_data(pdat, 0x32);
	gc9107_write_data(pdat, 0x98);
	gc9107_write_command(pdat, 0x74);
	gc9107_write_data(pdat, 0x10);
	gc9107_write_data(pdat, 0x85);
	gc9107_write_data(pdat, 0x80);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_data(pdat, 0x4e);
	gc9107_write_data(pdat, 0x00);
	gc9107_write_command(pdat, 0x98);
	gc9107_write_data(pdat, 0x3e);
	gc9107_write_data(pdat, 0x07);
	gc9107_write_command(pdat, 0x35);
	gc9107_write_command(pdat, 0x21);

	gc9107_write_command(pdat, 0x36);
	gc9107_write_data(pdat, 0xc8);

	gc9107_write_command(pdat, 0x11);
	gc9107_write_command(pdat, 0x29);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_gc9107_pdata_t * pdat = (struct fb_gc9107_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_gc9107_pdata_t * pdat = (struct fb_gc9107_pdata_t *)fb->priv;
	return led_get_brightness(pdat->backlight);
}

static struct surface_t * fb_create(struct framebuffer_t * fb, int width, int height)
{
	return surface_alloc(width, height);
}

static void fb_destroy(struct framebuffer_t * fb, struct surface_t * s)
{
	surface_free(s);
}

static void fb_present(struct framebuffer_t * fb, struct surface_t * s, struct dirtylist_t * l)
{
	struct fb_gc9107_pdata_t * pdat = (struct fb_gc9107_pdata_t *)fb->priv;
	uint8_t txbuf[pdat->width * pdat->height * 2];

	if(l && (l->count > 0))
	{
		for(int i = 0; i < l->count; i++)
		{
			struct region_t * r = &l->items[i].region;
			uint8_t * q = txbuf;
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
			gc9107_set_window(pdat, r->x, r->y, r->w, r->h);
			gc9107_write_command(pdat, 0x2c);
			spi_device_select(pdat->dev);
			spi_device_write_then_read(pdat->dev, txbuf, r->w * r->h * 2, 0, 0);
			spi_device_deselect(pdat->dev);
		}
	}
	else
	{
		uint32_t * p = s->pixels;
		uint8_t * q = txbuf;
		for(int y = 0; y < pdat->height; y++)
		{
			for(int x = 0; x < pdat->width; x++)
			{
				uint32_t v = *p++;
				*q++ = ((v >> 16) & 0xf8) | ((v >> 13) & 0x07);
				*q++ = ((v >> 5) & 0xe0) | ((v >> 3) & 0x1f);
			}
		}
		gc9107_set_window(pdat, 0, 0, pdat->width, pdat->height);
		gc9107_write_command(pdat, 0x2c);
		spi_device_select(pdat->dev);
		spi_device_write_then_read(pdat->dev, txbuf, pdat->width * pdat->height * 2, 0, 0);
		spi_device_deselect(pdat->dev);
	}
}

static struct device_t * fb_gc9107_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_gc9107_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct spi_device_t * spidev;
	int cd = dt_read_int(n, "cd-gpio", -1);

	if(!gpio_is_valid(cd))
		return NULL;

	spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 0), 8, dt_read_int(n, "speed", 0));
	if(!spidev)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_gc9107_pdata_t));
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
	pdat->width = dt_read_int(n, "width", 128);
	pdat->height = dt_read_int(n, "height", 128);
	pdat->pwidth = dt_read_int(n, "physical-width", 23);
	pdat->pheight = dt_read_int(n, "physical-height", 23);
	pdat->backlight = search_led(dt_read_string(n, "backlight", NULL));

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
	gc9107_init(pdat);

	if(!(dev = register_framebuffer(fb, drv)))
	{
		spi_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
		return NULL;
	}
	return dev;
}

static void fb_gc9107_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_gc9107_pdata_t * pdat = (struct fb_gc9107_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		spi_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_gc9107_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_gc9107_pdata_t * pdat = (struct fb_gc9107_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_gc9107_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_gc9107_pdata_t * pdat = (struct fb_gc9107_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_gc9107 = {
	.name		= "fb-gc9107",
	.probe		= fb_gc9107_probe,
	.remove		= fb_gc9107_remove,
	.suspend	= fb_gc9107_suspend,
	.resume		= fb_gc9107_resume,
};

static void fb_gc9107_driver_init(void)
{
	register_driver(&fb_gc9107);
}

static void fb_gc9107_driver_exit(void)
{
	unregister_driver(&fb_gc9107);
}

driver_initcall(fb_gc9107_driver_init);
driver_exitcall(fb_gc9107_driver_exit);
