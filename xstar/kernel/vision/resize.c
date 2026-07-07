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

#include <kernel/vision/resize.h>

struct vision_t * vision_resize(struct vision_t * v, int width, int height)
{
	if(v)
	{
		if((width > 0) || (height > 0))
		{
			if(width <= 0)
				width = vision_get_width(v) * height / vision_get_height(v);
			else if(height <= 0)
				height = vision_get_height(v) * width / vision_get_width(v);
			struct vision_t * o = vision_alloc(v->type, width, height);
			if(o)
			{
				switch(v->type)
				{
				case VISION_TYPE_GRAY:
					{
						unsigned char * pdst = (unsigned char *)o->datas;
						unsigned char * psrc = (unsigned char *)v->datas;
						float x_ratio = (float)(v->width - 1) / (float)(o->width - 1);
						float y_ratio = (float)(v->height - 1) / (float)(o->height - 1);
						for(int y = 0; y < o->height; y++)
						{
							for(int x = 0; x < o->width; x++)
							{
								float src_x = x * x_ratio;
								float src_y = y * y_ratio;
								int x0 = (int)src_x;
								int y0 = (int)src_y;
								int x1 = XMIN(x0 + 1, v->width - 1);
								int y1 = XMIN(y0 + 1, v->height - 1);
								float u = src_x - x0;
								float v_ratio = src_y - y0;
								unsigned int p00 = psrc[y0 * v->width + x0];
								unsigned int p10 = psrc[y0 * v->width + x1];
								unsigned int p01 = psrc[y1 * v->width + x0];
								unsigned int p11 = psrc[y1 * v->width + x1];
								float top = (1.0f - u) * p00 + u * p10;
								float bottom = (1.0f - u) * p01 + u * p11;
								float val = (1.0f - v_ratio) * top + v_ratio * bottom;
								*pdst = (unsigned char)(val + 0.5f);
								pdst++;
							}
						}
					}
					break;
				case VISION_TYPE_RGB:
					{
						unsigned char * prdst = &((unsigned char *)o->datas)[o->npixel * 0];
						unsigned char * pgdst = &((unsigned char *)o->datas)[o->npixel * 1];
						unsigned char * pbdst = &((unsigned char *)o->datas)[o->npixel * 2];
						unsigned char * prsrc = &((unsigned char *)v->datas)[v->npixel * 0];
						unsigned char * pgsrc = &((unsigned char *)v->datas)[v->npixel * 1];
						unsigned char * pbsrc = &((unsigned char *)v->datas)[v->npixel * 2];
						float x_ratio = (float)(v->width - 1) / (float)(o->width - 1);
						float y_ratio = (float)(v->height - 1) / (float)(o->height - 1);
						for(int y = 0; y < o->height; y++)
						{
							for(int x = 0; x < o->width; x++)
							{
								float src_x = x * x_ratio;
								float src_y = y * y_ratio;
								int x0 = (int)src_x;
								int y0 = (int)src_y;
								int x1 = XMIN(x0 + 1, v->width - 1);
								int y1 = XMIN(y0 + 1, v->height - 1);
								float u = src_x - x0;
								float v_ratio = src_y - y0;
								unsigned int pr00 = prsrc[y0 * v->width + x0];
								unsigned int pr10 = prsrc[y0 * v->width + x1];
								unsigned int pr01 = prsrc[y1 * v->width + x0];
								unsigned int pr11 = prsrc[y1 * v->width + x1];
								unsigned int pg00 = pgsrc[y0 * v->width + x0];
								unsigned int pg10 = pgsrc[y0 * v->width + x1];
								unsigned int pg01 = pgsrc[y1 * v->width + x0];
								unsigned int pg11 = pgsrc[y1 * v->width + x1];
								unsigned int pb00 = pbsrc[y0 * v->width + x0];
								unsigned int pb10 = pbsrc[y0 * v->width + x1];
								unsigned int pb01 = pbsrc[y1 * v->width + x0];
								unsigned int pb11 = pbsrc[y1 * v->width + x1];
								float rtop = (1.0f - u) * pr00 + u * pr10;
								float rbottom = (1.0f - u) * pr01 + u * pr11;
								float rval = (1.0f - v_ratio) * rtop + v_ratio * rbottom;
								float gtop = (1.0f - u) * pg00 + u * pg10;
								float gbottom = (1.0f - u) * pg01 + u * pg11;
								float gval = (1.0f - v_ratio) * gtop + v_ratio * gbottom;
								float btop = (1.0f - u) * pb00 + u * pb10;
								float bbottom = (1.0f - u) * pb01 + u * pb11;
								float bval = (1.0f - v_ratio) * btop + v_ratio * bbottom;
								*prdst = (unsigned char)(rval + 0.5f);
								*pgdst = (unsigned char)(gval + 0.5f);
								*pbdst = (unsigned char)(bval + 0.5f);
								prdst++;
								pgdst++;
								pbdst++;
							}
						}
					}
					break;
				default:
					break;
				}
				return o;
			}
		}
	}
	return NULL;
}
