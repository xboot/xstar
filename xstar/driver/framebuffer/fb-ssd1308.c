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
 * SSD1308 - Solomon oled display controller
 *
 * Required properties:
 * - i2c-bus: i2c bus name which the device is attached to
 *
 * Optional properties:
 * - slave-address: i2c slave address (default: 0x3c)
 * - physical-width: display physical width in millimeters
 * - physical-height: display physical height in millimeters
 *
 * Example device tree node:
 *   "fb-ssd1308": {
 *       "i2c-bus": "i2c-gpio.0",
 *       "slave-address": 60,
 *		 "physical-width": 22,
 *		 "physical-height": 11
 *   }
 */

struct fb_ssd1308_pdata_t {
	struct i2c_device_t * dev;
	uint8_t buffer[128 * 64 / 8];
	int brightness;
};

static int ssd1308_read(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int ssd1308_write(struct i2c_device_t * dev, uint8_t reg, uint8_t * buf, int len)
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

static int ssd1308_detect(struct i2c_device_t * dev)
{
	uint8_t val;

	if(!ssd1308_read(dev, 0x0, &val, 1))
		return 0;
	return 1;
}

static void ssd1308_initial(struct fb_ssd1308_pdata_t * pdat)
{
	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xae }, 1);
	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xa6 }, 1);
	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0x20 }, 1);
	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0x02 }, 1);
	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xaf }, 1);
}

static void ssd1308_refresh(struct fb_ssd1308_pdata_t * pdat)
{
	for(int i = 0; i < 8; i++)
	{
		ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xb0 + i }, 1);
		ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0x00 }, 1);
		ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0x10 }, 1);
		ssd1308_write(pdat->dev, 0x40, &pdat->buffer[128 * i], 128);
	}
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_ssd1308_pdata_t * pdat = (struct fb_ssd1308_pdata_t *)fb->priv;

	if(pdat->brightness != brightness)
	{
		ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0x81 }, 1);
		ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ (0xff * brightness / 1000) & 0xff }, 1);
		if(brightness == 0)
			ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xae }, 1);
		else
			ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xaf }, 1);
		pdat->brightness = brightness;
	}
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_ssd1308_pdata_t * pdat = (struct fb_ssd1308_pdata_t *)fb->priv;
	return pdat->brightness;
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
	struct fb_ssd1308_pdata_t * pdat = (struct fb_ssd1308_pdata_t *)fb->priv;
	uint32_t * p = s->pixels;

	for(int y = 0; y < 64; y++)
	{
		for(int x = 0; x < 128; x++)
		{
			if((*p++) & 0x00ffffff)
				pdat->buffer[((y >> 3) << 7) + x] |= (1 << (y & 0x7));
			else
				pdat->buffer[((y >> 3) << 7) + x] &= ~(1 << (y & 0x7));
		}
	}
	ssd1308_refresh(pdat);
}

static struct device_t * fb_ssd1308_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_ssd1308_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct i2c_device_t * i2cdev;

	i2cdev = i2c_device_alloc(dt_read_string(n, "i2c-bus", NULL), dt_read_int(n, "slave-address", 0x3c), 0);
	if(!i2cdev)
		return NULL;

	if(!ssd1308_detect(i2cdev))
	{
		i2c_device_free(i2cdev);
		return NULL;
	}

	pdat = xos_mem_malloc(sizeof(struct fb_ssd1308_pdata_t));
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

	xos_memset(pdat, 0, sizeof(struct fb_ssd1308_pdata_t));
	pdat->dev = i2cdev;
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
	fb->priv = pdat;

	ssd1308_initial(pdat);
	ssd1308_refresh(pdat);

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

static void fb_ssd1308_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_ssd1308_pdata_t * pdat = (struct fb_ssd1308_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		i2c_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_ssd1308_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_ssd1308_pdata_t * pdat = (struct fb_ssd1308_pdata_t *)fb->priv;

	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xae }, 1);
}

static void fb_ssd1308_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_ssd1308_pdata_t * pdat = (struct fb_ssd1308_pdata_t *)fb->priv;

	ssd1308_write(pdat->dev, 0x80, (uint8_t[]){ 0xaf }, 1);
}

static struct driver_t fb_ssd1308 = {
	.name		= "fb-ssd1308",
	.probe		= fb_ssd1308_probe,
	.remove		= fb_ssd1308_remove,
	.suspend	= fb_ssd1308_suspend,
	.resume		= fb_ssd1308_resume,
};

static void fb_ssd1308_driver_init(void)
{
	register_driver(&fb_ssd1308);
}

static void fb_ssd1308_driver_exit(void)
{
	unregister_driver(&fb_ssd1308);
}

driver_initcall(fb_ssd1308_driver_init);
driver_exitcall(fb_ssd1308_driver_exit);
