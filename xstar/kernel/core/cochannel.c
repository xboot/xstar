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

#include <xstar.h>
#include <kernel/core/cochannel.h>

struct cochannel_t * cochannel_alloc(unsigned int size)
{
	if(!xstar_feature_coroutine())
		return NULL;

	struct cochannel_t * c = xos_mem_malloc(sizeof(struct cochannel_t));
	if(!c)
		return NULL;

	if(size < 16)
		size = 16;
	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	c->buffer = xos_mem_malloc(size);
	if(!c->buffer)
	{
		xos_mem_free(c);
		return NULL;
	}
	c->size = size;
	c->in = 0;
	c->out = 0;

	return c;
}

void cochannel_free(struct cochannel_t * c)
{
	if(c)
	{
		xos_mem_free(c->buffer);
		xos_mem_free(c);
	}
}

static inline int cochannel_isempty(struct cochannel_t * c)
{
	return (c->in - c->out <= 0) ? 1 : 0;
}

static inline int cochannel_isfull(struct cochannel_t * c)
{
	return (c->in - c->out >= c->size) ? 1 : 0;
}

static inline unsigned int __cochannel_put(struct cochannel_t * c, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = XMIN(len, c->size - c->in + c->out);
	l = XMIN(len, c->size - (c->in & (c->size - 1)));
	xos_memcpy(c->buffer + (c->in & (c->size - 1)), buf, l);
	xos_memcpy(c->buffer, buf + l, len - l);
	c->in += len;

	return len;
}

static inline unsigned int __cochannel_get(struct cochannel_t * c, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = XMIN(len, c->in - c->out);
	l = XMIN(len, c->size - (c->out & (c->size - 1)));
	xos_memcpy(buf, c->buffer + (c->out & (c->size - 1)), l);
	xos_memcpy(buf + l, c->buffer, len - l);
	c->out += len;

	return len;
}

static inline unsigned int cochannel_put(struct cochannel_t * c, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	if(cochannel_isfull(c))
		l = 0;
	else
		l = __cochannel_put(c, buf, len);
	return l;
}

static inline unsigned int cochannel_get(struct cochannel_t * c, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	if(cochannel_isempty(c))
		l = 0;
	else
		l = __cochannel_get(c, buf, len);
	return l;
}

void cochannel_send(struct scheduler_t * sched, struct cochannel_t * c, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	if(c && buf)
	{
		while(len > 0)
		{
			l = cochannel_put(c, buf, len);
			buf += l;
			len -= l;
			coroutine_yield(sched);
		}
	}
}

void cochannel_recv(struct scheduler_t * sched, struct cochannel_t * c, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	if(c && buf)
	{
		while(len > 0)
		{
			l = cochannel_get(c, buf, len);
			buf += l;
			len -= l;
			coroutine_yield(sched);
		}
	}
}
