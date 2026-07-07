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

#include <libx/rc4.h>

void rc4_crypt(uint8_t * key, int kl, uint8_t * dat, int dl)
{
	uint8_t s[256], k[256];

	if(key && (kl > 0) && dat && (dl > 0))
	{
		for(int i = 0; i < 256; i++)
		{
			s[i] = (uint8_t)i;
			k[i] = key[i % kl];
		}
		for(int i = 0, j = 0; i < 256; i++)
		{
			j = (j + s[i] + k[i]) & 0xff;
			uint8_t t = s[i];
			s[i] = s[j];
			s[j] = t;
		}
		for(int i = 0, j = 0, o = 0; o < dl; o++)
		{
			i = (i + 1) & 0xff;
			j = (j + s[i]) & 0xff;
			uint8_t t = s[i];
			s[i] = s[j];
			s[j] = t;
			dat[o] ^= s[(s[i] + s[j]) & 0xff];
		}
	}
}
