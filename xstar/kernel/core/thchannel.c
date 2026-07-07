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
#include <kernel/core/thchannel.h>

struct thchannel_t * thchannel_alloc(unsigned int size)
{
	if(!xstar_feature_thread())
		return NULL;

	struct thchannel_t * c = xos_mem_malloc(sizeof(struct thchannel_t));
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
	xos_mutex_init(&c->lock);
	xos_semaphore_init(&c->ssem, 0);
	xos_semaphore_init(&c->rsem, 0);

	return c;
}

void thchannel_free(struct thchannel_t * c)
{
	if(c)
	{
		xos_mutex_exit(&c->lock);
		xos_semaphore_exit(&c->ssem);
		xos_semaphore_exit(&c->rsem);
		xos_mem_free(c->buffer);
		xos_mem_free(c);
	}
}

void thchannel_reset(struct thchannel_t * c)
{
	if(c)
	{
		xos_mutex_lock(&c->lock);
		{
			c->in = c->out = 0;
		}
		xos_mutex_unlock(&c->lock);
	}
}

int thchannel_isempty(struct thchannel_t * c)
{
	int ret = 0;

	if(c)
	{
		xos_mutex_lock(&c->lock);
		{
			ret = (c->in - c->out == 0) ? 1 : 0;
		}
		xos_mutex_unlock(&c->lock);
	}
	return ret;
}

int thchannel_isfull(struct thchannel_t * c)
{
	int ret = 0;

	if(c)
	{
		xos_mutex_lock(&c->lock);
		{
			ret = (c->in - c->out == c->size) ? 1 : 0;
		}
		xos_mutex_unlock(&c->lock);
	}
	return ret;
}

unsigned int thchannel_size(struct thchannel_t * c)
{
	unsigned int ret = 0;

	if(c)
	{
		xos_mutex_lock(&c->lock);
		{
			ret = c->size;
		}
		xos_mutex_unlock(&c->lock);
	}
	return ret;
}

unsigned int thchannel_length(struct thchannel_t * c)
{
	unsigned int ret = 0;

	if(c)
	{
		xos_mutex_lock(&c->lock);
		{
			ret = c->in - c->out;
		}
		xos_mutex_unlock(&c->lock);
	}
	return ret;
}

unsigned int thchannel_available(struct thchannel_t * c)
{
	unsigned int ret = 0;

	if(c)
	{
		xos_mutex_lock(&c->lock);
		{
			ret = c->size - c->in + c->out;
		}
		xos_mutex_unlock(&c->lock);
	}
	return ret;
}

static inline unsigned int thchannel_put(struct thchannel_t * c, unsigned char * buf, unsigned int len)
{
	xos_mutex_lock(&c->lock);
	{
		len = XMIN(len, c->size - c->in + c->out);
		if(len > 0)
		{
			unsigned int l = XMIN(len, c->size - (c->in & (c->size - 1)));
			xos_memcpy(c->buffer + (c->in & (c->size - 1)), buf, l);
			xos_memcpy(c->buffer, buf + l, len - l);
			c->in += len;
		}
	}
	xos_mutex_unlock(&c->lock);
	return len;
}

static inline unsigned int thchannel_get(struct thchannel_t * c, unsigned char * buf, unsigned int len)
{
	xos_mutex_lock(&c->lock);
	{
		len = XMIN(len, c->in - c->out);
		if(len > 0)
		{
			unsigned int l = XMIN(len, c->size - (c->out & (c->size - 1)));
			xos_memcpy(buf, c->buffer + (c->out & (c->size - 1)), l);
			xos_memcpy(buf + l, c->buffer, len - l);
			c->out += len;
		}
	}
	xos_mutex_unlock(&c->lock);
	return len;
}

unsigned int thchannel_send(struct thchannel_t * c, unsigned char * buf, unsigned int len, unsigned int timeout)
{
	unsigned int cnt = 0;

	if(c && buf)
	{
		while(len > 0)
		{
			unsigned int l = thchannel_put(c, buf, len);
			buf += l;
			len -= l;
			cnt += l;
			xos_semaphore_post(&c->rsem);
			if((l == 0) && !xos_semaphore_wait(&c->ssem, timeout))
				break;
		}
	}
	return cnt;
}

unsigned int thchannel_recv(struct thchannel_t * c, unsigned char * buf, unsigned int len, unsigned int timeout)
{
	unsigned int cnt = 0;

	if(c && buf)
	{
		while(len > 0)
		{
			unsigned int l = thchannel_get(c, buf, len);
			buf += l;
			len -= l;
			cnt += l;
			xos_semaphore_post(&c->ssem);
			if((l == 0) && !xos_semaphore_wait(&c->rsem, timeout))
				break;
		}
	}
	return cnt;
}
