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
 * ST77912 - Sitronix lcd display controller
 *
 * Example:
 *   "fb-st77912": {
 *	     "spi-bus": "spi-v821.1",
 *	     "chip-select": 0,
 *	     "type": 0,
 *	     "mode": 0,
 *	     "speed": 50000000,
 *	     "reset-gpio": 111,
 *	     "reset-gpio-config": 1,
 *	     "cd-gpio": 101,
 *	     "cd-gpio-config": 1,
 *	     "width": 240,
 *	     "height": 240,
 *	     "physical-width": 34,
 *	     "physical-height": 34,
 *	     "backlight": null
 *   }
 */

struct fb_st77912_pdata_t {
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

static void st77912_write_command(struct fb_st77912_pdata_t * pdat, uint8_t cmd)
{
	spi_device_select(pdat->dev);
	gpio_set_value(pdat->cd, 0);
	spi_device_write_then_read(pdat->dev, &cmd, 1, 0, 0);
	gpio_set_value(pdat->cd, 1);
	spi_device_deselect(pdat->dev);
}

static void st77912_write_data(struct fb_st77912_pdata_t * pdat, uint8_t dat)
{
	spi_device_select(pdat->dev);
	spi_device_write_then_read(pdat->dev, &dat, 1, 0, 0);
	spi_device_deselect(pdat->dev);
}

static void st77912_set_window(struct fb_st77912_pdata_t * pdat, int x, int y, int w, int h)
{
	st77912_write_command(pdat, 0x2a);
	st77912_write_data(pdat, (x >> 8) & 0xff);
	st77912_write_data(pdat, (x >> 0) & 0xff);
	st77912_write_data(pdat, ((x + w - 1) >> 8) & 0xff);
	st77912_write_data(pdat, ((x + w - 1) >> 0) & 0xff);
	st77912_write_command(pdat, 0x2b);
	st77912_write_data(pdat, (y >> 8) & 0xff);
	st77912_write_data(pdat, (y >> 0) & 0xff);
	st77912_write_data(pdat, ((y + h - 1) >> 8) & 0xff);
	st77912_write_data(pdat, ((y + h - 1) >> 0) & 0xff);
}

static void st77912_init(struct fb_st77912_pdata_t * pdat)
{
	st77912_write_command(pdat, 0xf0);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xf1);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x7a);
	st77912_write_data(pdat, 0x83);
	st77912_write_command(pdat, 0xb0);
	st77912_write_data(pdat, 0x5e);
	st77912_write_command(pdat, 0xb1);
	st77912_write_data(pdat, 0x55);
	st77912_write_command(pdat, 0xb2);
	st77912_write_data(pdat, 0x24);
	st77912_write_command(pdat, 0xb4);
	st77912_write_data(pdat, 0xa7);
	st77912_write_command(pdat, 0xb5);
	st77912_write_data(pdat, 0x54);
	st77912_write_command(pdat, 0xb6);
	st77912_write_data(pdat, 0x8b);
	st77912_write_command(pdat, 0xb7);
	st77912_write_data(pdat, 0x50);
	st77912_write_command(pdat, 0xba);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xbb);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0xbc);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0xbd);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xc0);
	st77912_write_data(pdat, 0x80);
	st77912_write_command(pdat, 0xc1);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0xc2);
	st77912_write_data(pdat, 0x54);
	st77912_write_command(pdat, 0xc3);
	st77912_write_data(pdat, 0x80);
	st77912_write_command(pdat, 0xc4);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0xc5);
	st77912_write_data(pdat, 0x54);
	st77912_write_command(pdat, 0xc6);
	st77912_write_data(pdat, 0xa9);
	st77912_write_command(pdat, 0xc7);
	st77912_write_data(pdat, 0x41);
	st77912_write_command(pdat, 0xc8);
	st77912_write_data(pdat, 0x51);
	st77912_write_command(pdat, 0xc9);
	st77912_write_data(pdat, 0xa9);
	st77912_write_command(pdat, 0xca);
	st77912_write_data(pdat, 0x41);
	st77912_write_command(pdat, 0xcb);
	st77912_write_data(pdat, 0x51);
	st77912_write_command(pdat, 0xd0);
	st77912_write_data(pdat, 0x80);
	st77912_write_command(pdat, 0xd1);
	st77912_write_data(pdat, 0xf0);
	st77912_write_command(pdat, 0xd2);
	st77912_write_data(pdat, 0xf0);
	st77912_write_command(pdat, 0xf5);
	st77912_write_data(pdat, 0x00);
	st77912_write_data(pdat, 0xa5);
	st77912_write_command(pdat, 0xdd);
	st77912_write_data(pdat, 0x36);
	st77912_write_command(pdat, 0xde);
	st77912_write_data(pdat, 0x36);
	st77912_write_command(pdat, 0xf0);
	st77912_write_data(pdat, 0x02);
	st77912_write_command(pdat, 0xf1);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xe0);
	st77912_write_data(pdat, 0xf0);
	st77912_write_data(pdat, 0x16);
	st77912_write_data(pdat, 0x1c);
	st77912_write_data(pdat, 0x0a);
	st77912_write_data(pdat, 0x0a);
	st77912_write_data(pdat, 0x06);
	st77912_write_data(pdat, 0x3e);
	st77912_write_data(pdat, 0x33);
	st77912_write_data(pdat, 0x53);
	st77912_write_data(pdat, 0x07);
	st77912_write_data(pdat, 0x14);
	st77912_write_data(pdat, 0x13);
	st77912_write_data(pdat, 0x31);
	st77912_write_data(pdat, 0x35);
	st77912_write_command(pdat, 0xe1);
	st77912_write_data(pdat, 0xf0);
	st77912_write_data(pdat, 0x16);
	st77912_write_data(pdat, 0x1c);
	st77912_write_data(pdat, 0x0a);
	st77912_write_data(pdat, 0x0a);
	st77912_write_data(pdat, 0x06);
	st77912_write_data(pdat, 0x3e);
	st77912_write_data(pdat, 0x33);
	st77912_write_data(pdat, 0x53);
	st77912_write_data(pdat, 0x07);
	st77912_write_data(pdat, 0x14);
	st77912_write_data(pdat, 0x13);
	st77912_write_data(pdat, 0x31);
	st77912_write_data(pdat, 0x35);
	st77912_write_command(pdat, 0xf0);
	st77912_write_data(pdat, 0x10);
	st77912_write_command(pdat, 0xf3);
	st77912_write_data(pdat, 0x10);
	st77912_write_command(pdat, 0xe0);
	st77912_write_data(pdat, 0x0b);
	st77912_write_command(pdat, 0xe1);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xe2);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xe3);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xe4);
	st77912_write_data(pdat, 0xe0);
	st77912_write_command(pdat, 0xe5);
	st77912_write_data(pdat, 0x06);
	st77912_write_command(pdat, 0xe6);
	st77912_write_data(pdat, 0x21);
	st77912_write_command(pdat, 0xe7);
	st77912_write_data(pdat, 0x80);
	st77912_write_command(pdat, 0xe8);
	st77912_write_data(pdat, 0x0a);
	st77912_write_command(pdat, 0xe9);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xea);
	st77912_write_data(pdat, 0x04);
	st77912_write_command(pdat, 0xeb);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xec);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xed);
	st77912_write_data(pdat, 0x24);
	st77912_write_command(pdat, 0xee);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xef);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xf8);
	st77912_write_data(pdat, 0xff);
	st77912_write_command(pdat, 0xf9);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xfa);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xfb);
	st77912_write_data(pdat, 0x30);
	st77912_write_command(pdat, 0xfc);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xfd);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xfe);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xff);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x60);
	st77912_write_data(pdat, 0x40);
	st77912_write_command(pdat, 0x61);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0x62);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x63);
	st77912_write_data(pdat, 0x41);
	st77912_write_command(pdat, 0x64);
	st77912_write_data(pdat, 0xed);
	st77912_write_command(pdat, 0x65);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x66);
	st77912_write_data(pdat, 0x40);
	st77912_write_command(pdat, 0x67);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x68);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x69);
	st77912_write_data(pdat, 0x40);
	st77912_write_command(pdat, 0x6a);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x6b);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x70);
	st77912_write_data(pdat, 0x40);
	st77912_write_command(pdat, 0x71);
	st77912_write_data(pdat, 0x07);
	st77912_write_command(pdat, 0x72);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x73);
	st77912_write_data(pdat, 0x41);
	st77912_write_command(pdat, 0x74);
	st77912_write_data(pdat, 0xec);
	st77912_write_command(pdat, 0x75);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x76);
	st77912_write_data(pdat, 0x40);
	st77912_write_command(pdat, 0x77);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x78);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x79);
	st77912_write_data(pdat, 0x40);
	st77912_write_command(pdat, 0x7a);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x7b);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x80);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0x81);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x82);
	st77912_write_data(pdat, 0x0a);
	st77912_write_command(pdat, 0x83);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x84);
	st77912_write_data(pdat, 0xea);
	st77912_write_command(pdat, 0x85);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x86);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x87);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x88);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0x89);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x8a);
	st77912_write_data(pdat, 0x0c);
	st77912_write_command(pdat, 0x8b);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x8c);
	st77912_write_data(pdat, 0xec);
	st77912_write_command(pdat, 0x8d);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x8e);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x8f);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x90);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0x91);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x92);
	st77912_write_data(pdat, 0x0e);
	st77912_write_command(pdat, 0x93);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x94);
	st77912_write_data(pdat, 0xee);
	st77912_write_command(pdat, 0x95);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x96);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x97);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x98);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0x99);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x9a);
	st77912_write_data(pdat, 0x10);
	st77912_write_command(pdat, 0x9b);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x9c);
	st77912_write_data(pdat, 0xf0);
	st77912_write_command(pdat, 0x9d);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x9e);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x9f);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xa0);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0xa1);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xa2);
	st77912_write_data(pdat, 0x09);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa4);
	st77912_write_data(pdat, 0xe9);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xa6);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xa7);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xa8);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0xa9);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xaa);
	st77912_write_data(pdat, 0x0b);
	st77912_write_command(pdat, 0xab);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xac);
	st77912_write_data(pdat, 0xeb);
	st77912_write_command(pdat, 0xad);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xae);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xaf);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xb0);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0xb1);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xb2);
	st77912_write_data(pdat, 0x0d);
	st77912_write_command(pdat, 0xb3);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xb4);
	st77912_write_data(pdat, 0xed);
	st77912_write_command(pdat, 0xb5);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xb6);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xb7);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xb8);
	st77912_write_data(pdat, 0x48);
	st77912_write_command(pdat, 0xb9);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xba);
	st77912_write_data(pdat, 0x0f);
	st77912_write_command(pdat, 0xbb);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xbc);
	st77912_write_data(pdat, 0xef);
	st77912_write_command(pdat, 0xbd);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xbe);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xbf);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xc0);
	st77912_write_data(pdat, 0x88);
	st77912_write_command(pdat, 0xc1);
	st77912_write_data(pdat, 0x99);
	st77912_write_command(pdat, 0xc2);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xc3);
	st77912_write_data(pdat, 0xaa);
	st77912_write_command(pdat, 0xc4);
	st77912_write_data(pdat, 0xbb);
	st77912_write_command(pdat, 0xc5);
	st77912_write_data(pdat, 0x74);
	st77912_write_command(pdat, 0xc6);
	st77912_write_data(pdat, 0x65);
	st77912_write_command(pdat, 0xc7);
	st77912_write_data(pdat, 0x56);
	st77912_write_command(pdat, 0xc8);
	st77912_write_data(pdat, 0x47);
	st77912_write_command(pdat, 0xc9);
	st77912_write_data(pdat, 0x10);
	st77912_write_command(pdat, 0xd0);
	st77912_write_data(pdat, 0x88);
	st77912_write_command(pdat, 0xd1);
	st77912_write_data(pdat, 0x99);
	st77912_write_command(pdat, 0xd2);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xd3);
	st77912_write_data(pdat, 0xaa);
	st77912_write_command(pdat, 0xd4);
	st77912_write_data(pdat, 0xbb);
	st77912_write_command(pdat, 0xd5);
	st77912_write_data(pdat, 0x74);
	st77912_write_command(pdat, 0xd6);
	st77912_write_data(pdat, 0x65);
	st77912_write_command(pdat, 0xd7);
	st77912_write_data(pdat, 0x56);
	st77912_write_command(pdat, 0xd8);
	st77912_write_data(pdat, 0x47);
	st77912_write_command(pdat, 0xd9);
	st77912_write_data(pdat, 0x10);
	st77912_write_command(pdat, 0xf0);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0xf2);
	st77912_write_data(pdat, 0x08);
	st77912_write_command(pdat, 0x71);
	st77912_write_data(pdat, 0x03);
	st77912_write_command(pdat, 0x73);
	st77912_write_data(pdat, 0x30);
	st77912_write_command(pdat, 0x76);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0x78);
	st77912_write_data(pdat, 0x33);
	st77912_write_command(pdat, 0x79);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x7b);
	st77912_write_data(pdat, 0xfa);
	st77912_write_command(pdat, 0x7e);
	st77912_write_data(pdat, 0x16);
	st77912_write_command(pdat, 0x86);
	st77912_write_data(pdat, 0x55);
	st77912_write_command(pdat, 0x89);
	st77912_write_data(pdat, 0x61);
	st77912_write_command(pdat, 0x8a);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xf0);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xf1);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa0);
	st77912_write_data(pdat, 0x0b);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x2a);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x2b);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x2c);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x2d);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x2e);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x2f);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x30);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x31);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x32);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa3);
	st77912_write_data(pdat, 0x33);
	st77912_write_command(pdat, 0xa5);
	st77912_write_data(pdat, 0xc3);
	st77912_write_command(pdat, 0x00);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0xa0);
	st77912_write_data(pdat, 0x09);
	st77912_write_command(pdat, 0xf0);
	st77912_write_data(pdat, 0x00);
	st77912_write_command(pdat, 0xf1);
	st77912_write_data(pdat, 0x10);
	st77912_write_command(pdat, 0xf2);
	st77912_write_data(pdat, 0x84);
	st77912_write_command(pdat, 0xf3);
	st77912_write_data(pdat, 0x01);
	st77912_write_command(pdat, 0x3a);
	st77912_write_data(pdat, 0x05);
	st77912_write_command(pdat, 0x21);
	st77912_write_command(pdat, 0x11);
	st77912_write_command(pdat, 0x29);
}

static void fb_setbl(struct framebuffer_t * fb, int brightness)
{
	struct fb_st77912_pdata_t * pdat = (struct fb_st77912_pdata_t *)fb->priv;
	led_set_brightness(pdat->backlight, brightness);
}

static int fb_getbl(struct framebuffer_t * fb)
{
	struct fb_st77912_pdata_t * pdat = (struct fb_st77912_pdata_t *)fb->priv;
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
	struct fb_st77912_pdata_t * pdat = (struct fb_st77912_pdata_t *)fb->priv;
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
			st77912_set_window(pdat, r->x, r->y, r->w, r->h);
			st77912_write_command(pdat, 0x2c);
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
		st77912_set_window(pdat, 0, 0, pdat->width, pdat->height);
		st77912_write_command(pdat, 0x2c);
		spi_device_select(pdat->dev);
		spi_device_write_then_read(pdat->dev, txbuf, pdat->width * pdat->height * 2, 0, 0);
		spi_device_deselect(pdat->dev);
	}
}

static struct device_t * fb_st77912_probe(struct driver_t * drv, struct dtnode_t * n)
{
	struct fb_st77912_pdata_t * pdat;
	struct framebuffer_t * fb;
	struct device_t * dev;
	struct spi_device_t * spidev;
	int cd = dt_read_int(n, "cd-gpio", -1);

	if(!gpio_is_valid(cd))
		return NULL;

	spidev = spi_device_alloc(dt_read_string(n, "spi-bus", NULL), dt_read_int(n, "chip-select", 0), dt_read_int(n, "type", 0), dt_read_int(n, "mode", 0), 8, dt_read_int(n, "speed", 0));
	if(!spidev)
		return NULL;

	pdat = xos_mem_malloc(sizeof(struct fb_st77912_pdata_t));
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
	pdat->width = dt_read_int(n, "width", 240);
	pdat->height = dt_read_int(n, "height", 240);
	pdat->pwidth = dt_read_int(n, "physical-width", 34);
	pdat->pheight = dt_read_int(n, "physical-height", 34);
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
	st77912_init(pdat);

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

static void fb_st77912_remove(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_st77912_pdata_t * pdat = (struct fb_st77912_pdata_t *)fb->priv;

	if(fb)
	{
		unregister_framebuffer(fb);
		spi_device_free(pdat->dev);
		free_device_name(fb->name);
		xos_mem_free(fb->priv);
		xos_mem_free(fb);
	}
}

static void fb_st77912_suspend(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_st77912_pdata_t * pdat = (struct fb_st77912_pdata_t *)fb->priv;

	pdat->brightness = led_get_brightness(pdat->backlight);
	led_set_brightness(pdat->backlight, 0);
}

static void fb_st77912_resume(struct device_t * dev)
{
	struct framebuffer_t * fb = (struct framebuffer_t *)dev->priv;
	struct fb_st77912_pdata_t * pdat = (struct fb_st77912_pdata_t *)fb->priv;

	led_set_brightness(pdat->backlight, pdat->brightness);
}

static struct driver_t fb_st77912 = {
	.name		= "fb-st77912",
	.probe		= fb_st77912_probe,
	.remove		= fb_st77912_remove,
	.suspend	= fb_st77912_suspend,
	.resume		= fb_st77912_resume,
};

static void fb_st77912_driver_init(void)
{
	register_driver(&fb_st77912);
}

static void fb_st77912_driver_exit(void)
{
	unregister_driver(&fb_st77912);
}

driver_initcall(fb_st77912_driver_init);
driver_exitcall(fb_st77912_driver_exit);
