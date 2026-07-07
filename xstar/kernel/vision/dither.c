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

#include <kernel/vision/dither.h>

void vision_dither(struct vision_t * v, const char * type)
{
	if(v && (v->type == VISION_TYPE_GRAY))
	{
		int width = vision_get_width(v);
		int height = vision_get_height(v);
		unsigned char * p;
		switch(shash(type))
		{
		case 0xf43bc797: /* "jarvis-judice-ninke" */
			p = vision_get_datas(v);
			for(int y = 0; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					int o = p[0];
					int n = (o > 127) ? 255 : 0;
					int e = o - n;
					p[0] = (unsigned char)n;
					if(x + 1 < width)
						p[1] = XCLAMP(p[1] + ((e * 7) / 48), 0, 255);
					if(x + 2 < width)
						p[2] = XCLAMP(p[2] + ((e * 5) / 48), 0, 255);
					if(y + 1 < height)
					{
						if(x > 1)
							p[width - 2] = XCLAMP(p[width - 2] + ((e * 3) / 48), 0, 255);
						if(x > 0)
							p[width - 1] = XCLAMP(p[width - 1] + ((e * 5) / 48), 0, 255);
						p[width] = XCLAMP(p[width] + ((e * 7) / 48), 0, 255);
						if(x + 1 < width)
							p[width + 1] = XCLAMP(p[width + 1] + ((e * 5) / 48), 0, 255);
						if(x + 2 < width)
							p[width + 2] = XCLAMP(p[width + 2] + ((e * 3) / 48), 0, 255);
					}
					if(y + 2 < height)
					{
						if(x > 1)
							p[2 * width - 2] = XCLAMP(p[2 * width - 2] + (e / 48), 0, 255);
						if(x > 0)
							p[2 * width - 1] = XCLAMP(p[2 * width - 1] + ((e * 3) / 48), 0, 255);
						p[2 * width] = XCLAMP(p[2 * width] + ((e * 5) / 48), 0, 255);
						if(x + 1 < width)
							p[2 * width + 1] = XCLAMP(p[2 * width + 1] + ((e * 3) / 48), 0, 255);
						if(x + 2 < width)
							p[2 * width + 2] = XCLAMP(p[2 * width + 2] + (e / 48), 0, 255);
					}
					p++;
				}
			}
			break;
		case 0x1c953b38: /* "stucki" */
			p = vision_get_datas(v);
			for(int y = 0; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					int o = p[0];
					int n = (o > 127) ? 255 : 0;
					int e = o - n;
					p[0] = (unsigned char)n;
					if(x + 1 < width)
						p[1] = XCLAMP(p[1] + ((e * 8) / 42), 0, 255);
					if(x + 2 < width)
						p[2] = XCLAMP(p[2] + ((e * 4) / 42), 0, 255);
					if(y + 1 < height)
					{
						if(x > 1)
							p[width - 2] = XCLAMP(p[width - 2] + ((e * 2) / 42), 0, 255);
						if(x > 0)
							p[width - 1] = XCLAMP(p[width - 1] + ((e * 4) / 42), 0, 255);
						p[width] = XCLAMP(p[width] + ((e * 8) / 42), 0, 255);
						if(x + 1 < width)
							p[width + 1] = XCLAMP(p[width + 1] + ((e * 4) / 42), 0, 255);
						if(x + 2 < width)
							p[width + 2] = XCLAMP(p[width + 2] + ((e * 2) / 42), 0, 255);
					}
					if(y + 2 < height)
					{
						if(x > 1)
							p[2 * width - 2] = XCLAMP(p[2 * width - 2] + (e / 42), 0, 255);
						if(x > 0)
							p[2 * width - 1] = XCLAMP(p[2 * width - 1] + ((e * 2) / 42), 0, 255);
						p[2 * width] = XCLAMP(p[2 * width] + ((e * 4) / 42), 0, 255);
						if(x + 1 < width)
							p[2 * width + 1] = XCLAMP(p[2 * width + 1] + ((e * 2) / 42), 0, 255);
						if(x + 2 < width)
							p[2 * width + 2] = XCLAMP(p[2 * width + 2] + (e / 42), 0, 255);
					}
					p++;
				}
			}
			break;
		case 0xbf62ac13: /* "floyd-steinberg" */
			p = vision_get_datas(v);
			for(int y = 0; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					int o = p[0];
					int n = o > 127 ? 255 : 0;
					int e = o - n;
					p[0] = (unsigned char)n;
					if(x < width - 1)
						p[1] = XCLAMP(p[1] + ((e * 7) / 16), 0, 255);
					if(y < height - 1)
					{
						if(x > 0)
							p[width - 1] = XCLAMP(p[width - 1] + ((e * 3) / 16), 0, 255);
						p[width] = XCLAMP(p[width] + ((e * 5) / 16), 0, 255);
						if(x < width - 1)
							p[width + 1] = XCLAMP(p[width + 1] + (e / 16), 0, 255);
					}
					p++;
				}
			}
			break;
		case 0xe34acc8c: /* "atkinson" */
			p = vision_get_datas(v);
			for(int y = 0; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					int o = p[0];
					int n = (o > 127) ? 255 : 0;
					int e = (o - n) / 8;
					p[0] = (unsigned char)n;
					if(x + 1 < width)
						p[1] = XCLAMP(p[1] + e, 0, 255);
					if(x + 2 < width)
						p[2] = XCLAMP(p[2] + e, 0, 255);
					if(y + 1 < height)
					{
						if(x - 1 >= 0)
							p[width - 1] = XCLAMP(p[width - 1] + e, 0, 255);
						p[width] = XCLAMP(p[width] + e, 0, 255);
						if(x + 1 < width)
							p[width + 1] = XCLAMP(p[width + 1] + e, 0, 255);
					}
					if(y + 2 < height)
						p[width + 2] = XCLAMP(p[width + 2] + e, 0, 255);
					p++;
				}
			}
			break;
		default:
			break;
		}
	}
}

void vision_dither_palette(struct vision_t * v, struct color_t * palette, int n)
{
	if(v)
	{
		switch(v->type)
		{
		case VISION_TYPE_GRAY:
			{
				if(!palette || (n <= 0))
				{
					static const struct color_t bw_palette[] = {
						{ 0x00, 0x00, 0x00, 0xff },
						{ 0xff, 0xff, 0xff, 0xff },
					};
					palette = (struct color_t *)bw_palette;
					n = ARRAY_SIZE(bw_palette);
				}
				int width = vision_get_width(v);
				int height = vision_get_height(v);
				unsigned char * p = vision_get_datas(v);
				for(int y = 0; y < height; y++)
				{
					for(int x = 0; x < width; x++)
					{
						int idx = y * width + x;
						int o = p[idx];
						int best = 0;
						int minimal = 255 * 255 + 1;
						for(int i = 0; i < n; i++)
						{
							int g = palette[i].r;
							int d = (o - g) * (o - g);
							if(d < minimal)
							{
								minimal = d;
								best = i;
							}
						}
						int nv = palette[best].r;
						int e = o - nv;
						p[idx] = (unsigned char)nv;
						if(x + 1 < width)
							p[idx + 1] = XCLAMP(p[idx + 1] + ((e * 7) / 16), 0, 255);
						if(y + 1 < height)
						{
							if(x > 0)
								p[idx + width - 1] = XCLAMP(p[idx + width - 1] + ((e * 3) / 16), 0, 255);
							p[idx + width] = XCLAMP(p[idx + width] + ((e * 5) / 16), 0, 255);
							if(x + 1 < width)
								p[idx + width + 1] = XCLAMP(p[idx + width + 1] + (e / 16), 0, 255);
						}
					}
				}
			}
			break;
		case VISION_TYPE_RGB:
			{
				if(!palette || (n <= 0))
				{
					static const struct color_t standard_palette[] = {
						{ 0xff, 0x00, 0x00, 0xff },
						{ 0x00, 0xff, 0x00, 0xff },
						{ 0x00, 0x00, 0xff, 0xff },
						{ 0x00, 0xff, 0xff, 0xff },
						{ 0xff, 0x00, 0xff, 0xff },
						{ 0xff, 0xff, 0x00, 0xff },
						{ 0x00, 0x00, 0x00, 0xff },
						{ 0xff, 0xff, 0xff, 0xff },
					};
					palette = (struct color_t *)standard_palette;
					n = ARRAY_SIZE(standard_palette);
				}
				int width = vision_get_width(v);
				int height = vision_get_height(v);
				int npixel = vision_get_npixel(v);
				unsigned char * p = vision_get_datas(v);
				unsigned char * pr = &p[npixel * 0];
				unsigned char * pg = &p[npixel * 1];
				unsigned char * pb = &p[npixel * 2];
				for(int y = 0; y < height; y++)
				{
					for(int x = 0; x < width; x++)
					{
						int idx = y * width + x;
						int or = pr[idx];
						int og = pg[idx];
						int ob = pb[idx];
						int best = 0;
						int minimal = 255 * 255 * 3 + 1;
						for(int i = 0; i < n; i++)
						{
							int dr = or - (int)palette[i].r;
							int dg = og - (int)palette[i].g;
							int db = ob - (int)palette[i].b;
							int d = dr * dr + dg * dg + db * db;
							if(d < minimal)
							{
								minimal = d;
								best = i;
							}
						}
						int nr = palette[best].r;
						int ng = palette[best].g;
						int nb = palette[best].b;
						int er = or - nr;
						int eg = og - ng;
						int eb = ob - nb;
						pr[idx] = (unsigned char)nr;
						pg[idx] = (unsigned char)ng;
						pb[idx] = (unsigned char)nb;
						if(x + 1 < width)
						{
							int q = idx + 1;
							pr[q] = XCLAMP(pr[q] + ((er * 7) / 16), 0, 255);
							pg[q] = XCLAMP(pg[q] + ((eg * 7) / 16), 0, 255);
							pb[q] = XCLAMP(pb[q] + ((eb * 7) / 16), 0, 255);
						}
						if(y + 1 < height)
						{
							if(x > 0)
							{
								int q = idx + width - 1;
								pr[q] = XCLAMP(pr[q] + ((er * 3) / 16), 0, 255);
								pg[q] = XCLAMP(pg[q] + ((eg * 3) / 16), 0, 255);
								pb[q] = XCLAMP(pb[q] + ((eb * 3) / 16), 0, 255);
							}
							{
								int q = idx + width;
								pr[q] = XCLAMP(pr[q] + ((er * 5) / 16), 0, 255);
								pg[q] = XCLAMP(pg[q] + ((eg * 5) / 16), 0, 255);
								pb[q] = XCLAMP(pb[q] + ((eb * 5) / 16), 0, 255);
							}
							if(x + 1 < width)
							{
								int q = idx + width + 1;
								pr[q] = XCLAMP(pr[q] + (er / 16), 0, 255);
								pg[q] = XCLAMP(pg[q] + (eg / 16), 0, 255);
								pb[q] = XCLAMP(pb[q] + (eb / 16), 0, 255);
							}
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
