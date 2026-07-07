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

static inline int digitval(int ch)
{
    unsigned int d;

    d = (unsigned int)(ch - '0');
    if (d < 10) return (int)d;

    d = (unsigned int)(ch - 'a');
    if (d < 6) return (int)(d+10);

    d = (unsigned int)(ch - 'A');
    if (d < 6) return (int)(d+10);

    return -1;
}

uintmax_t xos_strntoumax(const char * nptr, char ** endptr, int base, size_t n)
{
	const unsigned char * p = (const unsigned char *) nptr;
	const unsigned char * end = p + n;
	int minus = 0;
	uintmax_t v = 0;
	int d;

	while((p < end) && xos_isspace(*p))
		p++;
	if(p < end)
	{
		char c = p[0];
		if((c == '-') || (c == '+'))
		{
			minus = (c == '-');
			p++;
		}
	}
	if(base == 0)
	{
		if((p + 2 < end) && (p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))
		{
			p += 2;
			base = 16;
		}
		else if((p + 1 < end) && (p[0] == '0'))
		{
			p += 1;
			base = 8;
		}
		else
		{
			base = 10;
		}
	}
	else if(base == 16)
	{
		if((p + 2 < end) && (p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))
		{
			p += 2;
		}
	}
	while((p < end) && ((d = digitval(*p)) >= 0) && (d < base))
	{
		v = v * base + d;
		p += 1;
	}
	if(endptr)
		*endptr = (char *) p;
	return minus ? -v : v;
}
