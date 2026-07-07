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

#include <kernel/vision/gray.h>

void vision_gray(struct vision_t * v)
{
	if(v && (v->type == VISION_TYPE_RGB))
	{
		unsigned char * pr = &((unsigned char *)v->datas)[v->npixel * 0];
		unsigned char * pg = &((unsigned char *)v->datas)[v->npixel * 1];
		unsigned char * pb = &((unsigned char *)v->datas)[v->npixel * 2];
		for(int i = 0; i < v->npixel; i++, pr++, pg++, pb++)
		{
			unsigned char gray = ((*pr) * 13933L + (*pg) * 46871L + (*pb) * 4732L) >> 16;
			*pr = gray;
			*pg = gray;
			*pb = gray;
		}
	}
}
