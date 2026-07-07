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

#include <kernel/vision/vision.h>

struct vision_t * vision_alloc(enum vision_type_t type, int width, int height)
{
	struct vision_t * v;
	void * datas;
	size_t ndata;
	int npixel;

	if((width <= 0) || (height <= 0))
		return NULL;

	v = xos_mem_malloc(sizeof(struct vision_t));
	if(!v)
		return NULL;

	npixel = width * height;
	ndata = npixel * vision_type_get_bytes(type) * vision_type_get_channels(type);
	datas = xos_mem_malloc(ndata);
	if(!datas)
	{
		xos_mem_free(v);
		return NULL;
	}

	v->type = type;
	v->width = width;
	v->height = height;
	v->npixel = npixel;
	v->datas = datas;
	v->ndata = ndata;
	return v;
}

void vision_free(struct vision_t * v)
{
	if(v)
	{
		if(v->datas)
			xos_mem_free(v->datas);
		xos_mem_free(v);
	}
}

struct vision_t * vision_clone(struct vision_t * v, int x, int y, int w, int h)
{
	if(v)
	{
		if((w <= 0) || (h <= 0))
		{
			struct vision_t * o = vision_alloc(v->type, v->width, v->height);
			if(o)
			{
				xos_memcpy(o->datas, v->datas, o->ndata);
				return o;
			}
		}
		else
		{
			int x1, y1, x2, y2;
			x1 = XMAX(0, x);
			x2 = XMIN(v->width, x + w);
			if(x1 <= x2)
			{
				y1 = XMAX(0, y);
				y2 = XMIN(v->height, y + h);
				if(y1 <= y2)
				{
					int width = x2 - x1;
					int height = y2 - y1;
					struct vision_t * o = vision_alloc(v->type, width, height);
					if(o)
					{
						switch(o->type)
						{
						case VISION_TYPE_GRAY:
							{
								int ostride = o->width;
								int vstride = v->width;
								int len = ostride * vision_type_get_bytes(VISION_TYPE_GRAY);
								unsigned char * po = (unsigned char *)o->datas;
								unsigned char * pv = &((unsigned char *)v->datas)[y1 * v->width + x1];
								for(int i = 0; i < height; i++, po += ostride, pv += vstride)
									xos_memcpy(po, pv, len);
							}
							break;
						case VISION_TYPE_RGB:
							{
								int ostride = o->width;
								int vstride = v->width;
								int len = ostride * vision_type_get_bytes(VISION_TYPE_RGB);
								unsigned char * po, * pv;

								po = &((unsigned char *)o->datas)[o->npixel * 0];
								pv = &((unsigned char *)v->datas)[v->npixel * 0 + y1 * v->width + x1];
								for(int i = 0; i < height; i++, po += ostride, pv += vstride)
									xos_memcpy(po, pv, len);

								po = &((unsigned char *)o->datas)[o->npixel * 1];
								pv = &((unsigned char *)v->datas)[v->npixel * 1 + y1 * v->width + x1];
								for(int i = 0; i < height; i++, po += ostride, pv += vstride)
									xos_memcpy(po, pv, len);

								po = &((unsigned char *)o->datas)[o->npixel * 2];
								pv = &((unsigned char *)v->datas)[v->npixel * 2 + y1 * v->width + x1];
								for(int i = 0; i < height; i++, po += ostride, pv += vstride)
									xos_memcpy(po, pv, len);
							}
							break;
						default:
							break;
						}
						return o;
					}
				}
			}
		}
	}
	return NULL;
}

void vision_convert(struct vision_t * v, enum vision_type_t type)
{
	if(v && (v->type != type))
	{
		switch(v->type)
		{
		case VISION_TYPE_GRAY:
			switch(type)
			{
			case VISION_TYPE_RGB:
				{
					size_t ndata = v->npixel * vision_type_get_bytes(VISION_TYPE_RGB) * vision_type_get_channels(VISION_TYPE_RGB);
					void * datas = xos_mem_malloc(ndata);
					if(datas)
					{
						unsigned char * pr = &((unsigned char *)datas)[v->npixel * 0];
						unsigned char * pg = &((unsigned char *)datas)[v->npixel * 1];
						unsigned char * pb = &((unsigned char *)datas)[v->npixel * 2];
						unsigned char * pgray = (unsigned char *)v->datas;
						for(int i = 0; i < v->npixel; i++, pr++, pg++, pb++, pgray++)
							*pb = *pg = *pr = *pgray;
						if(v->datas)
							xos_mem_free(v->datas);
						v->datas = datas;
						v->ndata = ndata;
						v->type = VISION_TYPE_RGB;
					}
				}
				break;
			default:
				break;
			}
			break;

		case VISION_TYPE_RGB:
			switch(type)
			{
			case VISION_TYPE_GRAY:
				{
					unsigned char * pr = &((unsigned char *)v->datas)[v->npixel * 0];
					unsigned char * pg = &((unsigned char *)v->datas)[v->npixel * 1];
					unsigned char * pb = &((unsigned char *)v->datas)[v->npixel * 2];
					for(int i = 0; i < v->npixel; i++, pr++, pg++, pb++)
						*pr = ((*pr) * 13933L + (*pg) * 46871L + (*pb) * 4732L) >> 16;
					v->type = VISION_TYPE_GRAY;
				}
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}
}

void vision_clear(struct vision_t * v)
{
	if(v)
		xos_memset(v->datas, 0, v->npixel * vision_type_get_bytes(v->type) * vision_type_get_channels(v->type));
}

void vision_apply_surface(struct vision_t * v, struct surface_t * s)
{
	if(v && s)
	{
		int w = XMIN(vision_get_width(v), surface_get_width(s));
		int h = XMIN(vision_get_height(v), surface_get_height(s));
		switch(v->type)
		{
		case VISION_TYPE_GRAY:
			{
				unsigned char * pgray, * qgray = &((unsigned char *)v->datas)[v->npixel * 0];
				unsigned char * p, * q = surface_get_pixels(s);
				int vstride = v->width;
				int sstride = s->stride;
				int x, y;
				for(y = 0; y < h; y++, qgray += vstride, q += sstride)
				{
					for(x = 0, pgray = qgray, p = q; x < w; x++, pgray++, p += 4)
					{
						if(p[3] != 0)
						{
							if(p[3] == 255)
								*pgray = (p[2] * 13933L + p[1] * 46871L + p[0] * 4732L) >> 16;
							else
								*pgray = ((p[2] * 13933L + p[1] * 46871L + p[0] * 4732L) >> 16) * 255 / p[3];
						}
						else
							*pgray = 0;
					}
				}
			}
			break;
		case VISION_TYPE_RGB:
			{
				unsigned char * pr, * qr = &((unsigned char *)v->datas)[v->npixel * 0];
				unsigned char * pg, * qg = &((unsigned char *)v->datas)[v->npixel * 1];
				unsigned char * pb, * qb = &((unsigned char *)v->datas)[v->npixel * 2];
				unsigned char * p, * q = surface_get_pixels(s);
				int vstride = v->width;
				int sstride = s->stride;
				int x, y;
				for(y = 0; y < h; y++, qr += vstride, qg += vstride, qb += vstride, q += sstride)
				{
					for(x = 0, pr = qr, pg = qg, pb = qb, p = q; x < w; x++, pr++, pg++, pb++, p += 4)
					{
						if(p[3] != 0)
						{
							if(p[3] == 255)
							{
								*pr = p[2];
								*pg = p[1];
								*pb = p[0];
							}
							else
							{
								*pr = p[2] * 255 / p[3];
								*pg = p[1] * 255 / p[3];
								*pb = p[0] * 255 / p[3];
							}
						}
						else
						{
							*pr = 0;
							*pg = 0;
							*pb = 0;
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

void surface_apply_vision(struct surface_t * s, struct vision_t * v)
{
	if(s && v)
	{
		int w = XMIN(surface_get_width(s), vision_get_width(v));
		int h = XMIN(surface_get_height(s), vision_get_height(v));
		switch(v->type)
		{
		case VISION_TYPE_GRAY:
			{
				unsigned char * pgray, * qgray = &((unsigned char *)v->datas)[v->npixel * 0];
				unsigned char * p, * q = surface_get_pixels(s);
				int vstride = v->width;
				int sstride = s->stride;
				int x, y;
				for(y = 0; y < h; y++, qgray += vstride, q += sstride)
				{
					for(x = 0, pgray = qgray, p = q; x < w; x++, pgray++, p += 4)
					{
						p[3] = 255;
						p[2] = p[1] = p[0] = *pgray;
					}
				}
			}
			break;
		case VISION_TYPE_RGB:
			{
				unsigned char * pr, * qr = &((unsigned char *)v->datas)[v->npixel * 0];
				unsigned char * pg, * qg = &((unsigned char *)v->datas)[v->npixel * 1];
				unsigned char * pb, * qb = &((unsigned char *)v->datas)[v->npixel * 2];
				unsigned char * p, * q = surface_get_pixels(s);
				int vstride = v->width;
				int sstride = s->stride;
				int x, y;
				for(y = 0; y < h; y++, qr += vstride, qg += vstride, qb += vstride, q += sstride)
				{
					for(x = 0, pr = qr, pg = qg, pb = qb, p = q; x < w; x++, pr++, pg++, pb++, p += 4)
					{
						p[3] = 255;
						p[2] = *pr;
						p[1] = *pg;
						p[0] = *pb;
					}
				}
			}
			break;
		default:
			break;
		}
	}
}
