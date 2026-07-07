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
#include <libx/path.h>

char * path_basename(char * path)
{
	int i;
	if(!path || !*path)
		return ".";
	i = xos_strlen(path) - 1;
	for(; i && path[i] == '/'; i--)
		path[i] = 0;
	for(; i && path[i-1] != '/'; i--);
	return path + i;
}

char * path_dirname(char * path)
{
	int i;
	if(!path || !*path)
		return ".";
	i = xos_strlen(path) - 1;
	for(; path[i] == '/'; i--)
	{
		if(!i)
			return "/";
	}
	for(; path[i] != '/'; i--)
	{
		if(!i)
			return ".";
	}
	for(; path[i] == '/'; i--)
	{
		if(!i)
			return "/";
	}
	path[i + 1] = 0;
	return path;
}

const char * path_fileext(const char * filename)
{
	const char * ret = NULL;
	const char * p;

	if(filename != NULL)
	{
		ret = p = xos_strchr(filename, '.');
		while(p != NULL)
		{
			p = xos_strchr(p + 1, '.');
			if(p != NULL)
				ret = p;
		}
		if(ret != NULL)
			ret++;
	}
	return ret;
}
