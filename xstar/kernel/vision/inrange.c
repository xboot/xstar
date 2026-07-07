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

#include <kernel/vision/inrange.h>

struct vision_t * vision_inrange(struct vision_t * v, float * lower, float * upper)
{
	if(v && (v->type == VISION_TYPE_RGB))
	{
		struct vision_t * mask = vision_alloc(VISION_TYPE_GRAY, vision_get_width(v), vision_get_height(v));
		if(mask)
		{
			unsigned char * pmask = (unsigned char *)mask->datas;
			unsigned char * pr = &((unsigned char *)v->datas)[v->npixel * 0];
			unsigned char * pg = &((unsigned char *)v->datas)[v->npixel * 1];
			unsigned char * pb = &((unsigned char *)v->datas)[v->npixel * 2];
			float th, ts, tv;
			for(int i = 0; i < v->npixel; i++, pr++, pg++, pb++)
			{
				color_get_hsva(&(struct color_t){ *pr, *pg, *pb, 255 }, &th, &ts, &tv, NULL);
				if((th < lower[0]) || (th > upper[0]) || (ts < lower[1]) || (ts > upper[1]) || (tv < lower[2]) || (tv > upper[2]))
					*pmask = 0;
				else
					*pmask = 255;
			}
			return mask;
		}
	}
	return NULL;
}
