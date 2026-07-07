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

#include <libx/ps.h>

struct pscb_t {
	struct list_head_t head;
	int oneshot;
	void * sdat;
	void (*cb)(void * pdat, void * sdat);
};

struct pstopic_t {
	struct hlist_node_t node;
	struct list_head_t head;
	char * topic;
	struct list_head_t list;
	struct mutex_t lock;
};

struct psctx_t * ps_alloc(int size)
{
	if(size < 16)
		size = 16;
	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	struct psctx_t * ctx = xos_mem_malloc(sizeof(struct psctx_t));
	if(!ctx)
		return NULL;

	ctx->hash = xos_mem_malloc(sizeof(struct hlist_head_t) * size);
	if(!ctx->hash)
	{
		xos_mem_free(ctx);
		return NULL;
	}
	for(int i = 0; i < size; i++)
		init_hlist_head(&ctx->hash[i]);
	init_list_head(&ctx->list);
	xos_mutex_init(&ctx->lock);
	ctx->size = size;
	ctx->n = 0;

	return ctx;
}

void ps_free(struct psctx_t * ctx)
{
	if(ctx)
	{
		xos_mutex_lock(&ctx->lock);
		{
			struct pstopic_t * tpos, * tn;
			list_for_each_entry_safe(tpos, tn, &ctx->list, head)
			{
				xos_mutex_lock(&tpos->lock);
				{
					struct pscb_t * pos, * n;
					list_for_each_entry_safe(pos, n, &tpos->list, head)
					{
						list_del(&pos->head);
						xos_mem_free(pos);
					}
					list_del(&tpos->head);
					hlist_del(&tpos->node);
				}
				xos_mutex_unlock(&tpos->lock);
				xos_mutex_exit(&tpos->lock);
				xos_mem_free(tpos->topic);
				xos_mem_free(tpos);
			}
		}
		xos_mutex_unlock(&ctx->lock);
		xos_mutex_exit(&ctx->lock);
		xos_mem_free(ctx->hash);
		xos_mem_free(ctx);
	}
}

static struct pstopic_t * ps_search_topic(struct psctx_t * ctx, const char * topic)
{
	if(ctx && topic)
	{
		struct pstopic_t * pos;
		struct hlist_node_t * n;

		hlist_for_each_entry_safe(pos, n, &ctx->hash[shash(topic) & (ctx->size - 1)], node)
		{
			if(xos_strcmp(pos->topic, topic) == 0)
				return pos;
		}
	}
	return NULL;
}

static inline void ps_resize(struct psctx_t * ctx, unsigned int size)
{
	struct pstopic_t * pos, * n;

	if(size < 16)
		size = 16;
	if(size & (size - 1))
		size = roundup_pow_of_two(size);

	struct hlist_head_t * hash = xos_mem_malloc(sizeof(struct hlist_head_t) * size);
	if(!hash)
		return;
	for(int i = 0; i < size; i++)
		init_hlist_head(&hash[i]);

	xos_mem_free(ctx->hash);
	ctx->hash = hash;
	ctx->size = size;

	list_for_each_entry_safe(pos, n, &ctx->list, head)
	{
		init_hlist_node(&pos->node);
		hlist_add_head(&pos->node, &ctx->hash[shash(pos->topic) & (ctx->size - 1)]);
	}
}

static struct pstopic_t * ps_add_topic(struct psctx_t * ctx, const char * topic)
{
	if(ctx && topic)
	{
		xos_mutex_lock(&ctx->lock);
		struct pstopic_t * pstop = ps_search_topic(ctx, topic);
		xos_mutex_unlock(&ctx->lock);

		if(!pstop)
		{
			pstop = xos_mem_malloc(sizeof(struct pstopic_t));
			if(pstop)
			{
				pstop->topic = xos_strdup(topic);
				if(!pstop->topic)
				{
				    xos_mem_free(pstop);
				    return NULL;
				}
				init_list_head(&pstop->list);
				init_list_head(&pstop->head);
				init_hlist_node(&pstop->node);
				xos_mutex_init(&pstop->lock);

				xos_mutex_lock(&ctx->lock);
				{
					struct pstopic_t * top = ps_search_topic(ctx, topic);
					if(top)
					{
						xos_mutex_unlock(&ctx->lock);
						xos_mutex_exit(&pstop->lock);
						xos_mem_free(pstop->topic);
						xos_mem_free(pstop);
						return top;
					}
					if(ctx->n > (ctx->size >> 1))
						ps_resize(ctx, ctx->size << 1);
					hlist_add_head(&pstop->node, &ctx->hash[shash(pstop->topic) & (ctx->size - 1)]);
					list_add_tail(&pstop->head, &ctx->list);
					ctx->n++;
				}
				xos_mutex_unlock(&ctx->lock);
			}
		}
		return pstop;
	}
	return NULL;
}

void ps_publish(struct psctx_t * ctx, const char * topic, void * pdat)
{
	if(ctx && topic)
	{
		xos_mutex_lock(&ctx->lock);
		struct pstopic_t * pstop = ps_search_topic(ctx, topic);
		xos_mutex_unlock(&ctx->lock);

		if(pstop)
		{
			struct list_head_t locallist = { &locallist, &locallist };
			struct list_head_t keeplist = { &keeplist, &keeplist };
			struct list_head_t freelist = { &freelist, &freelist };
			struct pscb_t * pos, * n;

			xos_mutex_lock(&pstop->lock);
			list_splice_init(&pstop->list, &locallist);
			xos_mutex_unlock(&pstop->lock);

			list_for_each_entry_safe(pos, n, &locallist, head)
			{
				if(pos->cb)
					pos->cb(pdat, pos->sdat);
			}

			xos_mutex_lock(&pstop->lock);
			{
				list_for_each_entry_safe(pos, n, &locallist, head)
				{
					list_del(&pos->head);
					if(pos->oneshot)
						list_add_tail(&pos->head, &freelist);
					else
						list_add_tail(&pos->head, &keeplist);
				}
				list_splice(&keeplist, &pstop->list);
			}
			xos_mutex_unlock(&pstop->lock);

			list_for_each_entry_safe(pos, n, &freelist, head)
			{
				list_del(&pos->head);
				xos_mem_free(pos);
			}
		}
	}
}

void ps_subscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat, int oneshot)
{
	if(ctx && topic && cb)
	{
		struct pstopic_t * pstop = ps_add_topic(ctx, topic);
		if(pstop)
		{
			struct pscb_t * pscb = xos_mem_malloc(sizeof(struct pscb_t));
			if(pscb)
			{
				init_list_head(&pscb->head);
				pscb->oneshot = oneshot ? 1 : 0;
				pscb->sdat = sdat;
				pscb->cb = cb;

				xos_mutex_lock(&pstop->lock);
				{
					struct pscb_t * pos, * n;
					list_for_each_entry_safe(pos, n, &pstop->list, head)
					{
						if((pos->cb == cb) && (pos->sdat == sdat))
						{
							xos_mutex_unlock(&pstop->lock);
							xos_mem_free(pscb);
							return;
						}
					}
					list_add_tail(&pscb->head, &pstop->list);
				}
				xos_mutex_unlock(&pstop->lock);
			}
		}
	}
}

void ps_unsubscribe(struct psctx_t * ctx, const char * topic, void (*cb)(void *, void *), void * sdat)
{
	if(ctx && topic)
	{
		xos_mutex_lock(&ctx->lock);
		struct pstopic_t * pstop = ps_search_topic(ctx, topic);
		xos_mutex_unlock(&ctx->lock);

		if(pstop)
		{
			struct list_head_t freelist = { &freelist, &freelist };
			struct pscb_t * pos, * n;

			xos_mutex_lock(&pstop->lock);
			{
				list_for_each_entry_safe(pos, n, &pstop->list, head)
				{
					if((cb == NULL) || ((pos->cb == cb) && (pos->sdat == sdat)))
					{
						list_del(&pos->head);
						list_add_tail(&pos->head, &freelist);
					}
				}
			}
			xos_mutex_unlock(&pstop->lock);

			list_for_each_entry_safe(pos, n, &freelist, head)
			{
				list_del(&pos->head);
				xos_mem_free(pos);
			}
		}
	}
}
