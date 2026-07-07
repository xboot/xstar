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

#include <kernel/vision/text.h>

static void vision_callback_gray(void * data, int x0, int y0, void * gray, int width, int height)
{
	struct {
		struct vision_t * v;
		uint8_t gray;
	} * ctx = data;
	uint8_t * p, * q = &((uint8_t *)vision_get_datas(ctx->v))[y0 * vision_get_width(ctx->v) + x0];
	uint8_t * pgray, * qgray = gray;
	int w = XMIN(width, vision_get_width(ctx->v) - x0);
	int h = XMIN(height, vision_get_height(ctx->v) - y0);
	int x, y;

	for(y = 0; y < h; y++, qgray += width, q += vision_get_width(ctx->v))
	{
		for(x = 0, pgray = qgray, p = q; x < w; x++, pgray++, p++)
		{
			uint8_t alpha = *pgray;
			if(alpha > 0)
			{
				if(alpha == 255)
					*p = ctx->gray;
				else
					*p = XCLAMP((int)(XDIV255((*p) * (255 - alpha)) + XDIV255(ctx->gray * alpha)), 0, 255);
			}
		}
	}
}

static void vision_callback_rgb(void * data, int x0, int y0, void * gray, int width, int height)
{
	struct {
		struct vision_t * v;
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} * ctx = data;
	uint8_t * pr, * qr = &((uint8_t *)vision_get_datas(ctx->v))[vision_get_npixel(ctx->v) * 0 + y0 * vision_get_width(ctx->v) + x0];
	uint8_t * pg, * qg = &((uint8_t *)vision_get_datas(ctx->v))[vision_get_npixel(ctx->v) * 1 + y0 * vision_get_width(ctx->v) + x0];
	uint8_t * pb, * qb = &((uint8_t *)vision_get_datas(ctx->v))[vision_get_npixel(ctx->v) * 2 + y0 * vision_get_width(ctx->v) + x0];
	uint8_t * pgray, * qgray = gray;
	int w = XMIN(width, vision_get_width(ctx->v) - x0);
	int h = XMIN(height, vision_get_height(ctx->v) - y0);
	int x, y;

	for(y = 0; y < h; y++, qgray += width, qr += vision_get_width(ctx->v), qg += vision_get_width(ctx->v), qb += vision_get_width(ctx->v))
	{
		for(x = 0, pgray = qgray, pr = qr, pg = qg, pb = qb; x < w; x++, pgray++, pr++, pg++, pb++)
		{
			uint8_t alpha = *pgray;
			if(alpha > 0)
			{
				if(alpha == 255)
				{
					*pr = ctx->r;
					*pg = ctx->g;
					*pb = ctx->b;
				}
				else
				{
					*pr = XCLAMP((int)(XDIV255((*pr) * (255 - alpha)) + XDIV255(ctx->r * alpha)), 0, 255);
					*pg = XCLAMP((int)(XDIV255((*pg) * (255 - alpha)) + XDIV255(ctx->g * alpha)), 0, 255);
					*pb = XCLAMP((int)(XDIV255((*pb) * (255 - alpha)) + XDIV255(ctx->b * alpha)), 0, 255);
				}
			}
		}
	}
}

void vision_text(struct vision_t * v, int x, int y, int wrap, const char * family, enum font_style_t style, int size, struct color_t * c, const char * fmt, ...)
{
	if(v)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			if(v->type == VISION_TYPE_GRAY)
				font_text_render(family, style, size, x, y, wrap, str, vision_callback_gray, (void *)&(struct{struct vision_t * v;uint8_t gray;}){ v, c ? ((c->r) * 13933L + (c->g) * 46871L + (c->b) * 4732L) >> 16 : 0xff});
			else if(v->type == VISION_TYPE_RGB)
				font_text_render(family, style, size, x, y, wrap, str, vision_callback_rgb, (void *)&(struct{struct vision_t * v;uint8_t r;uint8_t g;uint8_t b;}){ v, c ? c->r : 0xff, c ? c->g : 0xff, c ? c->b : 0xff });
			xos_mem_free(str);
		}
	}
}

void vision_icon(struct vision_t * v, int x, int y, const char * family, int size, uint32_t code, struct color_t * c)
{
	if(v)
	{
		if(v->type == VISION_TYPE_GRAY)
			font_icon_render(family, size, x, y, code, vision_callback_gray, (void *)&(struct{struct vision_t * v;uint8_t gray;}){ v, c ? ((c->r) * 13933L + (c->g) * 46871L + (c->b) * 4732L) >> 16 : 0xff});
		else if(v->type == VISION_TYPE_RGB)
			font_icon_render(family, size, x, y, code, vision_callback_rgb, (void *)&(struct{struct vision_t * v;uint8_t r;uint8_t g;uint8_t b;}){ v, c ? c->r : 0xff, c ? c->g : 0xff, c ? c->b : 0xff });
	}
}
