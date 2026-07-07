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

int xos_strnatcasecmp(const char * s1, const char * s2)
{
	if(s1 == s2)
		return 0;
	while(1)
	{
		int c1 = xos_tolower(*s1++);
		int c2 = xos_tolower(*s2++);
		if(xos_isdigit(c1) && xos_isdigit(c2))
		{
			while(c1 == '0')
				c1 = xos_tolower(*s1++);
			while(c2 == '0')
				c2 = xos_tolower(*s2++);
			const char * p1 = s1 - 1;
			const char * p2 = s2 - 1;
			while(xos_isdigit(*s1))
				s1++;
			while(xos_isdigit(*s2))
				s2++;
			int len1 = s1 - p1;
			int len2 = s2 - p2;
			if(len1 != len2)
				return len1 - len2;
			while((p1 < s1) && (p2 < s2))
			{
				if(xos_tolower(*p1) != xos_tolower(*p2))
					return xos_tolower(*p1) - xos_tolower(*p2);
				p1++;
				p2++;
			}
			c1 = xos_tolower(*s1++);
			c2 = xos_tolower(*s2++);
		}
		if((c1 == '\0') && (c2 == '\0'))
			return 0;
		else if(c1 == '\0')
			return -1;
		else if(c2 == '\0')
			return 1;
		else
			return c1 - c2;
	}
}
