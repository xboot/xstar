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

#include <kernel/vision/rectangle.h>

static void vision_fill_gray(struct vision_t * v, int x, int y, int w, int h, uint8_t gray)
{
	int x1 = XMAX(0, x);
	int x2 = XMIN(v->width, x + w);
	if(x1 <= x2)
	{
		int y1 = XMAX(0, y);
		int y2 = XMIN(v->height, y + h);
		if(y1 <= y2)
		{
			int width = x2 - x1;
			int height = y2 - y1;
			unsigned char * pv = &((unsigned char *)v->datas)[y1 * v->width + x1];
			for(int i = 0; i < height; i++, pv += v->width)
				xos_memset(pv, gray, width);
		}
	}
}

static void vision_fill_rgb(struct vision_t * v, int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b)
{
	int x1 = XMAX(0, x);
	int x2 = XMIN(v->width, x + w);
	if(x1 <= x2)
	{
		int y1 = XMAX(0, y);
		int y2 = XMIN(v->height, y + h);
		if(y1 <= y2)
		{
			int width = x2 - x1;
			int height = y2 - y1;
			unsigned char * pr = &((unsigned char *)v->datas)[v->npixel * 0 + y1 * v->width + x1];
			for(int i = 0; i < height; i++, pr += v->width)
				xos_memset(pr, r, width);
			unsigned char * pg = &((unsigned char *)v->datas)[v->npixel * 1 + y1 * v->width + x1];
			for(int i = 0; i < height; i++, pg += v->width)
				xos_memset(pg, g, width);
			unsigned char * pb = &((unsigned char *)v->datas)[v->npixel * 2 + y1 * v->width + x1];
			for(int i = 0; i < height; i++, pb += v->width)
				xos_memset(pb, b, width);
		}
	}
}

void vision_rectangle(struct vision_t * v, int x, int y, int w, int h, int thickness, struct color_t * c)
{
	if(v)
	{
		switch(v->type)
		{
		case VISION_TYPE_GRAY:
			{
				uint8_t gray = 0xff;
				if(c)
					gray = ((c->r) * 13933L + (c->g) * 46871L + (c->b) * 4732L) >> 16;
				if(thickness > 0)
				{
					x -= thickness >> 1;
					y -= thickness >> 1;
					w += thickness;
					h += thickness;
					vision_fill_gray(v, x, y, w, thickness, gray);
					vision_fill_gray(v, x, y + h - thickness, w, thickness, gray);
					vision_fill_gray(v, x, y + thickness, thickness, h - (thickness << 1), gray);
					vision_fill_gray(v, x + w - thickness, y + thickness, thickness, h - (thickness << 1), gray);
				}
				else
					vision_fill_gray(v, x, y, w, h, gray);
			}
			break;
		case VISION_TYPE_RGB:
			{
				uint8_t r = 0xff, g = 0xff, b = 0xff;
				if(c)
				{
					r = c->r;
					g = c->g;
					b = c->b;
				}
				if(thickness > 0)
				{
					x -= thickness >> 1;
					y -= thickness >> 1;
					w += thickness;
					h += thickness;
					vision_fill_rgb(v, x, y, w, thickness, r, g, b);
					vision_fill_rgb(v, x, y + h - thickness, w, thickness, r, g, b);
					vision_fill_rgb(v, x, y + thickness, thickness, h - (thickness << 1), r, g, b);
					vision_fill_rgb(v, x + w - thickness, y + thickness, thickness, h - (thickness << 1), r, g, b);
				}
				else
					vision_fill_rgb(v, x, y, w, h, r, g, b);
			}
			break;
		default:
			break;
		}
	}
}
