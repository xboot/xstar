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
#include <driver/printer/printer.h>

static ssize_t printer_read_dpl(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	int dpl = printer_get_dpl(p);
	return xos_sprintf(buf, "%d", dpl);
}

static ssize_t printer_read_dpi(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	int dpi = printer_get_dpi(p);
	return xos_sprintf(buf, "%d", dpi);
}

static ssize_t printer_read_paper(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	int paper = printer_get_paper(p);
	return xos_sprintf(buf, "%d", paper ? 1 : 0);
}

static ssize_t printer_read_temperature(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	int temperature = printer_get_temperature(p);
	return xos_sprintf(buf, "%.3f", (float)temperature / 1000.0f);
}

static ssize_t printer_read_density(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	float density = printer_get_density(p);
	return xos_sprintf(buf, "%g", density);
}

static ssize_t printer_write_density(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	float density = xos_strtod(buf, NULL);
	printer_set_density(p, density);
	return size;
}

static ssize_t printer_write_feed(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	int lines = xos_strtol(buf, NULL, 0);
	printer_feed(p, lines);
	return size;
}

static ssize_t printer_write_cut(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	printer_cut(p);
	return size;
}

static ssize_t printer_write_standby(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	printer_standby(p);
	return size;
}

static ssize_t printer_write_testpage(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;

	/*
	 * test page
	 */
	printer_print_text(p, PRINTER_ALIGN_CENTER, NULL, FONT_STYLE_BOLD, 64, "Test Page");
	printer_feed(p, 16);
	printer_print_hr(p, 8, 8);
	printer_feed(p, 16);
	/*
	 * banner
	 */
	printer_print_text(p, PRINTER_ALIGN_LEFT, NULL, FONT_STYLE_REGULAR, 32, "%s", xstar_banner());
	printer_feed(p, 16);
	printer_print_hr(p, 8, 8);
	printer_feed(p, 16);
	/*
	 * text
	 */
	for(int i = 24; i < 48; i += 4)
		printer_print_text(p, PRINTER_ALIGN_LEFT, NULL, FONT_STYLE_REGULAR, i, "[%d] Powered by xstar", i);
	printer_feed(p, 16);
	printer_print_hr(p, 8, 8);
	printer_feed(p, 16);
	/*
	 * pattern
	 */
	for(int j = 0; j < 16; j++)
	{
		for(int i = 0; i < 16; i++)
			printer_print_hr(p, 16 - j, 16);
	}
	for(int j = 0; j < 16; j++)
	{
		for(int i = 0; i < 16; i++)
			printer_print_hr(p, j + 1, 16);
	}
	printer_feed(p, 16);
	printer_print_hr(p, 8, 8);
	printer_feed(p, 16);
	/*
	 * qrcode
	 */
	printer_print_qrcode(p, PRINTER_ALIGN_CENTER, 8, "%s", xos_copyright_uniqueid());
	printer_feed(p, 16);
	printer_print_hr(p, 8, 8);
	printer_feed(p, 16);
	/*
	 * standby
	 */
	printer_feed(p, 64);
	printer_cut(p);
	printer_standby(p);

	return size;
}

static ssize_t printer_write_text(struct kobj_t * kobj, void * buf, size_t size)
{
	struct printer_t * p = (struct printer_t *)kobj->priv;
	printer_print_text(p, PRINTER_ALIGN_LEFT, NULL, FONT_STYLE_REGULAR, 32, "%s", (char *)buf);
	printer_feed(p, 100);
	printer_cut(p);
	printer_standby(p);
	return size;
}

struct printer_t * search_printer(const char * name)
{
	struct device_t * dev;

	dev = search_device(name, DEVICE_TYPE_PRINTER);
	if(!dev)
		return NULL;
	return (struct printer_t *)dev->priv;
}

struct printer_t * search_first_printer(void)
{
	struct device_t * dev;

	dev = search_first_device(DEVICE_TYPE_PRINTER);
	if(!dev)
		return NULL;
	return (struct printer_t *)dev->priv;
}

struct device_t * register_printer(struct printer_t * p, struct driver_t * drv)
{
	struct device_t * dev;

	if(!p || !p->name)
		return NULL;

	dev = xos_mem_malloc(sizeof(struct device_t));
	if(!dev)
		return NULL;

	dev->name = xos_strdup(p->name);
	dev->type = DEVICE_TYPE_PRINTER;
	dev->driver = drv;
	dev->priv = p;
	dev->kobj = kobj_alloc_directory(dev->name);
	kobj_add_regular(dev->kobj, "dpl", printer_read_dpl, NULL, p);
	kobj_add_regular(dev->kobj, "dpi", printer_read_dpi, NULL, p);
	kobj_add_regular(dev->kobj, "paper", printer_read_paper, NULL, p);
	kobj_add_regular(dev->kobj, "temperature", printer_read_temperature, NULL, p);
	kobj_add_regular(dev->kobj, "density", printer_read_density, printer_write_density, p);
	kobj_add_regular(dev->kobj, "feed", NULL, printer_write_feed, p);
	kobj_add_regular(dev->kobj, "cut", NULL, printer_write_cut, p);
	kobj_add_regular(dev->kobj, "standby", NULL, printer_write_standby, p);
	kobj_add_regular(dev->kobj, "testpage", NULL, printer_write_testpage, p);
	kobj_add_regular(dev->kobj, "text", NULL, printer_write_text, p);

	if(!register_device(dev))
	{
		kobj_remove_self(dev->kobj);
		xos_mem_free(dev->name);
		xos_mem_free(dev);
		return NULL;
	}
	return dev;
}

void unregister_printer(struct printer_t * p)
{
	struct device_t * dev;

	if(p && p->name)
	{
		dev = search_device(p->name, DEVICE_TYPE_PRINTER);
		if(dev && unregister_device(dev))
		{
			kobj_remove_self(dev->kobj);
			xos_mem_free(dev->name);
			xos_mem_free(dev);
		}
	}
}

void printer_print(struct printer_t * p, unsigned char * buf, unsigned int len)
{
	if(p && p->print)
	{
		if(buf && (len > 0))
			p->print(p, buf, len);
	}
}

void printer_feed(struct printer_t * p, int lines)
{
	if(p && p->feed)
		p->feed(p, lines);
}

void printer_cut(struct printer_t * p)
{
	if(p && p->cut)
		p->cut(p);
}

void printer_standby(struct printer_t * p)
{
	if(p && p->standby)
		p->standby(p);
}

void printer_print_hr(struct printer_t * p, int black, int white)
{
	if(p)
	{
		int dpl = printer_get_dpl(p);
		unsigned char line[dpl];
		if(black <= 0)
		{
			for(int i = 0; i < dpl; i++)
				line[i] = 0;
		}
		else if(white <= 0)
		{
			for(int i = 0; i < dpl; i++)
				line[i] = 255;
		}
		else
		{
			int cycle = black + white;
			for(int i = 0; i < dpl; i++)
			{
				if((i % cycle) < black)
					line[i] = 255;
				else
					line[i] = 0;
			}
		}
		printer_print(p, line, dpl);
	}
}

static void printer_callback(void * data, int x0, int y0, void * gray, int width, int height)
{
	struct {
		unsigned char * pixels;
		int w;
		int h;
	} * ctx = data;
	uint8_t * p, * q = &ctx->pixels[y0 * ctx->w + x0];
	uint8_t * pgray, * qgray = gray;
	int w = XMIN(width, ctx->w - x0);
	int h = XMIN(height, ctx->h - y0);
	int x, y;

	for(y = 0; y < h; y++, qgray += width, q += ctx->w)
	{
		for(x = 0, pgray = qgray, p = q; x < w; x++, pgray++, p++)
		{
			*p = *pgray;
		}
	}
}

void printer_print_text(struct printer_t * p, enum printer_align_t align, const char * family, enum font_style_t style, int size, const char * fmt, ...)
{
	if(p)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			int dpl = printer_get_dpl(p);
			int width, height;
			if(font_text_bound(family, style, size, dpl, str, &width, &height))
			{
				unsigned char * pixels = xos_mem_calloc(height, width);
				if(pixels)
				{
					font_text_render(family, style, size, 0, 0, dpl, str, printer_callback, (void *)&(struct{unsigned char * pixels;int w; int h;}){ pixels, width, height });
					unsigned char line[dpl];
					int ox = ((int)align) >> 2;
					switch(align & 0x3)
					{
					case PRINTER_ALIGN_LEFT:
						ox = 0;
						break;
					case PRINTER_ALIGN_CENTER:
						ox = (dpl - width) >> 1;
						break;
					case PRINTER_ALIGN_RIGHT:
						ox = dpl - width;
						break;
					default:
						break;
					}
					for(int y = 0; y < height; y++)
					{
						xos_memset(line, 0, dpl);
						unsigned char * pgray = &pixels[y * width];
						for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < width); i++, j++)
							line[i] = pgray[j];
						printer_print(p, line, dpl);
					}
					xos_mem_free(pixels);
				}
			}
			xos_mem_free(str);
		}
	}
}

void printer_print_qrcode(struct printer_t * p, enum printer_align_t align, int pixsz, const char * fmt, ...)
{
	if(p)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			uint8_t qrc[QRCGEN_BUFFER_LEN_MAX];
			uint8_t tmp[QRCGEN_BUFFER_LEN_MAX];
			if(qrcgen_encode_text(str, tmp, qrc, QRCGEN_ECC_MEDIUM, QRCGEN_VERSION_MIN, QRCGEN_VERSION_MAX, QRCGEN_MASK_AUTO, 1))
			{
				int qrs = qrcgen_get_size(qrc);
				if(qrs > 0)
				{
					int size = (qrs + 4) * ((pixsz > 0) ? pixsz : 1);
					unsigned char * pixels = xos_mem_malloc(size * size);
					if(pixels)
					{
						xos_memset(pixels, 0, size * size);
						for(int j = 0; j < qrs; j++)
						{
							for(int i = 0; i < qrs; i++)
							{
								if(qrcgen_get_pixel(qrc, i, j))
								{
									int x1 = (i + 2) * pixsz;
									int y1 = (j + 2) * pixsz;
									int x2 = x1 + pixsz;
									int y2 = y1 + pixsz;
									int x, y;
									unsigned char * t, * q = &pixels[y1 * size + x1];
									for(y = y1; y < y2; y++, q += size)
									{
										for(x = x1, t = q; x < x2; x++, t++)
											*t = 255;
									}
								}
							}
						}
						int dpl = printer_get_dpl(p);
						unsigned char line[dpl];
						int ox = ((int)align) >> 2;
						switch(align & 0x3)
						{
						case PRINTER_ALIGN_LEFT:
							ox = 0;
							break;
						case PRINTER_ALIGN_CENTER:
							ox = (dpl - size) >> 1;
							break;
						case PRINTER_ALIGN_RIGHT:
							ox = dpl - size;
							break;
						default:
							break;
						}
						for(int y = 0; y < size; y++)
						{
							xos_memset(line, 0, dpl);
							unsigned char * pgray = &pixels[y * size];
							for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < size); i++, j++)
								line[i] = pgray[j];
							printer_print(p, line, dpl);
						}
						xos_mem_free(pixels);
					}
				}
			}
			xos_mem_free(str);
		}
	}
}

void printer_print_vision(struct printer_t * p, enum printer_align_t align, struct vision_t * v)
{
	if(p && v)
	{
		struct vision_t * o = vision_clone(v, 0, 0, 0, 0);
		if(o)
		{
			if(o->type == VISION_TYPE_RGB)
				vision_convert(o, VISION_TYPE_GRAY);
			if(o->type == VISION_TYPE_GRAY)
			{
				unsigned char isbin = 1;
				unsigned char * q = (unsigned char *)o->datas;
				for(int i = 0; i < o->npixel; i++, q++)
				{
					if((*q > 0) && (*q < 255))
					{
						isbin = 0;
						break;
					}
				}
				if(!isbin)
					vision_dither(o, "atkinson");
				int dpl = printer_get_dpl(p);
				unsigned char line[dpl];
				int ox = ((int)align) >> 2;
				switch(align & 0x3)
				{
				case PRINTER_ALIGN_LEFT:
					ox = 0;
					break;
				case PRINTER_ALIGN_CENTER:
					ox = (dpl - o->width) >> 1;
					break;
				case PRINTER_ALIGN_RIGHT:
					ox = dpl - o->width;
					break;
				default:
					break;
				}
				for(int y = 0; y < o->height; y++)
				{
					xos_memset(line, 0, dpl);
					unsigned char * pgray = &((unsigned char *)o->datas)[y * o->width];
					for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < o->width); i++, j++)
						line[i] = 255 - pgray[j];
					printer_print(p, line, dpl);
				}
			}
			vision_free(o);
		}
	}
}

void printer_print_surface(struct printer_t * p, enum printer_align_t align, struct surface_t * s)
{
	if(p && s)
	{
		struct vision_t * o = vision_alloc(VISION_TYPE_GRAY, surface_get_width(s), surface_get_height(s));
		if(o)
		{
			vision_apply_surface(o, s);
			if(o->type == VISION_TYPE_GRAY)
			{
				unsigned char isbin = 1;
				unsigned char * q = (unsigned char *)o->datas;
				for(int i = 0; i < o->npixel; i++, q++)
				{
					if((*q > 0) && (*q < 255))
					{
						isbin = 0;
						break;
					}
				}
				if(!isbin)
					vision_dither(o, "atkinson");
				int dpl = printer_get_dpl(p);
				unsigned char line[dpl];
				int ox = ((int)align) >> 2;
				switch(align & 0x3)
				{
				case PRINTER_ALIGN_LEFT:
					ox = 0;
					break;
				case PRINTER_ALIGN_CENTER:
					ox = (dpl - o->width) >> 1;
					break;
				case PRINTER_ALIGN_RIGHT:
					ox = dpl - o->width;
					break;
				default:
					break;
				}
				for(int y = 0; y < o->height; y++)
				{
					xos_memset(line, 0, dpl);
					unsigned char * pgray = &((unsigned char *)o->datas)[y * o->width];
					for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < o->width); i++, j++)
						line[i] = 255 - pgray[j];
					printer_print(p, line, dpl);
				}
			}
			vision_free(o);
		}
	}
}

/*
 * printer async
 */
struct printer_async_ctx_t * printer_async_ctx_alloc(const char * name)
{
	struct printer_t * printer = name ? search_printer(name) : search_first_printer();

	if(printer)
	{
		struct thworker_t * worker = thworker_alloc(printer->name);
		if(worker)
		{
			struct printer_async_ctx_t * ctx = xos_mem_malloc(sizeof(struct printer_async_ctx_t));
			if(!ctx)
			{
				thworker_free(worker);
				return NULL;
			}
			ctx->printer = printer;
			ctx->worker = worker;
			return ctx;
		}
	}
	return NULL;
}

void printer_async_ctx_free(struct printer_async_ctx_t * ctx)
{
	if(ctx)
	{
		thworker_wait(ctx->worker);
		thworker_free(ctx->worker);
		xos_mem_free(ctx);
	}
}

static void clrcb(void (*func)(void *), void * data)
{
	if(data)
		xos_mem_free(data);
}

void printer_async_ctx_clear(struct printer_async_ctx_t * ctx)
{
	if(ctx)
	{
		thworker_clear(ctx->worker, clrcb);
		printer_standby(ctx->printer);
	}
}

struct printer_async_print_pdat_t {
	struct printer_t * printer;
	unsigned int len;
	unsigned char buf[];
};

static void printer_async_print_func(void * data)
{
	struct printer_async_print_pdat_t * pdat = (struct printer_async_print_pdat_t *)data;

	if(pdat)
	{
		printer_print(pdat->printer, pdat->buf, pdat->len);
		xos_mem_free(pdat);
	}
}

void printer_async_print(struct printer_async_ctx_t * ctx, unsigned char * buf, unsigned int len)
{
	if(ctx && buf && (len > 0))
	{
		struct printer_async_print_pdat_t * pdat = xos_mem_malloc(sizeof(struct printer_async_print_pdat_t) + len);
		if(pdat)
		{
			pdat->printer = ctx->printer;
			pdat->len = len;
			xos_memcpy(pdat->buf, buf, len);
			thworker_submit(ctx->worker, printer_async_print_func, pdat);
		}
	}
}

struct printer_async_feed_pdat_t {
	struct printer_t * printer;
	int lines;
};

static void printer_async_feed_func(void * data)
{
	struct printer_async_feed_pdat_t * pdat = (struct printer_async_feed_pdat_t *)data;

	if(pdat)
	{
		printer_feed(pdat->printer, pdat->lines);
		xos_mem_free(pdat);
	}
}

void printer_async_feed(struct printer_async_ctx_t * ctx, int lines)
{
	if(ctx)
	{
		struct printer_async_feed_pdat_t * pdat = xos_mem_malloc(sizeof(struct printer_async_feed_pdat_t));
		if(pdat)
		{
			pdat->printer = ctx->printer;
			pdat->lines = lines;
			thworker_submit(ctx->worker, printer_async_feed_func, pdat);
		}
	}
}

struct printer_async_cut_pdat_t {
	struct printer_t * printer;
};

static void printer_async_cut_func(void * data)
{
	struct printer_async_cut_pdat_t * pdat = (struct printer_async_cut_pdat_t *)data;

	if(pdat)
	{
		printer_cut(pdat->printer);
		xos_mem_free(pdat);
	}
}

void printer_async_cut(struct printer_async_ctx_t * ctx)
{
	if(ctx)
	{
		struct printer_async_cut_pdat_t * pdat = xos_mem_malloc(sizeof(struct printer_async_cut_pdat_t));
		if(pdat)
		{
			pdat->printer = ctx->printer;
			thworker_submit(ctx->worker, printer_async_cut_func, pdat);
		}
	}
}

struct printer_async_standby_pdat_t {
	struct printer_t * printer;
};

static void printer_async_standby_func(void * data)
{
	struct printer_async_standby_pdat_t * pdat = (struct printer_async_standby_pdat_t *)data;

	if(pdat)
	{
		printer_standby(pdat->printer);
		xos_mem_free(pdat);
	}
}

void printer_async_standby(struct printer_async_ctx_t * ctx)
{
	if(ctx)
	{
		struct printer_async_standby_pdat_t * pdat = xos_mem_malloc(sizeof(struct printer_async_standby_pdat_t));
		if(pdat)
		{
			pdat->printer = ctx->printer;
			thworker_submit(ctx->worker, printer_async_standby_func, pdat);
		}
	}
}

void printer_async_print_hr(struct printer_async_ctx_t * ctx, int black, int white)
{
	if(ctx)
	{
		int dpl = printer_get_dpl(ctx->printer);
		unsigned char line[dpl];
		if(black <= 0)
		{
			for(int i = 0; i < dpl; i++)
				line[i] = 0;
		}
		else if(white <= 0)
		{
			for(int i = 0; i < dpl; i++)
				line[i] = 255;
		}
		else
		{
			int cycle = black + white;
			for(int i = 0; i < dpl; i++)
			{
				if((i % cycle) < black)
					line[i] = 255;
				else
					line[i] = 0;
			}
		}
		printer_async_print(ctx, line, dpl);
	}
}

void printer_async_print_text(struct printer_async_ctx_t * ctx, enum printer_align_t align, const char * family, enum font_style_t style, int size, const char * fmt, ...)
{
	if(ctx)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			int dpl = printer_get_dpl(ctx->printer);
			int width, height;
			if(font_text_bound(family, style, size, dpl, str, &width, &height))
			{
				unsigned char * pixels = xos_mem_calloc(height, width);
				if(pixels)
				{
					font_text_render(family, style, size, 0, 0, dpl, str, printer_callback, (void *)&(struct{unsigned char * pixels;int w; int h;}){ pixels, width, height });
					unsigned char line[dpl];
					int ox = ((int)align) >> 2;
					switch(align & 0x3)
					{
					case PRINTER_ALIGN_LEFT:
						ox = 0;
						break;
					case PRINTER_ALIGN_CENTER:
						ox = (dpl - width) >> 1;
						break;
					case PRINTER_ALIGN_RIGHT:
						ox = dpl - width;
						break;
					default:
						break;
					}
					for(int y = 0; y < height; y++)
					{
						xos_memset(line, 0, dpl);
						unsigned char * pgray = &pixels[y * width];
						for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < width); i++, j++)
							line[i] = pgray[j];
						printer_async_print(ctx, line, dpl);
					}
					xos_mem_free(pixels);
				}
			}
			xos_mem_free(str);
		}
	}
}

void printer_async_print_qrcode(struct printer_async_ctx_t * ctx, enum printer_align_t align, int pixsz, const char * fmt, ...)
{
	if(ctx)
	{
		va_list ap;
		char * str = NULL;
		va_start(ap, fmt);
		int len = xos_vasprintf(&str, fmt, ap);
		va_end(ap);
		if(str && (len > 0))
		{
			uint8_t qrc[QRCGEN_BUFFER_LEN_MAX];
			uint8_t tmp[QRCGEN_BUFFER_LEN_MAX];
			if(qrcgen_encode_text(str, tmp, qrc, QRCGEN_ECC_MEDIUM, QRCGEN_VERSION_MIN, QRCGEN_VERSION_MAX, QRCGEN_MASK_AUTO, 1))
			{
				int qrs = qrcgen_get_size(qrc);
				if(qrs > 0)
				{
					int size = (qrs + 4) * ((pixsz > 0) ? pixsz : 1);
					unsigned char * pixels = xos_mem_malloc(size * size);
					if(pixels)
					{
						xos_memset(pixels, 0, size * size);
						for(int j = 0; j < qrs; j++)
						{
							for(int i = 0; i < qrs; i++)
							{
								if(qrcgen_get_pixel(qrc, i, j))
								{
									int x1 = (i + 2) * pixsz;
									int y1 = (j + 2) * pixsz;
									int x2 = x1 + pixsz;
									int y2 = y1 + pixsz;
									int x, y;
									unsigned char * t, * q = &pixels[y1 * size + x1];
									for(y = y1; y < y2; y++, q += size)
									{
										for(x = x1, t = q; x < x2; x++, t++)
											*t = 255;
									}
								}
							}
						}
						int dpl = printer_get_dpl(ctx->printer);
						unsigned char line[dpl];
						int ox = ((int)align) >> 2;
						switch(align & 0x3)
						{
						case PRINTER_ALIGN_LEFT:
							ox = 0;
							break;
						case PRINTER_ALIGN_CENTER:
							ox = (dpl - size) >> 1;
							break;
						case PRINTER_ALIGN_RIGHT:
							ox = dpl - size;
							break;
						default:
							break;
						}
						for(int y = 0; y < size; y++)
						{
							xos_memset(line, 0, dpl);
							unsigned char * pgray = &pixels[y * size];
							for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < size); i++, j++)
								line[i] = pgray[j];
							printer_async_print(ctx, line, dpl);
						}
						xos_mem_free(pixels);
					}
				}
			}
			xos_mem_free(str);
		}
	}
}

void printer_async_print_vision(struct printer_async_ctx_t * ctx, enum printer_align_t align, struct vision_t * v)
{
	if(ctx && v)
	{
		struct vision_t * o = vision_clone(v, 0, 0, 0, 0);
		if(o)
		{
			if(o->type == VISION_TYPE_RGB)
				vision_convert(o, VISION_TYPE_GRAY);
			if(o->type == VISION_TYPE_GRAY)
			{
				unsigned char isbin = 1;
				unsigned char * q = (unsigned char *)o->datas;
				for(int i = 0; i < o->npixel; i++, q++)
				{
					if((*q > 0) && (*q < 255))
					{
						isbin = 0;
						break;
					}
				}
				if(!isbin)
					vision_dither(o, "atkinson");
				int dpl = printer_get_dpl(ctx->printer);
				unsigned char line[dpl];
				int ox = ((int)align) >> 2;
				switch(align & 0x3)
				{
				case PRINTER_ALIGN_LEFT:
					ox = 0;
					break;
				case PRINTER_ALIGN_CENTER:
					ox = (dpl - o->width) >> 1;
					break;
				case PRINTER_ALIGN_RIGHT:
					ox = dpl - o->width;
					break;
				default:
					break;
				}
				for(int y = 0; y < o->height; y++)
				{
					xos_memset(line, 0, dpl);
					unsigned char * pgray = &((unsigned char *)o->datas)[y * o->width];
					for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < o->width); i++, j++)
						line[i] = 255 - pgray[j];
					printer_async_print(ctx, line, dpl);
				}
			}
			vision_free(o);
		}
	}
}

void printer_async_print_surface(struct printer_async_ctx_t * ctx, enum printer_align_t align, struct surface_t * s)
{
	if(ctx && s)
	{
		struct vision_t * o = vision_alloc(VISION_TYPE_GRAY, surface_get_width(s), surface_get_height(s));
		if(o)
		{
			vision_apply_surface(o, s);
			if(o->type == VISION_TYPE_GRAY)
			{
				unsigned char isbin = 1;
				unsigned char * q = (unsigned char *)o->datas;
				for(int i = 0; i < o->npixel; i++, q++)
				{
					if((*q > 0) && (*q < 255))
					{
						isbin = 0;
						break;
					}
				}
				if(!isbin)
					vision_dither(o, "atkinson");
				int dpl = printer_get_dpl(ctx->printer);
				unsigned char line[dpl];
				int ox = ((int)align) >> 2;
				switch(align & 0x3)
				{
				case PRINTER_ALIGN_LEFT:
					ox = 0;
					break;
				case PRINTER_ALIGN_CENTER:
					ox = (dpl - o->width) >> 1;
					break;
				case PRINTER_ALIGN_RIGHT:
					ox = dpl - o->width;
					break;
				default:
					break;
				}
				for(int y = 0; y < o->height; y++)
				{
					xos_memset(line, 0, dpl);
					unsigned char * pgray = &((unsigned char *)o->datas)[y * o->width];
					for(int i = (ox < 0) ? 0 : ox, j = (ox < 0) ? -ox : 0; (i < dpl) && (j < o->width); i++, j++)
						line[i] = 255 - pgray[j];
					printer_async_print(ctx, line, dpl);
				}
			}
			vision_free(o);
		}
	}
}
