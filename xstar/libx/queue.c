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
#include <libx/queue.h>

struct queue_t * queue_alloc(void)
{
	struct queue_t * q = xos_mem_malloc(sizeof(struct queue_t));

	if(q)
	{
		init_list_head(&q->head);
		return q;
	}
	return NULL;
}

void queue_free(struct queue_t * q, void (*cb)(void *))
{
	if(q)
	{
		queue_clear(q, cb);
		xos_mem_free(q);
	}
}

void queue_clear(struct queue_t * q, void (*cb)(void *))
{
	if(q)
	{
		struct queue_node_t * pos, * n;
		list_for_each_entry_safe(pos, n, &q->head, entry)
		{
			list_del(&pos->entry);
			if(cb)
				cb(pos->data);
			xos_mem_free(pos);
		}
	}
}

int queue_isempty(struct queue_t * q)
{
	if(q)
		return list_empty(&q->head) ? 1 : 0;
	return 1;
}

void queue_push(struct queue_t * q, void * data)
{
	if(q)
	{
		struct queue_node_t * node = xos_mem_malloc(sizeof(struct queue_node_t));
		if(node)
		{
			node->data = data;
			list_add_tail(&node->entry, &q->head);
		}
	}
}

void * queue_pop(struct queue_t * q)
{
	if(q)
	{
		if(!list_empty(&q->head))
		{
			struct list_head_t * pos = q->head.next;
			struct queue_node_t * node = list_entry(pos, struct queue_node_t, entry);
			void * data = node->data;
			list_del(pos);
			xos_mem_free(node);
			return data;
		}
	}
	return NULL;
}

void * queue_peek(struct queue_t * q)
{
	if(q)
	{
		if(!list_empty(&q->head))
		{
			struct list_head_t * pos = q->head.next;
			struct queue_node_t * node = list_entry(pos, struct queue_node_t, entry);
			return node->data;
		}
	}
	return NULL;
}
