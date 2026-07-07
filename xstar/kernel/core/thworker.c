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
#include <kernel/core/thworker.h>

struct thworker_task_t {
	struct list_head_t entry;
	void (*func)(void *);
	void * data;
};

static void thwoker_thread(void * data)
{
	struct thworker_t * w = (struct thworker_t *)data;

	while(w->running)
	{
		if(xos_semaphore_wait(&w->sem, 0))
		{
			xos_mutex_lock(&w->lock);
			struct thworker_task_t * task = (struct thworker_task_t *)list_first_entry_or_null(&w->head, struct thworker_task_t, entry);
			if(task)
				list_del(&task->entry);
			xos_mutex_unlock(&w->lock);
			if(task)
			{
				if(task->func)
					task->func(task->data);
				xos_mem_free(task);
			}
		}
	}
}

struct thworker_t * thworker_alloc(const char * name)
{
	if(xstar_feature_thread())
	{
		struct thworker_t * w = xos_mem_malloc(sizeof(struct thworker_t));
		if(w)
		{
			init_list_head(&w->head);
			xos_mutex_init(&w->lock);
			xos_semaphore_init(&w->sem, 0);
			w->running = 1;
			w->thread = xos_thread_create(name, thwoker_thread, w, 0);
			return w;
		}
	}
	return NULL;
}

void thworker_free(struct thworker_t * w)
{
	if(w)
	{
		thworker_wait(w);
		xos_mutex_lock(&w->lock);
		w->running = 0;
		xos_mutex_unlock(&w->lock);
		xos_semaphore_post(&w->sem);
		xos_thread_wait(w->thread);
		xos_thread_destroy(w->thread);
		xos_semaphore_exit(&w->sem);
		xos_mutex_exit(&w->lock);
		xos_mem_free(w);
	}
}

void thworker_wait(struct thworker_t * w)
{
	if(w)
	{
		while(1)
		{
			xos_mutex_lock(&w->lock);
			struct thworker_task_t * task = (struct thworker_task_t *)list_first_entry_or_null(&w->head, struct thworker_task_t, entry);
			xos_mutex_unlock(&w->lock);
			if(!task)
				break;
			xos_thread_msleep(1);
		}
	}
}

void thworker_clear(struct thworker_t * w, void (*cb)(void (*func)(void *), void * data))
{
	if(w)
	{
		struct thworker_task_t * pos, * n;
		xos_mutex_lock(&w->lock);
		list_for_each_entry_safe(pos, n, &w->head, entry)
		{
			list_del(&pos->entry);
			if(cb)
				cb(pos->func, pos->data);
			xos_mem_free(pos);
		}
		xos_mutex_unlock(&w->lock);
		thworker_wait(w);
	}
}

void thworker_submit(struct thworker_t * w, void (*func)(void *), void * data)
{
	if(w && func)
	{
		struct thworker_task_t * task = xos_mem_malloc(sizeof(struct thworker_task_t));
		if(task)
		{
			init_list_head(&task->entry);
			task->func = func;
			task->data = data;
			xos_mutex_lock(&w->lock);
			list_add_tail(&task->entry, &w->head);
			xos_mutex_unlock(&w->lock);
			xos_semaphore_post(&w->sem);
		}
	}
}
