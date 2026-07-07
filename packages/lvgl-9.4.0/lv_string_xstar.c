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

#if LV_USE_STDLIB_STRING == LV_STDLIB_CUSTOM

#include <xstar.h>
#include <lvgl/src/lv_conf_internal.h>
#include <lvgl/src/stdlib/lv_string.h>

void * lv_memcpy(void * dst, const void * src, size_t len)
{
	return xos_memcpy(dst, src, len);
}

void lv_memset(void * dst, uint8_t v, size_t len)
{
	xos_memset(dst, v, len);
}

void * lv_memmove(void * dst, const void * src, size_t len)
{
	return xos_memmove(dst, src, len);
}

int lv_memcmp(const void * p1, const void * p2, size_t len)
{
	return xos_memcmp(p1, p2, len);
}

size_t lv_strlen(const char * str)
{
	return xos_strlen(str);
}

size_t lv_strnlen(const char * str, size_t len)
{
	return xos_strnlen(str, len);
}

size_t lv_strlcpy(char * dst, const char * src, size_t dst_size)
{
	return xos_strlcpy(dst, src, dst_size);
}

char * lv_strncpy(char * dst, const char * src, size_t dest_size)
{
	return xos_strncpy(dst, src, dest_size);
}

char * lv_strcpy(char * dst, const char * src)
{
	return xos_strcpy(dst, src);
}

int lv_strcmp(const char * s1, const char * s2)
{
	return xos_strcmp(s1, s2);
}

int lv_strncmp(const char * s1, const char * s2, size_t len)
{
	return xos_strncmp(s1, s2, len);
}

char * lv_strdup(const char * src)
{
	return xos_strdup(src);
}

char * lv_strndup(const char * src, size_t len)
{
	return xos_strndup(src, len);
}

char * lv_strcat(char * dst, const char * src)
{
	return xos_strcat(dst, src);
}

char * lv_strncat(char * dst, const char * src, size_t src_len)
{
	return xos_strncat(dst, src, src_len);
}

char * lv_strchr(const char * str, int c)
{
	return xos_strchr(str, c);
}

#endif
