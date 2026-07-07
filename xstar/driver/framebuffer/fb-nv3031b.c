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
 * LCD - Shanghai New Vision NV3031B Lcd Driver
 *
 * Example:
 *	"fb-nv3031b": {
 *		"spi-bus": "spi-v821.1",
 *		"chip-select": 0,
 *		"type": 0,
 *		"mode": 0,
 *		"speed": 50000000,
 *		"reset-gpio": 111,
 *		"reset-gpio-config": 1,
 *		"cd-gpio": 101,
 *		"cd-gpio-config": 1,
 *		"width": 170,
 *		"height": 320,
 *		"physical-width": 18,
 *		"physical-height": 33,
 *		"backlight": null
 *	}
 */

struct fb_nv3031b_pdata_t {
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

static void nv3031b_write_command(struct fb_nv3031b_pdata_t * pdat, uint8_t cmd)
{
	spi_device_select(pdat->dev);
	gpio_set_value(pdat->cd, 0);
	spi_device_write_then_read(pdat->dev, &cmd, 1, 0, 0);
	gpio_set_value(pdat->cd, 1);
	spi_device_deselect(pdat->dev);
}

static void nv3031b_write_data(struct fb_nv3031b_pdata_t * pdat, uint8_t dat)
{
	spi_device_select(pdat->dev);
	spi_device_write_then_read(pdat->dev, &dat, 1, 0, 0);
	spi_device_deselect(pdat->dev);
}

static void nv3031b_set_window(struct fb_nv3031b_pdata_t * pdat, int x, int y, int w, int h)
{
	x += 35;

	nv3031b_write_command(pdat, 0x2a);
	nv3031b_write_data(pdat, (x >> 8) & 0xff);
	nv3031b_write_data(pdat, (x >> 0) & 0xff);
	nv3031b_write_data(pdat, ((x + w - 1) >> 8) & 0xff);
	nv3031b_write_data(pdat, ((x + w - 1) >> 0) & 0xff);
	nv3031b_write_command(pdat, 0x2b);
	nv3031b_write_data(pdat, (y >> 8) & 0xff);
	nv3031b_write_data(pdat, (y >> 0) & 0xff);
	nv3031b_write_data(pdat, ((y + h - 1) >> 8) & 0xff);
	nv3031b_write_data(pdat, ((y + h - 1) >> 0) & 0xff);
}

static void nv3031b_init(struct fb_nv3031b_pdata_t * pdat)
{
	nv3031b_write_command(pdat, 0xfd);
	nv3031b_write_data(pdat, 0x06);
	nv3031b_write_data(pdat, 0x08);
	nv3031b_write_command(pdat, 0x61);
	nv3031b_write_data(pdat, 0x07);
	nv3031b_write_data(pdat, 0x07);
	nv3031b_write_command(pdat, 0x73);
	nv3031b_write_data(pdat, 0x70);
	nv3031b_write_command(pdat, 0x73);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_command(pdat, 0x62);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_data(pdat, 0x44);
	nv3031b_write_data(pdat, 0x40);
	nv3031b_write_command(pdat, 0x65);
	nv3031b_write_data(pdat, 0x08);
	nv3031b_write_data(pdat, 0x10);
	nv3031b_write_data(pdat, 0x21);
	nv3031b_write_command(pdat, 0x66);
	nv3031b_write_data(pdat, 0x08);
	nv3031b_write_data(pdat, 0x10);
	nv3031b_write_data(pdat, 0x21);
	nv3031b_write_command(pdat, 0x67);
	nv3031b_write_data(pdat, 0x21);
	nv3031b_write_data(pdat, 0x40);
	nv3031b_write_command(pdat, 0x68);
	nv3031b_write_data(pdat, 0x9f);
	nv3031b_write_data(pdat, 0x30);
	nv3031b_write_data(pdat, 0x2a);
	nv3031b_write_data(pdat, 0x3c);
	nv3031b_write_command(pdat, 0xb1);
	nv3031b_write_data(pdat, 0x0f);
	nv3031b_write_data(pdat, 0x02);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_command(pdat, 0xb4);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_command(pdat, 0xb5);
	nv3031b_write_data(pdat, 0x02);
	nv3031b_write_data(pdat, 0x02);
	nv3031b_write_data(pdat, 0x0a);
	nv3031b_write_data(pdat, 0x14);
	nv3031b_write_command(pdat, 0xb6);
	nv3031b_write_data(pdat, 0x44);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_data(pdat, 0x9f);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_data(pdat, 0x02);
	nv3031b_write_command(pdat, 0xe6);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_data(pdat, 0xff);
	nv3031b_write_command(pdat, 0xe7);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_data(pdat, 0x04);
	nv3031b_write_data(pdat, 0x03);
	nv3031b_write_data(pdat, 0x03);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_data(pdat, 0x12);
	nv3031b_write_command(pdat, 0xe8);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_data(pdat, 0x70);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_command(pdat, 0xec);
	nv3031b_write_data(pdat, 0x52);
	nv3031b_write_command(pdat, 0xdf);
	nv3031b_write_data(pdat, 0x11);
	nv3031b_write_command(pdat, 0xe0);
	nv3031b_write_data(pdat, 0x03);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_data(pdat, 0x0c);
	nv3031b_write_data(pdat, 0x09);
	nv3031b_write_data(pdat, 0x0c);
	nv3031b_write_data(pdat, 0x0d);
	nv3031b_write_data(pdat, 0x13);
	nv3031b_write_data(pdat, 0x16);
	nv3031b_write_command(pdat, 0xe3);
	nv3031b_write_data(pdat, 0x17);
	nv3031b_write_data(pdat, 0x13);
	nv3031b_write_data(pdat, 0x0e);
	nv3031b_write_data(pdat, 0x0c);
	nv3031b_write_data(pdat, 0x0d);
	nv3031b_write_data(pdat, 0x0c);
	nv3031b_write_data(pdat, 0x02);
	nv3031b_write_data(pdat, 0x03);
	nv3031b_write_command(pdat, 0xe1);
	nv3031b_write_data(pdat, 0x18);
	nv3031b_write_data(pdat, 0x65);
	nv3031b_write_command(pdat, 0xe4);
	nv3031b_write_data(pdat, 0x65);
	nv3031b_write_data(pdat, 0x18);
	nv3031b_write_command(pdat, 0xe2);
	nv3031b_write_data(pdat, 0x26);
	nv3031b_write_data(pdat, 0x1d);
	nv3031b_write_data(pdat, 0x23);
	nv3031b_write_data(pdat, 0x38);
	nv3031b_write_data(pdat, 0x3a);
	nv3031b_write_data(pdat, 0x3f);
	nv3031b_write_command(pdat, 0xe5);
	nv3031b_write_data(pdat, 0x3f);
	nv3031b_write_data(pdat, 0x3c);
	nv3031b_write_data(pdat, 0x39);
	nv3031b_write_data(pdat, 0x18);
	nv3031b_write_data(pdat, 0x27);
	nv3031b_write_data(pdat, 0x26);
	nv3031b_write_command(pdat, 0xf1);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_data(pdat, 0x02);
	nv3031b_write_command(pdat, 0xf6);
	nv3031b_write_data(pdat, 0x01);
	nv3031b_write_data(pdat, 0x30);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_command(pdat, 0xfd);
	nv3031b_write_data(pdat, 0xfa);
	nv3031b_write_data(pdat, 0xfc);
	nv3031b_write_command(pdat, 0x3a);
	nv3031b_write_data(pdat, 0x55);
	nv3031b_write_command(pdat, 0x35);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_command(pdat, 0x36);
	nv3031b_write_data(pdat, 0x00);
	nv3031b_write_command(pdat, 0x11);
	nv3031b_write_command(pdat, 0x29);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_nv3031b_pdata_t * pdat = (struct fb_nv3031b_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_nv3031b_pdata_t * pdat = (struct fb_nv3031b_pdata_t *)fb->priv;
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
	struct fb_nv3031b_pdata_t * pdat = (struct fb_nv3031b_pdata_t *)fb->priv;
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
			nv3031b_set_window(pdat, r->x, r->y, r->w, r->h);
			nv3031b_write_command(pdat, 0x2c);
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
		nv3031b_set_window(pdat, 0, 0, pdat->width, pdat->height);
		nv3031b_write_command(pdat, 0x2c);
		spi_device_select(pdat->dev);
		spi_device_write_then_read(pdat->dev, txbuf, pdat->width * pdat->height * 2, 0, 0);
		spi_device_deselect(pdat->dev);
	}
}

static struct device_t * fb_nv3031b_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_nv3031b_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct spi_device_t * spidev;
	int cd = dt_read_int(n, "cd-gpio", -1);

	if(!gpio_is_valid(cd))
		return NULL;

	spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 0), 8, dt_read_int(n, "speed", 0));
	if(!spidev)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_nv3031b_pdata_t));
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
	nv3031b_init(pdat);

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

static void fb_nv3031b_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_nv3031b_pdata_t * pdat = (struct fb_nv3031b_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		spi_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_nv3031b_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_nv3031b_pdata_t * pdat = (struct fb_nv3031b_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_nv3031b_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_nv3031b_pdata_t * pdat = (struct fb_nv3031b_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_nv3031b = {
	.name		= "fb-nv3031b",
	.probe		= fb_nv3031b_probe,
	.remove		= fb_nv3031b_remove,
	.suspend	= fb_nv3031b_suspend,
	.resume		= fb_nv3031b_resume,
};

static void fb_nv3031b_driver_init(void)
{
	register_driver(&fb_nv3031b);
}

static void fb_nv3031b_driver_exit(void)
{
	unregister_driver(&fb_nv3031b);
}

driver_initcall(fb_nv3031b_driver_init);
driver_exitcall(fb_nv3031b_driver_exit);
