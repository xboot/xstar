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

#include <kernel/command/command.h>

static struct list_head_t __command_list = {
	.next = &__command_list,
	.prev = &__command_list,
};
static struct mutex_t __command_lock;

struct command_t * search_command(const char * name)
{
	if(name)
	{
		xos_mutex_lock(&__command_lock);
		{
			struct command_t * pos, * n;
			list_for_each_entry_safe(pos, n, &__command_list, list)
			{
				if(xos_strcmp(pos->name, name) == 0)
				{
					xos_mutex_unlock(&__command_lock);
					return pos;
				}
			}
		}
		xos_mutex_unlock(&__command_lock);
	}
	return NULL;
}

int register_command(struct command_t * cmd)
{
	if(cmd && cmd->name && cmd->desc && cmd->usage && cmd->exec)
	{
		xos_mutex_lock(&__command_lock);
		{
			struct command_t * pos, * n;
			list_for_each_entry_safe(pos, n, &__command_list, list)
			{
				if(xos_strcmp(pos->name, cmd->name) == 0)
				{
					xos_mutex_unlock(&__command_lock);
					return FALSE;
				}
			}
			list_add_tail(&cmd->list, &__command_list);
		}
		xos_mutex_unlock(&__command_lock);
		return TRUE;
	}
	return FALSE;
}

int unregister_command(struct command_t * cmd)
{
	if(cmd)
	{
		xos_mutex_lock(&__command_lock);
		{
			list_del_init(&cmd->list);
		}
		xos_mutex_unlock(&__command_lock);
		return TRUE;
	}
	return FALSE;
}

void command_foreach(void (*cb)(struct command_t * cmd, void * data), void * data)
{
	if(cb)
	{
		xos_mutex_lock(&__command_lock);
		{
			struct command_t * pos, * n;
			list_for_each_entry_safe(pos, n, &__command_list, list)
			{
				cb(pos, data);
			}
		}
		xos_mutex_unlock(&__command_lock);
	}
}

static void command_pure_init(void)
{
	xos_mutex_init(&__command_lock);
}

static void command_pure_exit(void)
{
	xos_mutex_exit(&__command_lock);
}

pure_initcall(command_pure_init);
pure_exitcall(command_pure_exit);
