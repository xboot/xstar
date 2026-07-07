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
#include <libx/fifo.h>

void __fifo_reset(struct fifo_t * f)
{
	f->in = f->out = 0;
}

int __fifo_isempty(struct fifo_t * f)
{
	return (f->in - f->out) == 0;
}

int __fifo_isfull(struct fifo_t * f)
{
	return (f->in - f->out) == f->size;
}

unsigned int __fifo_size(struct fifo_t * f)
{
	return f->size;
}

unsigned int __fifo_length(struct fifo_t * f)
{
	return f->in - f->out;
}

unsigned int __fifo_available(struct fifo_t * f)
{
	return f->size - f->in + f->out;
}

unsigned int __fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = XMIN(len, f->size - f->in + f->out);
	l = XMIN(len, f->size - (f->in & (f->size - 1)));
	xos_memcpy(f->buffer + (f->in & (f->size - 1)), buf, l);
	xos_memcpy(f->buffer, buf + l, len - l);
	f->in += len;

	return len;
}

unsigned int __fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = XMIN(len, f->in - f->out);
	l = XMIN(len, f->size - (f->out & (f->size - 1)));
	xos_memcpy(buf, f->buffer + (f->out & (f->size - 1)), l);
	xos_memcpy(buf + l, f->buffer, len - l);
	f->out += len;

	return len;
}

unsigned int __fifo_peek(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int l;

	len = XMIN(len, f->in - f->out);
	l = XMIN(len, f->size - (f->out & (f->size - 1)));
	xos_memcpy(buf, f->buffer + (f->out & (f->size - 1)), l);
	xos_memcpy(buf + l, f->buffer, len - l);

	return len;
}

unsigned int __fifo_put_packet(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	if((len > 0) && (__fifo_available(f) >= sizeof(unsigned int) + len))
	{
		__fifo_put(f, (unsigned char *)&len, sizeof(unsigned int));
		return __fifo_put(f, buf, len);
	}
	return 0;
}

unsigned int  __fifo_get_packet(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret = 0;

	if(__fifo_length(f) >= sizeof(unsigned int))
	{
		unsigned int l = 0;
		if(__fifo_peek(f, (unsigned char *)&l, sizeof(unsigned int)) == sizeof(unsigned int))
		{
			if((l > 0) && (l <= __fifo_length(f) - sizeof(unsigned int)))
			{
				__fifo_get(f, (unsigned char *)&l, sizeof(unsigned int));
				if(l <= len)
				{
					if(__fifo_get(f, buf, l) == l)
						ret = l;
				}
				else
				{
					unsigned char * pkt = xos_mem_malloc(l);
					if(pkt)
					{
						if(__fifo_get(f, pkt, l) == l)
						{
							ret = len;
							xos_memcpy(buf, pkt, len);
						}
						xos_mem_free(pkt);
					}
				}
			}
		}
	}
	return ret;
}

struct fifo_t * fifo_alloc(unsigned int size)
{
	struct fifo_t * f;

	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	f = xos_mem_malloc(sizeof(struct fifo_t));
	if(!f)
		return NULL;

	f->buffer = xos_mem_malloc(size);
	if(!f->buffer)
	{
		xos_mem_free(f);
		return NULL;
	}
	f->size = size;
	f->in = 0;
	f->out = 0;
	xos_mutex_init(&f->lock);

	return f;
}

void fifo_free(struct fifo_t * f)
{
	if(f)
	{
		xos_mutex_exit(&f->lock);
		xos_mem_free(f->buffer);
		xos_mem_free(f);
	}
}

void fifo_reset(struct fifo_t * f)
{
	xos_mutex_lock(&f->lock);
	__fifo_reset(f);
	xos_mutex_unlock(&f->lock);
}

int fifo_isempty(struct fifo_t * f)
{
	int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_isempty(f);
	xos_mutex_unlock(&f->lock);

	return ret;
}

int fifo_isfull(struct fifo_t * f)
{
	int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_isfull(f);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_size(struct fifo_t * f)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_size(f);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_length(struct fifo_t * f)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_length(f);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_available(struct fifo_t * f)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_available(f);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_put(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_put(f, buf, len);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_get(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_get(f, buf, len);
	if(f->in == f->out)
		f->in = f->out = 0;
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_peek(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_peek(f, buf, len);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_put_packet(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_put_packet(f, buf, len);
	xos_mutex_unlock(&f->lock);

	return ret;
}

unsigned int fifo_get_packet(struct fifo_t * f, unsigned char * buf, unsigned int len)
{
	unsigned int ret;

	xos_mutex_lock(&f->lock);
	ret = __fifo_get_packet(f, buf, len);
	if(f->in == f->out)
		f->in = f->out = 0;
	xos_mutex_unlock(&f->lock);

	return ret;
}
