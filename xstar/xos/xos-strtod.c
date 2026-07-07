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

double xos_strtod(const char * nptr, char ** endptr)
{
	double number;
	int exponent;
	int negative;
	char * p = (char *)nptr;
	double p10;
	int n;
	int num_digits;
	int num_decimals;

	while(xos_isspace(*p))
		p++;
	negative = 0;
	switch(*p)
	{
	case '-':
		negative = 1;
		p++;
		break;
	case '+':
		p++;
		break;
	}
	number = 0.;
	exponent = 0;
	num_digits = 0;
	num_decimals = 0;
	while(xos_isdigit(*p))
	{
		number = number * 10. + (*p - '0');
		p++;
		num_digits++;
	}
	if(*p == '.')
	{
		p++;
		while(xos_isdigit(*p))
		{
			number = number * 10. + (*p - '0');
			p++;
			num_digits++;
			num_decimals++;
		}
		exponent -= num_decimals;
	}
	if(num_digits == 0)
	{
		return 0.0;
	}
	if(negative)
		number = -number;
	if((*p == 'e') || (*p == 'E'))
	{
		negative = 0;
		switch(*++p)
		{
		case '-':
			negative = 1;
			p++;
			break;
		case '+':
			p++;
			break;
		}
		n = 0;
		while(xos_isdigit(*p))
		{
			n = n * 10 + (*p - '0');
			p++;
		}
		if(negative)
			exponent -= n;
		else
			exponent += n;
	}
	if((exponent < -307) || (exponent > 308))
	{
		return 0.0;
	}
	p10 = 10.;
	n = exponent;
	if(n < 0)
		n = -n;
	while(n)
	{
		if(n & 1)
		{
			if(exponent < 0)
				number /= p10;
			else
				number *= p10;
		}
		n >>= 1;
		p10 *= p10;
	}
	if(endptr)
		*endptr = p;
	return number;
}
