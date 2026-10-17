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

#include <libx/cobs.h>

int cobs_encode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len)
{
	unsigned int read = 0;
	unsigned int written = 1;
	unsigned int code_index = 0;
	unsigned char code = 1;

	if(!dst || (!src && len) || dst == src)
		return -1;
	if(cobs_encode_bound(len) <= 0)
		return -1;
	if(capacity == 0)
		return -1;
	while(read < len)
	{
		unsigned char value = src[read++];

		if(value == 0)
		{
			dst[code_index] = code;
			if(written == capacity)
				return -1;
			code_index = written++;
			code = 1;
		}
		else
		{
			if(written == capacity)
				return -1;
			dst[written++] = value;
			if(++code == 0xff && read < len)
			{
				dst[code_index] = code;
				if(written == capacity)
					return -1;
				code_index = written++;
				code = 1;
			}
		}
	}
	dst[code_index] = code;
	return (int)written;
}

int cobs_decode(unsigned char * dst, unsigned int capacity, unsigned char * src, unsigned int len)
{
	unsigned int read = 0;
	unsigned int written = 0;

	if((!src && len) || (!dst && capacity))
		return -1;
	if(len == 0)
		return -1;
	while(read < len)
	{
		unsigned char code = src[read++];
		unsigned int copy = (unsigned int)code - 1;

		if(code == 0 || copy > len - read)
			return -1;
		if(copy > capacity - written)
			return -1;
		while(copy--)
		{
			unsigned char value = src[read++];

			if(value == 0)
				return -1;
			dst[written++] = value;
		}
		if(code != 0xff && read < len)
		{
			if(written == capacity)
				return -1;
			dst[written++] = 0;
		}
	}
	return (int)written;
}
