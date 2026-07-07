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
#include <libx/uri.h>

static char * uri_strnstr(const char * s1, const char * s2, size_t n)
{
	size_t l2;

	l2 = xos_strlen(s2);
	if (!l2)
		return (char *)s1;

	while (n >= l2)
	{
		n--;
		if (!xos_memcmp(s1, s2, l2))
			return (char *)s1;
		s1++;
	}

	return NULL;
}

static const char * find_string(const char * s, size_t len, const char * str)
{
	const char * end = s + len;
	const char * p = s;
	size_t n = xos_strlen(str);
	int i;

	while(p < end)
	{
		for(i = 0; i < n; i++)
		{
			if(*p == str[i])
				return p;
		}
		p++;
	}
	return NULL;
}

static const char * find_string_reverse(const char * s, size_t len, const char * str)
{
	const char * end = s + len;
	const char * p = end - 1;
	size_t n = xos_strlen(str);
	int i;

	while(p >= s)
	{
		for(i = 0; i < n; i++)
		{
			if(*p == str[i])
				return p;
		}
		p--;
	}
	return NULL;
}

static const char * lookup_scheme(const char * s)
{
	const char * p = s;
	char c;

	if(xos_strlen(s) == 0)
		return NULL;
	if(!xos_isalpha(*p))
		return NULL;
	p++;
	while(*p != '\0')
	{
		c = *p;
		if(c == ':')
			return p;
		if(!xos_isalpha(c) && !xos_isdigit(c) && (c != '+') && (c != '-') && (c != '.'))
			return NULL;
		p++;
	}
	return NULL;
}

static int parse_user_password(const char * s, size_t len, struct uri_t * uri)
{
	const char * end = s + len;
	const char * found;

	found = uri_strnstr(s, ":", len);
	if(found)
	{
		uri->user = xos_strndup(s, found - s);
		if(uri->user == NULL)
			return 0;
		uri->pass = xos_strndup(found + 1, end - found - 1);
		if(uri->pass == NULL)
			return 0;
	}
	else
	{
		uri->user = xos_strndup(s, len);
		if(uri->user == NULL)
			return 0;
	}
	return 1;
}

static int parse_authority(const char * s, size_t len, struct uri_t * uri)
{
	const char * end = s + len;
	const char * p, * found, * host_start, * host_end;
	int port;

	uri->port = 0;
	if(len == 0)
		return 1;
	found = uri_strnstr(s, "@", len);
	if(found)
	{
		if(!parse_user_password(s, found - s, uri))
			return 0;
		host_start = found + 1;
	}
	else
		host_start = s;
	if(*host_start == '[')
	{
		if(find_string(host_start + 1, end - host_start - 1, "["))
			return 0;
		host_end = find_string(host_start + 1, end - host_start - 1, "]");
		if(!host_end)
			return 0;
		if(host_end + 1 != end && host_end[1] != ':')
			return 0;
		host_end++;
	}
	else
	{
		host_end = find_string_reverse(host_start, end - host_start, ":");
		if(host_end == NULL)
			host_end = end;
		if(find_string(host_start, host_end - host_start, "[]"))
			return 0;
	}
	if(find_string(host_start, host_end - host_start, " "))
		return 0;
	if(host_end == end)
	{
		if(host_start == end)
			return 0;
		uri->host = xos_strndup(host_start, end - host_start);
		if(uri->host == NULL)
			return 0;
		return 1;
	}
	if(host_start == host_end)
		return 0;
	if(host_end + 1 < end)
	{
		p = host_end + 1;
		port = 0;
		while(p < end)
		{
			if(*p < '0' || *p > '9')
				return 0;
			port = port * 10 + *p - '0';
			if(port > 65535)
				return 0;
			p++;
		}
	}
	else
		port = 0;
	uri->host = xos_strndup(host_start, (size_t)(host_end - host_start));
	if(uri->host == NULL)
		return 0;
	uri->port = port;
	return 1;
}

struct uri_t * uri_alloc(const char * s)
{
	struct uri_t * uri;
	const char * p;
	const char * end;
	const char * found;
	size_t len;

	if(!s)
		return NULL;
	end = s + xos_strlen(s);
	for(p = s; p < end; p++)
	{
		if(xos_iscntrl(*p))
			return NULL;
	}
	uri = xos_mem_malloc(sizeof(struct uri_t));
	if(!uri)
		return NULL;
	xos_memset(uri, 0, sizeof(struct uri_t));
	uri->port = 0;
	p = s;
	found = lookup_scheme(p);
	if(found)
	{
		uri->scheme = xos_strndup(s, (size_t)(found - p));
		if(uri->scheme == NULL)
			goto error;
		p = found + 1;
		if(p >= end)
			return uri;
	}
	if((xos_strlen(p) >= 2) && (p[0] == '/') && (p[1] == '/'))
	{
		p = p + 2;
		found = find_string(p, xos_strlen(p), "/?#");
		if(found == NULL)
			len = xos_strlen(p);
		else
			len = (size_t)(found - p);
		if(!parse_authority(p, len, uri))
			goto error;
		if(!found)
			return uri;
		p = found;
	}
	if((*p != '?') && (*p != '#'))
	{
		found = find_string(p, xos_strlen(p), "?#");
		if(found == NULL)
		{
			uri->path = xos_strdup(p);
			if(uri->path == NULL)
				goto error;
		}
		else
		{
			if(found != p)
			{
				uri->path = xos_strndup(p, (size_t)(found - p));
				if(uri->path == NULL)
					goto error;
			}
		}
		if(!found)
			return uri;
		p = found;
	}
	if(*p == '?')
	{
		p = p + 1;
		found = find_string(p, xos_strlen(p), "#");
		if(found == NULL)
			uri->query = xos_strdup(p);
		else
			uri->query = xos_strndup(p, (size_t)(found - p));
		if(uri->query == NULL)
			goto error;
		if(!found)
			return uri;
		p = found;
	}
	p = p + 1;
	uri->fragment = xos_strdup(p);
	if(uri->fragment == NULL)
		goto error;
	return uri;
error:
	uri_free(uri);
	return NULL;
}

void uri_free(struct uri_t * uri)
{
	if(uri)
	{
		if(uri->scheme)
			xos_mem_free(uri->scheme);
		if(uri->user)
			xos_mem_free(uri->user);
		if(uri->pass)
			xos_mem_free(uri->pass);
		if(uri->host)
			xos_mem_free(uri->host);
		if(uri->path)
			xos_mem_free(uri->path);
		if(uri->query)
			xos_mem_free(uri->query);
		if(uri->fragment)
			xos_mem_free(uri->fragment);
		xos_mem_free(uri);
	}
}
