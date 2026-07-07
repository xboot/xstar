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
 * LCD - Shanghai New Vision NV3041A Lcd Driver
 *
 * Example:
 *	"fb-nv3041a": {
 *		"spi-bus": "spi-v821.1",
 *		"chip-select": 0,
 *		"type": 0,
 *		"mode": 0,
 *		"speed": 50000000,
 *		"reset-gpio": 111,
 *		"reset-gpio-config": 1,
 *		"cd-gpio": 101,
 *		"cd-gpio-config": 1,
 *		"width": 480,
 *		"height": 272,
 *		"physical-width": 95,
 *		"physical-height": 54,
 *		"backlight": null
 *	}
 */

struct fb_nv3041a_pdata_t {
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

static void nv3041a_write_command(struct fb_nv3041a_pdata_t * pdat, uint8_t cmd)
{
	spi_device_select(pdat->dev);
	gpio_set_value(pdat->cd, 0);
	spi_device_write_then_read(pdat->dev, &cmd, 1, 0, 0);
	gpio_set_value(pdat->cd, 1);
	spi_device_deselect(pdat->dev);
}

static void nv3041a_write_data(struct fb_nv3041a_pdata_t * pdat, uint8_t dat)
{
	spi_device_select(pdat->dev);
	spi_device_write_then_read(pdat->dev, &dat, 1, 0, 0);
	spi_device_deselect(pdat->dev);
}

static void nv3041a_set_window(struct fb_nv3041a_pdata_t * pdat, int x, int y, int w, int h)
{
	nv3041a_write_command(pdat, 0x2a);
	nv3041a_write_data(pdat, (x >> 8) & 0xff);
	nv3041a_write_data(pdat, (x >> 0) & 0xff);
	nv3041a_write_data(pdat, ((x + w - 1) >> 8) & 0xff);
	nv3041a_write_data(pdat, ((x + w - 1) >> 0) & 0xff);
	nv3041a_write_command(pdat, 0x2b);
	nv3041a_write_data(pdat, (y >> 8) & 0xff);
	nv3041a_write_data(pdat, (y >> 0) & 0xff);
	nv3041a_write_data(pdat, ((y + h - 1) >> 8) & 0xff);
	nv3041a_write_data(pdat, ((y + h - 1) >> 0) & 0xff);
}

static void nv3041a_init(struct fb_nv3041a_pdata_t * pdat)
{
	nv3041a_write_command(pdat, 0xff);
	nv3041a_write_data(pdat, 0xa5);
	nv3041a_write_command(pdat, 0xe7);
	nv3041a_write_data(pdat, 0x10);
	nv3041a_write_command(pdat, 0x35);
	nv3041a_write_data(pdat, 0x00);
	nv3041a_write_command(pdat, 0x3a);
	nv3041a_write_data(pdat, 0x01);
	nv3041a_write_command(pdat, 0x40);
	nv3041a_write_data(pdat, 0x01);
	nv3041a_write_command(pdat, 0x41);
	nv3041a_write_data(pdat, 0x01);
	nv3041a_write_command(pdat, 0x55);
	nv3041a_write_data(pdat, 0x01);
	nv3041a_write_command(pdat, 0x44);
	nv3041a_write_data(pdat, 0x15);
	nv3041a_write_command(pdat, 0x45);
	nv3041a_write_data(pdat, 0x15);
	nv3041a_write_command(pdat, 0x7d);
	nv3041a_write_data(pdat, 0x03);
	nv3041a_write_command(pdat, 0xc1);
	nv3041a_write_data(pdat, 0xbb);
	nv3041a_write_command(pdat, 0xc2);
	nv3041a_write_data(pdat, 0x14);
	nv3041a_write_command(pdat, 0xc3);
	nv3041a_write_data(pdat, 0x13);
	nv3041a_write_command(pdat, 0xc6);
	nv3041a_write_data(pdat, 0x3e);
	nv3041a_write_command(pdat, 0xc7);
	nv3041a_write_data(pdat, 0x25);
	nv3041a_write_command(pdat, 0xc8);
	nv3041a_write_data(pdat, 0x11);
	nv3041a_write_command(pdat, 0x7a);
	nv3041a_write_data(pdat, 0x7c);
	nv3041a_write_command(pdat, 0x6f);
	nv3041a_write_data(pdat, 0x56);
	nv3041a_write_command(pdat, 0x78);
	nv3041a_write_data(pdat, 0x2a);
	nv3041a_write_command(pdat, 0x73);
	nv3041a_write_data(pdat, 0x08);
	nv3041a_write_command(pdat, 0x74);
	nv3041a_write_data(pdat, 0x12);
	nv3041a_write_command(pdat, 0xc9);
	nv3041a_write_data(pdat, 0x00);
	nv3041a_write_command(pdat, 0x67);
	nv3041a_write_data(pdat, 0x11);
	nv3041a_write_command(pdat, 0x51);
	nv3041a_write_data(pdat, 0x4b);
	nv3041a_write_command(pdat, 0x52);
	nv3041a_write_data(pdat, 0x7c);
	nv3041a_write_command(pdat, 0x53);
	nv3041a_write_data(pdat, 0x45);
	nv3041a_write_command(pdat, 0x54);
	nv3041a_write_data(pdat, 0x77);
	nv3041a_write_command(pdat, 0x46);
	nv3041a_write_data(pdat, 0x0a);
	nv3041a_write_command(pdat, 0x47);
	nv3041a_write_data(pdat, 0x2a);
	nv3041a_write_command(pdat, 0x48);
	nv3041a_write_data(pdat, 0x0a);
	nv3041a_write_command(pdat, 0x49);
	nv3041a_write_data(pdat, 0x1a);
	nv3041a_write_command(pdat, 0x56);
	nv3041a_write_data(pdat, 0x43);
	nv3041a_write_command(pdat, 0x57);
	nv3041a_write_data(pdat, 0x42);
	nv3041a_write_command(pdat, 0x58);
	nv3041a_write_data(pdat, 0x3c);
	nv3041a_write_command(pdat, 0x59);
	nv3041a_write_data(pdat, 0x64);
	nv3041a_write_command(pdat, 0x5a);
	nv3041a_write_data(pdat, 0x41);
	nv3041a_write_command(pdat, 0x5b);
	nv3041a_write_data(pdat, 0x3c);
	nv3041a_write_command(pdat, 0x5c);
	nv3041a_write_data(pdat, 0x02);
	nv3041a_write_command(pdat, 0x5d);
	nv3041a_write_data(pdat, 0x3c);
	nv3041a_write_command(pdat, 0x5e);
	nv3041a_write_data(pdat, 0x1f);
	nv3041a_write_command(pdat, 0x60);
	nv3041a_write_data(pdat, 0x80);
	nv3041a_write_command(pdat, 0x61);
	nv3041a_write_data(pdat, 0x3f);
	nv3041a_write_command(pdat, 0x62);
	nv3041a_write_data(pdat, 0x21);
	nv3041a_write_command(pdat, 0x63);
	nv3041a_write_data(pdat, 0x07);
	nv3041a_write_command(pdat, 0x64);
	nv3041a_write_data(pdat, 0xe0);
	nv3041a_write_command(pdat, 0x65);
	nv3041a_write_data(pdat, 0x01);
	nv3041a_write_command(pdat, 0xca);
	nv3041a_write_data(pdat, 0x20);
	nv3041a_write_command(pdat, 0xcb);
	nv3041a_write_data(pdat, 0x52);
	nv3041a_write_command(pdat, 0xcc);
	nv3041a_write_data(pdat, 0x10);
	nv3041a_write_command(pdat, 0xcd);
	nv3041a_write_data(pdat, 0x42);
	nv3041a_write_command(pdat, 0xd0);
	nv3041a_write_data(pdat, 0x20);
	nv3041a_write_command(pdat, 0xd1);
	nv3041a_write_data(pdat, 0x52);
	nv3041a_write_command(pdat, 0xd2);
	nv3041a_write_data(pdat, 0x10);
	nv3041a_write_command(pdat, 0xd3);
	nv3041a_write_data(pdat, 0x42);
	nv3041a_write_command(pdat, 0xd4);
	nv3041a_write_data(pdat, 0x0a);
	nv3041a_write_command(pdat, 0xd5);
	nv3041a_write_data(pdat, 0x32);
	nv3041a_write_command(pdat, 0x6e);
	nv3041a_write_data(pdat, 0x14);
	nv3041a_write_command(pdat, 0xe5);
	nv3041a_write_data(pdat, 0x06);
	nv3041a_write_command(pdat, 0xe6);
	nv3041a_write_data(pdat, 0x00);
	nv3041a_write_command(pdat, 0xf8);
	nv3041a_write_data(pdat, 0x06);
	nv3041a_write_command(pdat, 0xf9);
	nv3041a_write_data(pdat, 0x00);
	nv3041a_write_command(pdat, 0x80);
	nv3041a_write_data(pdat, 0x08);
	nv3041a_write_command(pdat, 0xa0);
	nv3041a_write_data(pdat, 0x08);
	nv3041a_write_command(pdat, 0x81);
	nv3041a_write_data(pdat, 0x0a);
	nv3041a_write_command(pdat, 0xa1);
	nv3041a_write_data(pdat, 0x0a);
	nv3041a_write_command(pdat, 0x82);
	nv3041a_write_data(pdat, 0x09);
	nv3041a_write_command(pdat, 0xa2);
	nv3041a_write_data(pdat, 0x09);
	nv3041a_write_command(pdat, 0x86);
	nv3041a_write_data(pdat, 0x38);
	nv3041a_write_command(pdat, 0xa6);
	nv3041a_write_data(pdat, 0x2a);
	nv3041a_write_command(pdat, 0x87);
	nv3041a_write_data(pdat, 0x4a);
	nv3041a_write_command(pdat, 0xa7);
	nv3041a_write_data(pdat, 0x40);
	nv3041a_write_command(pdat, 0x83);
	nv3041a_write_data(pdat, 0x39);
	nv3041a_write_command(pdat, 0xa3);
	nv3041a_write_data(pdat, 0x39);
	nv3041a_write_command(pdat, 0x84);
	nv3041a_write_data(pdat, 0x37);
	nv3041a_write_command(pdat, 0xa4);
	nv3041a_write_data(pdat, 0x37);
	nv3041a_write_command(pdat, 0x85);
	nv3041a_write_data(pdat, 0x28);
	nv3041a_write_command(pdat, 0xa5);
	nv3041a_write_data(pdat, 0x28);
	nv3041a_write_command(pdat, 0x88);
	nv3041a_write_data(pdat, 0x0b);
	nv3041a_write_command(pdat, 0xa8);
	nv3041a_write_data(pdat, 0x04);
	nv3041a_write_command(pdat, 0x89);
	nv3041a_write_data(pdat, 0x13);
	nv3041a_write_command(pdat, 0xa9);
	nv3041a_write_data(pdat, 0x09);
	nv3041a_write_command(pdat, 0x8a);
	nv3041a_write_data(pdat, 0x1b);
	nv3041a_write_command(pdat, 0xaa);
	nv3041a_write_data(pdat, 0x11);
	nv3041a_write_command(pdat, 0x8b);
	nv3041a_write_data(pdat, 0x11);
	nv3041a_write_command(pdat, 0xab);
	nv3041a_write_data(pdat, 0x0d);
	nv3041a_write_command(pdat, 0x8c);
	nv3041a_write_data(pdat, 0x14);
	nv3041a_write_command(pdat, 0xac);
	nv3041a_write_data(pdat, 0x13);
	nv3041a_write_command(pdat, 0x8d);
	nv3041a_write_data(pdat, 0x15);
	nv3041a_write_command(pdat, 0xad);
	nv3041a_write_data(pdat, 0x0e);
	nv3041a_write_command(pdat, 0x8e);
	nv3041a_write_data(pdat, 0x10);
	nv3041a_write_command(pdat, 0xae);
	nv3041a_write_data(pdat, 0x0f);
	nv3041a_write_command(pdat, 0x8f);
	nv3041a_write_data(pdat, 0x18);
	nv3041a_write_command(pdat, 0xaf);
	nv3041a_write_data(pdat, 0x0e);
	nv3041a_write_command(pdat, 0x90);
	nv3041a_write_data(pdat, 0x07);
	nv3041a_write_command(pdat, 0xb0);
	nv3041a_write_data(pdat, 0x05);
	nv3041a_write_command(pdat, 0x91);
	nv3041a_write_data(pdat, 0x11);
	nv3041a_write_command(pdat, 0xb1);
	nv3041a_write_data(pdat, 0x0e);
	nv3041a_write_command(pdat, 0x92);
	nv3041a_write_data(pdat, 0x19);
	nv3041a_write_command(pdat, 0xb2);
	nv3041a_write_data(pdat, 0x14);
	nv3041a_write_command(pdat, 0xff);
	nv3041a_write_data(pdat, 0x00);
	nv3041a_write_command(pdat, 0x11);
	nv3041a_write_command(pdat, 0x29);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_nv3041a_pdata_t * pdat = (struct fb_nv3041a_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_nv3041a_pdata_t * pdat = (struct fb_nv3041a_pdata_t *)fb->priv;
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
	struct fb_nv3041a_pdata_t * pdat = (struct fb_nv3041a_pdata_t *)fb->priv;
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
			nv3041a_set_window(pdat, r->x, r->y, r->w, r->h);
			nv3041a_write_command(pdat, 0x2c);
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
		nv3041a_set_window(pdat, 0, 0, pdat->width, pdat->height);
		nv3041a_write_command(pdat, 0x2c);
		spi_device_select(pdat->dev);
		spi_device_write_then_read(pdat->dev, txbuf, pdat->width * pdat->height * 2, 0, 0);
		spi_device_deselect(pdat->dev);
	}
}

static struct device_t * fb_nv3041a_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_nv3041a_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct spi_device_t * spidev;
	int cd = dt_read_int(n, "cd-gpio", -1);

	if(!gpio_is_valid(cd))
		return NULL;

	spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 0), 8, dt_read_int(n, "speed", 0));
	if(!spidev)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_nv3041a_pdata_t));
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
	pdat->width = dt_read_int(n, "width", 480);
	pdat->height = dt_read_int(n, "height", 272);
	pdat->pwidth = dt_read_int(n, "physical-width", 95);
	pdat->pheight = dt_read_int(n, "physical-height", 54);
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
	nv3041a_init(pdat);

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

static void fb_nv3041a_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_nv3041a_pdata_t * pdat = (struct fb_nv3041a_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		spi_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_nv3041a_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_nv3041a_pdata_t * pdat = (struct fb_nv3041a_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_nv3041a_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_nv3041a_pdata_t * pdat = (struct fb_nv3041a_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_nv3041a = {
	.name		= "fb-nv3041a",
	.probe		= fb_nv3041a_probe,
	.remove		= fb_nv3041a_remove,
	.suspend	= fb_nv3041a_suspend,
	.resume		= fb_nv3041a_resume,
};

static void fb_nv3041a_driver_init(void)
{
	register_driver(&fb_nv3041a);
}

static void fb_nv3041a_driver_exit(void)
{
	unregister_driver(&fb_nv3041a);
}

driver_initcall(fb_nv3041a_driver_init);
driver_exitcall(fb_nv3041a_driver_exit);
