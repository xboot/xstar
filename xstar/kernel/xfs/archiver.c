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

#include <kernel/xfs/archiver.h>

static struct list_head_t __archiver_list = {
	.next = &__archiver_list,
	.prev = &__archiver_list,
};
static struct mutex_t __archiver_lock;

int register_archiver(struct xfs_archiver_t * archiver)
{
	if(!archiver || !archiver->name)
		return FALSE;

	xos_mutex_lock(&__archiver_lock);
	init_list_head(&archiver->list);
	list_add_tail(&archiver->list, &__archiver_list);
	xos_mutex_unlock(&__archiver_lock);

	return TRUE;
}

int unregister_archiver(struct xfs_archiver_t * archiver)
{
	if(!archiver || !archiver->name)
		return FALSE;

	xos_mutex_lock(&__archiver_lock);
	list_del(&archiver->list);
	xos_mutex_unlock(&__archiver_lock);

	return TRUE;
}

void * mount_archiver(const char * path, struct xfs_archiver_t ** archiver, int * writable)
{
	struct xfs_archiver_t * pos, * n;

	list_for_each_entry_safe(pos, n, &__archiver_list, list)
	{
		void * m = pos->mount(path, writable);
		if(m)
		{
			if(archiver)
				*archiver = pos;
			return m;
		}
	}
	return NULL;
}

static void archiver_pure_init(void)
{
	xos_mutex_init(&__archiver_lock);
}

static void archiver_pure_exit(void)
{
	xos_mutex_exit(&__archiver_lock);
}

pure_initcall(archiver_pure_init);
pure_exitcall(archiver_pure_exit);
