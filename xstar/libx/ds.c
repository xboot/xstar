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
#include <libx/ds.h>

static int ds_grow(struct ds_t * ds, size_t needed)
{
	if(ds->capacity < needed)
	{
		size_t ncap = (ds->capacity << 1);
		while(ncap < needed)
			ncap <<= 1;
		char * ndata = xos_mem_realloc(ds->data, ncap);
		if(!ndata)
			return 0;
		ds->data = ndata;
		ds->capacity = ncap;
	}
	return 1;
}

struct ds_t * ds_alloc(void)
{
	struct ds_t * ds = xos_mem_malloc(sizeof(struct ds_t));

	if(ds)
	{
		ds->data = xos_mem_malloc(32);
		if(!ds->data)
		{
			xos_mem_free(ds);
			return NULL;
		}
		ds->data[0] = '\0';
		ds->length = 0;
		ds->capacity = 32;
		return ds;
	}
	return NULL;
}

struct ds_t * ds_alloc_from_str(const char * str)
{
	struct ds_t * ds = ds_alloc();

	if(ds)
	{
		if(str && (str[0] != '\0'))
		{
			size_t slen = xos_strlen(str);
			if(ds_grow(ds, slen + 1))
			{
				xos_memcpy(ds->data, str, slen + 1);
				ds->length = slen;
			}
		}
	}
	return ds;
}

struct ds_t * ds_alloc_from_buf(const void * buf, int len)
{
	struct ds_t * ds = ds_alloc();

	if(ds)
	{
		if(buf && (len > 0))
		{
			if(ds_grow(ds, len + 1))
			{
				xos_memcpy(ds->data, buf, len);
				ds->data[len] = '\0';
				ds->length = len;
			}
		}
	}
	return ds;
}

void ds_free(struct ds_t * ds)
{
	if(ds)
	{
		xos_mem_free(ds->data);
		xos_mem_free(ds);
	}
}

struct ds_t * ds_clone(struct ds_t * ds)
{
	return ds_alloc_from_str(ds_cstr(ds));
}

struct ds_t * ds_substr(struct ds_t * ds, size_t start, size_t len)
{
	struct ds_t * sub = ds_alloc();

	if(sub && ds)
	{
		if(start >= ds->length)
			return sub;
		if(start + len > ds->length)
			len = ds->length - start;
		if(ds_grow(sub, len + 1))
		{
			if(len > 0)
				xos_memcpy(sub->data, ds->data + start, len);
			sub->data[len] = '\0';
			sub->length = len;
		}
	}
	return sub;
}

void ds_clear(struct ds_t * ds)
{
	if(ds)
	{
		ds->data[0] = '\0';
		ds->length = 0;
	}
}

void ds_trim(struct ds_t * ds)
{
	if(ds && (ds->length > 0))
	{
		size_t start = 0;
		size_t end = ds->length - 1;
		while((start < ds->length) && xos_isspace((unsigned char)ds->data[start]))
			start++;
		while((end > start) && xos_isspace((unsigned char)ds->data[end]))
			end--;
		size_t len = end - start + 1;
		if(start > 0)
			xos_memmove(ds->data, ds->data + start, len);
		ds->data[len] = '\0';
		ds->length = len;
	}
}

int ds_append(struct ds_t * ds, const char * str)
{
	if(ds && str)
	{
		size_t slen = xos_strlen(str);
		if(slen == 0)
			return 1;
		if(!ds_grow(ds, ds->length + slen + 1))
			return 0;
		xos_memcpy(ds->data + ds->length, str, slen + 1);
		ds->length += slen;
		return 1;
	}
	return 0;
}

int ds_append_char(struct ds_t * ds, const char c)
{
	if(ds)
	{
		if(!ds_grow(ds, ds->length + 2))
			return 0;
		ds->data[ds->length] = c;
		ds->data[ds->length + 1] = '\0';
		ds->length++;
		return 1;
	}
	return 0;
}

int ds_prepend(struct ds_t * ds, const char * str)
{
	if(ds && str)
	{
		size_t slen = xos_strlen(str);
		if(slen == 0)
			return 1;
		if(!ds_grow(ds, ds->length + slen + 1))
			return 0;
		xos_memmove(ds->data + slen, ds->data, ds->length + 1);
		xos_memcpy(ds->data, str, slen);
		ds->length += slen;
		return 1;
	}
	return 0;
}

int ds_insert(struct ds_t * ds, size_t pos, const char * str)
{
	if(ds && str)
	{
		if(pos > ds->length)
			pos = ds->length;
		size_t slen = xos_strlen(str);
		if(slen == 0)
			return 1;
		if(!ds_grow(ds, ds->length + slen + 1))
			return 0;
		xos_memmove(ds->data + pos + slen, ds->data + pos, ds->length - pos + 1);
		xos_memcpy(ds->data + pos, str, slen);
		ds->length += slen;
		return 1;
	}
	return 0;
}

int ds_delete(struct ds_t * ds, size_t start, size_t len)
{
	if(ds)
	{
		if(start >= ds->length)
			return 1;
		if(start + len > ds->length)
			len = ds->length - start;
		xos_memmove(ds->data + start, ds->data + start + len, ds->length - start - len + 1);
		ds->length -= len;
		return 1;
	}
	return 0;
}

int ds_copy(struct ds_t * ds, const char * fmt, ...)
{
	if(ds && fmt)
	{
		va_list args, cpys;
		va_start(args, fmt);
		va_copy(cpys, args);
		int len = xos_vsnprintf(NULL, 0, fmt, args);
		va_end(args);
		if(len < 0)
		{
			va_end(cpys);
			return 0;
		}
		if(!ds_grow(ds, len + 1))
		{
			va_end(cpys);
			return 0;
		}
		xos_vsnprintf(ds->data, ds->capacity, fmt, cpys);
		ds->length = len;
		va_end(cpys);
		return len;
	}
	return 0;
}

int ds_concat(struct ds_t * ds, const char * fmt, ...)
{
	if(ds && fmt)
	{
		va_list args, cpys;
		va_start(args, fmt);
		va_copy(cpys, args);
		int len = xos_vsnprintf(NULL, 0, fmt, args);
		va_end(args);
		if(len < 0)
		{
			va_end(cpys);
			return 0;
		}
		if(!ds_grow(ds, ds->length + len + 1))
		{
			va_end(cpys);
			return 0;
		}
		xos_vsnprintf(ds->data + ds->length, len + 1, fmt, cpys);
		ds->length += len;
		va_end(cpys);
		return len;
	}
	return 0;
}

int ds_find(struct ds_t * ds, const char * sub)
{
	if(!ds || !sub)
		return -1;
	if(xos_strlen(sub) == 0)
		return (int)ds->length;
	char * p = xos_strstr(ds->data, sub);
	if(!p)
		return -1;
	return (int)(p - ds->data);
}

int ds_rfind(struct ds_t * ds, const char * sub)
{
	if(!ds || !sub || (ds->length == 0))
		return -1;
	size_t sub_len = xos_strlen(sub);
	if(sub_len == 0)
		return (int)ds->length;
	if(sub_len > ds->length)
		return -1;
	for(int i = (int)(ds->length - sub_len); i >= 0; i--)
	{
		if(xos_memcmp(ds->data + i, sub, sub_len) == 0)
			return i;
	}
	return -1;
}

int ds_replace(struct ds_t * ds, const char * ostr, const char * nstr)
{
	if(!ds || !ostr || !nstr)
		return 0;
	size_t old_len = xos_strlen(ostr);
	size_t new_len = xos_strlen(nstr);
	if(old_len == 0)
		return 0;
	int count = 0;
	char * p = ds->data;
	while((p = xos_strstr(p, ostr)) != NULL)
	{
		count++;
		p += old_len;
	}
	if(count == 0)
		return 0;
	size_t new_total_len = ds->length + count * (new_len - old_len);
	if(!ds_grow(ds, new_total_len + 1))
		return 0;
	char * result = xos_mem_malloc(new_total_len + 1);
	if(!result)
		return 0;
	char * src = ds->data;
	char * dst = result;
	p = src;
	while((p = xos_strstr(p, ostr)) != NULL)
	{
		size_t seg_len = p - src;
		xos_memcpy(dst, src, seg_len);
		dst += seg_len;
		xos_memcpy(dst, nstr, new_len);
		dst += new_len;
		p += old_len;
		src = p;
	}
	xos_strcpy(dst, src);
	xos_memcpy(ds->data, result, new_total_len + 1);
	xos_mem_free(result);
	ds->length = new_total_len;
	return count;
}

int ds_replace_first(struct ds_t * ds, const char * ostr, const char * nstr)
{
	if(!ds || !ostr || !nstr)
		return 0;
	int pos = ds_find(ds, ostr);
	if(pos < 0)
		return 0;
	ds_delete(ds, pos, xos_strlen(ostr));
	return ds_insert(ds, pos, nstr);
}

int ds_replace_last(struct ds_t * ds, const char * ostr, const char * nstr)
{
	if(!ds || !ostr || !nstr)
		return 0;
	int pos = ds_rfind(ds, ostr);
	if(pos < 0)
		return 0;
	ds_delete(ds, pos, xos_strlen(ostr));
	return ds_insert(ds, pos, nstr);
}

struct ds_iter_t * ds_iter_alloc(struct ds_t * ds, const char * delim)
{
	if(ds && delim && (ds->length > 0))
	{
		struct ds_iter_t * iter = xos_mem_malloc(sizeof(struct ds_iter_t));
		if(iter)
		{
			iter->data = xos_strdup(ds->data);
			if(!iter->data)
			{
				xos_mem_free(iter);
				return NULL;
			}
			iter->delim = xos_strdup(delim);
			iter->pos = iter->data;
			return iter;
		}
	}
	return NULL;

}

void ds_iter_free(struct ds_iter_t * iter)
{
	if(iter)
	{
		if(iter->data)
			xos_mem_free(iter->data);
		if(iter->delim)
			xos_mem_free(iter->delim);
		xos_mem_free(iter);
	}
}

char * ds_iter_next(struct ds_iter_t * iter)
{
	if(iter && iter->pos && (*iter->pos != '\0'))
	{
		char * token = iter->pos;
		char * end = xos_strstr(token, iter->delim);
		if(end)
		{
			*end = '\0';
			iter->pos = end + xos_strlen(iter->delim);
		}
		else
			iter->pos = NULL;
		return token;
	}
	return NULL;
}
