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

#include <xos/xos.h>
#include <libx/yuv.h>

/*
 * ITU-R BT.601
 */
void bt601_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b)
{
	int8_t tu = u - 128;
	int8_t tv = v - 128;
	int16_t tb = (129 * tu) >> 6;
	int16_t tr = (102 * tv) >> 6;
	int16_t tg = (50 * tu + 104 * tv) >> 7;
	int16_t ty = (149 * (y - 16)) >> 7;
	*r = XCLAMP(ty + tr, 0, 255);
	*g = XCLAMP(ty - tg, 0, 255);
	*b = XCLAMP(ty + tb, 0, 255);
}

void bt601_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v)
{
	uint8_t ty = (77 * r + 150 * g + 29 * b) >> 8;
	int16_t tu = b - ty;
	int16_t tv = r - ty;
	*y = ((ty * 110) >> 7) + 16;
	*u = ((tu * 127) >> 8) + 128;
	*v = ((tv * 160) >> 8) + 128;
}

/*
 * ITU-R BT.709
 */
void bt709_yuv2rgb(uint8_t y, uint8_t u, uint8_t v, uint8_t * r, uint8_t * g, uint8_t * b)
{
	int8_t tu = u - 128;
	int8_t tv = v - 128;
	int16_t tb = (135 * tu) >> 6;
	int16_t tr = (115 * tv) >> 6;
	int16_t tg = (27 * tu + 68 * tv) >> 7;
	int16_t ty = (149 * (y - 16)) >> 7;
	*r = XCLAMP(ty + tr, 0, 255);
	*g = XCLAMP(ty - tg, 0, 255);
	*b = XCLAMP(ty + tb, 0, 255);
}

void bt709_rgb2yuv(uint8_t r, uint8_t g, uint8_t b, uint8_t * y, uint8_t * u, uint8_t * v)
{
	uint8_t ty = (54 * r + 184 * g + 18 * b) >> 8;
	int16_t tu = b - ty;
	int16_t tv = r - ty;
	*y = ((ty * 110) >> 7) + 16;
	*u = ((tu * 121) >> 8) + 128;
	*v = ((tv * 143) >> 8) + 128;
}
