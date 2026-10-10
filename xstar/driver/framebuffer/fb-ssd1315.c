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
 * SSD1315 - Solomon oled display controller
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: i2c slave address (default: 0x3c)
 * - dither: enable bayer 8x8 ordered dithering, false for luma threshold, true for bayer
 * - physical-width: display physical width in millimeters
 * - physical-height: display physical height in millimeters
 *
 * Example device tree node:
 *   "fb-ssd1315": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 60,
 *		 "dither": true,
 *		 "physical-width": 22,
 *		 "physical-height": 11
 *   }
 */

struct fb_ssd1315_pdata_t {
	struct i2c_device_t * dev;
	uint8_t buffer[128 * 64 / 8];
	int dither;
	int brightness;
};

static int ssd1315_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int ssd1315_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int ssd1315_detect(struct i2c_device_t * dev)
{
	uint8_t val;

	if(!ssd1315_read(dev, 0x0, &val, 1))
		return 0;
	return 1;
}

static void ssd1315_initial(struct fb_ssd1315_pdata_t * pdat)
{
	ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xae }, 1);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0x00, 0x10, 0x40, 0xb0 }, 4);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0x81, 0xff }, 2);
	ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xa1 }, 1);
	ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xa6 }, 1);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xa8, 0x3f }, 2);
	ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xc8 }, 1);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xd3, 0x00 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xd5, 0x80 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xd8, 0x05 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xd9, 0xf1 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xda, 0x12 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0xdb, 0x30 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0x8d, 0x14 }, 2);
	ssd1315_write(pdat->dev, 0x00, (uint8_t[]){ 0x20, 0x02 }, 2);
	ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xaf }, 1);
}

static void ssd1315_convert(struct fb_ssd1315_pdata_t * pdat, uint32_t * p, int stride, int x0, int y0, int x1, int y1)
{
	static const uint8_t bayer8[8][8] = {
		{  0, 32,  8, 40,  2, 34, 10, 42 },
		{ 48, 16, 56, 24, 50, 18, 58, 26 },
		{ 12, 44,  4, 36, 14, 46,  6, 38 },
		{ 60, 28, 52, 20, 62, 30, 54, 22 },
		{  3, 35, 11, 43,  1, 33,  9, 41 },
		{ 51, 19, 59, 27, 49, 17, 57, 25 },
		{ 15, 47,  7, 39, 13, 45,  5, 37 },
		{ 63, 31, 55, 23, 61, 29, 53, 21 },
	};
	for(int y = y0; y < y1; y++)
	{
		for(int x = x0; x < x1; x++)
		{
			uint32_t v = p[y * stride + x];
			int luma = ((v >> 16) & 0xff) * 299 + ((v >> 8) & 0xff) * 587 + (v & 0xff) * 114;
			int lit;
			if(pdat->dither)
				lit = (luma > (bayer8[y & 7][x & 7] * 4 + 2) * 1000);
			else
				lit = (luma >= 128000);
			if(lit)
				pdat->buffer[((y >> 3) << 7) + x] |= (1 << (y & 0x7));
			else
				pdat->buffer[((y >> 3) << 7) + x] &= ~(1 << (y & 0x7));
		}
	}
}

static void ssd1315_refresh(struct fb_ssd1315_pdata_t * pdat, int x, int y, int w, int h)
{
	int p0 = y >> 3;
	int p1 = (y + h - 1) >> 3;

	for(int i = p0; i <= p1; i++)
	{
		ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xb0 + i }, 1);
		ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0x00 | (x & 0x0f) }, 1);
		ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0x10 | ((x >> 4) & 0x07) }, 1);
		ssd1315_write(pdat->dev, 0x40, &pdat->buffer[128 * i + x], w);
	}
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_ssd1315_pdata_t * pdat = (struct fb_ssd1315_pdata_t *)fb->priv;

	if(pdat->brightness != brightness)
	{
		ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0x81 }, 1);
		ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ (0xff * brightness / 1000) & 0xff }, 1);
		if(brightness == 0)
			ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xae }, 1);
		else
			ssd1315_write(pdat->dev, 0x80, (uint8_t[]){ 0xaf }, 1);
		pdat->brightness = brightness;
	}
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_ssd1315_pdata_t * pdat = (struct fb_ssd1315_pdata_t *)fb->priv;
	return pdat->brightness;
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
	struct fb_ssd1315_pdata_t * pdat = (struct fb_ssd1315_pdata_t *)fb->priv;
	uint32_t * p = s->pixels;
	int stride = s->stride >> 2;

	if(l && (l->nitems > 0))
	{
		for(unsigned int i = 0; i < l->nitems; i++)
		{
			struct region_t * r = &l->items[i];
			int x0 = (r->x > 0) ? r->x : 0;
			int y0 = (r->y > 0) ? r->y : 0;
			int x1 = (r->x + r->w < 128) ? (r->x + r->w) : 128;
			int y1 = (r->y + r->h < 64) ? (r->y + r->h) : 64;
			if((x0 < x1) && (y0 < y1))
			{
				ssd1315_convert(pdat, p, stride, x0, y0, x1, y1);
				ssd1315_refresh(pdat, x0, y0, x1 - x0, y1 - y0);
			}
		}
	}
	return 0;
}

static void fb_wait(struct framebuffer_t * fb)
{
}

static struct device_t * fb_ssd1315_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_ssd1315_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x3c), 0);
	if(!i2cdev)
		return NULL;

	if(!ssd1315_detect(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct fb_ssd1315_pdata_t));
	if(!pdat)
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	fb = xos_mem_malloc(sizeof(struct framebuffer_t));
	if(!fb)
	{
		i2c_device_free(i2cdev);
		xos_mem_free(pdat);
		return NULL;
	}

	xos_memset(pdat, 0, sizeof(struct fb_ssd1315_pdata_t));
	pdat->dev = i2cdev;
	pdat->dither = dt_read_bool(n, "dither", 0);
	pdat->brightness = -1;

	fb->name = alloc_device_name(dt_read_name(n), dt_read_id(n));
	fb->width = 128;
	fb->height = 64;
	fb->pwidth = dt_read_int(n, "physical-width", 22);
	fb->pheight = dt_read_int(n, "physical-height", 11);
	fb->setbl = fb_setbl;
	fb->getbl = fb_getbl;
	fb->create = fb_create;
	fb->destroy = fb_destroy;
	fb->present = fb_present;
	fb->wait = fb_wait;
	fb->priv = pdat;

	ssd1315_initial(pdat);
	ssd1315_refresh(pdat, 0, 0, 128, 64);

	if(!(dev = register_framebuffer(fb, drv)))
	{
		i2c_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
		return NULL;
	}
	return dev;
}

static void fb_ssd1315_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_ssd1315_pdata_t * pdat = (struct fb_ssd1315_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		i2c_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_ssd1315_suspend(struct device_t * dev)
{
}

static void fb_ssd1315_resume(struct device_t * dev)
{
}

static struct driver_t fb_ssd1315 = {
	.name		= "fb-ssd1315",
	.probe		= fb_ssd1315_probe,
	.remove		= fb_ssd1315_remove,
	.suspend	= fb_ssd1315_suspend,
	.resume		= fb_ssd1315_resume,
};

static void fb_ssd1315_driver_init(void)
{
	register_driver(&fb_ssd1315);
}

static void fb_ssd1315_driver_exit(void)
{
	unregister_driver(&fb_ssd1315);
}

driver_initcall(fb_ssd1315_driver_init);
driver_exitcall(fb_ssd1315_driver_exit);
