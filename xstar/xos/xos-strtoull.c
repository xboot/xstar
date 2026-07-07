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

unsigned long long xos_strtoull(const char * nptr, char ** endptr, int base)
{
	const char * s;
	unsigned long long acc, cutoff;
	int c;
	int neg, any, cutlim;

	s = nptr;
	do {
		c = (unsigned char)*s++;
	} while(xos_isspace(c));
	if(c == '-')
	{
		neg = 1;
		c = *s++;
	}
	else
	{
		neg = 0;
		if(c == '+')
			c = *s++;
	}
	if(((base == 0) || (base == 16)) && (c == '0') && ((*s == 'x') || (*s == 'X')))
	{
		c = s[1];
		s += 2;
		base = 16;
	}
	if(base == 0)
		base = c == '0' ? 8 : 10;
	switch(base)
	{
    case 4:
    	cutoff = ULLONG_MAX / 4;
        cutlim = ULLONG_MAX % 4;
        break;
    case 8:
    	cutoff = ULLONG_MAX / 8;
        cutlim = ULLONG_MAX % 8;
        break;
    case 10:
    	cutoff = ULLONG_MAX / 10;
        cutlim = ULLONG_MAX % 10;
        break;
    case 16:
    	cutoff = ULLONG_MAX / 16;
        cutlim = ULLONG_MAX % 16;
        break;
	default:
		cutoff = ULLONG_MAX / base;
		cutlim = ULLONG_MAX % base;
		break;
	}
	for(acc = 0, any = 0; ; c = (unsigned char) *s++)
	{
		if(xos_isdigit(c))
			c -= '0';
		else if(xos_isalpha(c))
			c -= xos_isupper(c) ? 'A' - 10 : 'a' - 10;
		else
			break;
		if(c >= base)
			break;
		if(any < 0)
			continue;
		if((acc > cutoff) || ((acc == cutoff) && (c > cutlim)))
		{
			any = -1;
			acc = ULLONG_MAX;
		}
		else
		{
			any = 1;
			acc *= (unsigned long long)base;
			acc += c;
		}
	}
	if(neg && any > 0)
		acc = -acc;
	if(endptr != 0)
		*endptr = (char *) (any ? s - 1 : nptr);
	return acc;
}
