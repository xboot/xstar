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
#include <kernel/core/thmailbox.h>

#define THMAILBOX_BUFHDR_MAGIC	(0x4d425831)

struct bufhdr_t {
	union {
		struct bufhdr_t * next;
		uint32_t magic;
	};
	uint32_t len;
	int refcnt;
	struct thmailbox_t * mbox;
};

static inline struct bufhdr_t * bufhdr_of(void * buf)
{
	return &((struct bufhdr_t *)buf)[-1];
}

struct thmailbox_t * thmailbox_alloc(unsigned int size, unsigned int count)
{
	if(!xstar_feature_thread())
		return NULL;

	if(size == 0)
		size = 16;
	if(count == 0)
		count = 1;
	else if(count > (1U << 20))
		count = (1U << 20);

	struct thmailbox_t * m = xos_mem_malloc(sizeof(struct thmailbox_t));
	if(!m)
		return NULL;

	m->block_size = size;
	m->block_count = count;
	m->ring_size = roundup_pow_of_two(count);

	uint64_t pool_bytes = ((uint64_t)sizeof(struct bufhdr_t) + size) * count;
	uint64_t ring_bytes = (uint64_t)sizeof(void *) * m->ring_size;
	if((pool_bytes > (uint64_t)~(size_t)0) || (ring_bytes > (uint64_t)~(size_t)0))
	{
		xos_mem_free(m);
		return NULL;
	}

	m->pool = xos_mem_malloc((size_t)pool_bytes);
	m->ring = xos_mem_malloc((size_t)ring_bytes);
	if(!m->pool || !m->ring)
	{
		if(m->pool)
			xos_mem_free(m->pool);
		if(m->ring)
			xos_mem_free(m->ring);
		xos_mem_free(m);
		return NULL;
	}

	m->freelist = NULL;
	m->freecount = count;
	for(unsigned int i = 0; i < count; i++)
	{
		struct bufhdr_t * h = (struct bufhdr_t *)(m->pool + (sizeof(struct bufhdr_t) + size) * i);
		h->next = (struct bufhdr_t *)m->freelist;
		m->freelist = h;
	}
	m->in = 0;
	m->out = 0;
	xos_mutex_init(&m->lock);
	xos_semaphore_init(&m->ssem, 0);
	xos_semaphore_init(&m->rsem, 0);
	xos_semaphore_init(&m->psem, 0);

	return m;
}

void thmailbox_free(struct thmailbox_t * m)
{
	if(m)
	{
		xos_mutex_exit(&m->lock);
		xos_semaphore_exit(&m->ssem);
		xos_semaphore_exit(&m->rsem);
		xos_semaphore_exit(&m->psem);
		xos_mem_free(m->ring);
		xos_mem_free(m->pool);
		xos_mem_free(m);
	}
}

void thmailbox_reset(struct thmailbox_t * m)
{
	if(m)
	{
		xos_mutex_lock(&m->lock);
		{
			while(m->in != m->out)
			{
				void * buf = m->ring[m->out & (m->ring_size - 1)];
				m->out++;
				xos_mutex_unlock(&m->lock);
				thmailbox_buf_free(m, buf);
				xos_mutex_lock(&m->lock);
			}
		}
		xos_mutex_unlock(&m->lock);
		while(xos_semaphore_wait(&m->ssem, 0));
		while(xos_semaphore_wait(&m->rsem, 0));
		while(xos_semaphore_wait(&m->psem, 0));
	}
}

void * thmailbox_buf_alloc(struct thmailbox_t * m, unsigned int len, int timeout)
{
	void * ret = NULL;

	if(m && (len <= m->block_size))
	{
		ktime_t deadline = ktime_add_ms(ktime_get(), (timeout > 0) ? (uint64_t)timeout : 0);
		xos_mutex_lock(&m->lock);
		{
			struct bufhdr_t * h = (struct bufhdr_t *)m->freelist;
			while(!h)
			{
				xos_mutex_unlock(&m->lock);
				if(!xos_semaphore_wait(&m->psem, (timeout > 0) ? XMAX((int)ktime_ms_delta(deadline, ktime_get()), 0) : timeout))
					return NULL;
				xos_mutex_lock(&m->lock);
				h = (struct bufhdr_t *)m->freelist;
			}
			m->freelist = h->next;
			m->freecount--;
			h->magic = THMAILBOX_BUFHDR_MAGIC;
			h->len = 0;
			h->refcnt = 1;
			h->mbox = m;
			ret = (void *)(h + 1);
		}
		xos_mutex_unlock(&m->lock);
	}
	return ret;
}

void thmailbox_buf_free(struct thmailbox_t * m, void * buf)
{
	struct bufhdr_t * h = buf ? bufhdr_of(buf) : NULL;

	if(m && h && (h->magic == THMAILBOX_BUFHDR_MAGIC) && (h->mbox == m))
	{
		int recycled = 0;
		xos_mutex_lock(&m->lock);
		{
			if(h->refcnt > 0)
				h->refcnt--;
			if(h->refcnt == 0)
			{
				h->next = (struct bufhdr_t *)m->freelist;
				m->freelist = h;
				m->freecount++;
				recycled = 1;
			}
		}
		xos_mutex_unlock(&m->lock);
		if(recycled)
			xos_semaphore_post(&m->psem);
	}
}

unsigned int thmailbox_buf_available(struct thmailbox_t * m)
{
	unsigned int ret = 0;

	if(m)
	{
		xos_mutex_lock(&m->lock);
		{
			ret = m->freecount;
		}
		xos_mutex_unlock(&m->lock);
	}
	return ret;
}

unsigned int thmailbox_send(struct thmailbox_t * m, void * buf, unsigned int len, int timeout)
{
	if(m && buf && (len > 0) && (len <= m->block_size))
	{
		struct bufhdr_t * h = bufhdr_of(buf);
		if((h->magic == THMAILBOX_BUFHDR_MAGIC) && (h->mbox == m))
		{
			ktime_t deadline = ktime_add_ms(ktime_get(), (timeout > 0) ? (uint64_t)timeout : 0);
			xos_mutex_lock(&m->lock);
			{
				while(m->in - m->out == m->ring_size)
				{
					xos_mutex_unlock(&m->lock);
					if(!xos_semaphore_wait(&m->ssem, (timeout > 0) ? XMAX((int)ktime_ms_delta(deadline, ktime_get()), 0) : timeout))
						return 0;
					xos_mutex_lock(&m->lock);
				}
				h->len = len;
				m->ring[m->in & (m->ring_size - 1)] = buf;
				m->in++;
			}
			xos_mutex_unlock(&m->lock);
			xos_semaphore_post(&m->rsem);
			return len;
		}
	}
	return 0;
}

unsigned int thmailbox_recv(struct thmailbox_t * m, void ** pbuf, int timeout)
{
	unsigned int len = 0;

	if(m && pbuf)
	{
		ktime_t deadline = ktime_add_ms(ktime_get(), (timeout > 0) ? (uint64_t)timeout : 0);
		void * buf = NULL;
		xos_mutex_lock(&m->lock);
		{
			while(m->in == m->out)
			{
				xos_mutex_unlock(&m->lock);
				if(!xos_semaphore_wait(&m->rsem, (timeout > 0) ? XMAX((int)ktime_ms_delta(deadline, ktime_get()), 0) : timeout))
					return 0;
				xos_mutex_lock(&m->lock);
			}
			buf = m->ring[m->out & (m->ring_size - 1)];
			len = bufhdr_of(buf)->len;
			m->out++;
		}
		xos_mutex_unlock(&m->lock);
		xos_semaphore_post(&m->ssem);
		*pbuf = buf;
	}
	return len;
}
