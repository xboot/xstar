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
#include <kernel/command/command.h>
#include <kernel/shell/context.h>

struct shell_history_t {
	struct list_head_t head;
	uint32_t * ucs4;
};

struct shell_context_t {
	struct xfs_context_t * xfs;
	struct mutex_t cwdlock;
	struct ds_t * cwd;
	struct list_head_t history_list;
	struct list_head_t * history_current;
	ssize_t (*read)(void * buf, size_t count);
	ssize_t (*write)(void * buf, size_t count);
};

static ssize_t shell_stdio_read(void * buf, size_t count)
{
	return xos_stdio_read(buf, count);
}

static ssize_t shell_stdio_write(void * buf, size_t count)
{
	return xos_stdio_write(buf, count);
}

static struct shell_context_t * shell_context_get(void)
{
	static struct shell_context_t * ctx = NULL;

	if(!ctx)
	{
		ctx = xos_mem_malloc(sizeof(struct shell_context_t));
		if(ctx)
		{
			ctx->xfs = xfs_alloc();
			xos_mutex_init(&ctx->cwdlock);
			ctx->cwd = ds_alloc_from_str("/");
			init_list_head(&ctx->history_list);
			ctx->history_current = &ctx->history_list;
			ctx->read = shell_stdio_read;
			ctx->write = shell_stdio_write;
		}
	}
	return ctx;
}

struct xfs_context_t * shell_getxfs(void)
{
	struct shell_context_t * ctx = shell_context_get();
	return ctx->xfs;
}

int shell_realpath(const char * path, char * fpath)
{
	struct shell_context_t * ctx = shell_context_get();
	char * p, * q, * s;
	int left_len, full_len;
	char left[CONFIG_XSTAR_MAX_PATH];
	char next_token[CONFIG_XSTAR_MAX_PATH];

	if(path[0] == '/')
	{
		fpath[0] = '/';
		fpath[1] = '\0';
		if(path[1] == '\0')
			return 0;
		full_len = 1;
		left_len = xos_strlcpy(left, (const char *)(path + 1), sizeof(left));
	}
	else
	{
		xos_mutex_lock(&ctx->cwdlock);
		xos_strlcpy(fpath, ds_cstr(ctx->cwd), CONFIG_XSTAR_MAX_PATH);
		xos_mutex_unlock(&ctx->cwdlock);
		full_len = xos_strlen(fpath);
		left_len = xos_strlcpy(left, path, sizeof(left));
	}
	if((left_len >= sizeof(left)) || (full_len >= CONFIG_XSTAR_MAX_PATH))
		return -1;
	while(left_len != 0)
	{
		p = xos_strchr(left, '/');
		s = p ? p : left + left_len;
		if((int)(s - left) >= sizeof(next_token))
			return -1;
		xos_memcpy(next_token, left, s - left);
		next_token[s - left] = '\0';
		left_len -= s - left;
		if(p != NULL)
		{
			xos_memmove(left, s + 1, left_len + 1);
		}
		if(fpath[full_len - 1] != '/')
		{
			if (full_len + 1 >= CONFIG_XSTAR_MAX_PATH)
				return -1;

			fpath[full_len++] = '/';
			fpath[full_len] = '\0';
		}
		if(next_token[0] == '\0' || xos_strcmp(next_token, ".") == 0)
		{
			continue;
		}
		else if(xos_strcmp(next_token, "..") == 0)
		{
			if(full_len > 1)
			{
				fpath[full_len - 1] = '\0';
				q = xos_strrchr(fpath, '/') + 1;
				*q = '\0';
				full_len = q - fpath;
			}
			continue;
		}
		full_len = xos_strlcat(fpath, next_token, CONFIG_XSTAR_MAX_PATH);
		if(full_len >= CONFIG_XSTAR_MAX_PATH)
			return -1;
	}
	if((full_len > 1) && (fpath[full_len - 1] == '/'))
		fpath[full_len - 1] = '\0';
	return 0;
}

const char * shell_getcwd(void)
{
	struct shell_context_t * ctx = shell_context_get();
	return ds_cstr(ctx->cwd);
}

int shell_setcwd(const char * path)
{
	struct shell_context_t * ctx = shell_context_get();
	char fpath[CONFIG_XSTAR_MAX_PATH];

	if(shell_realpath(path, fpath) < 0)
		return -1;
	if(xfs_isdir(ctx->xfs, fpath))
	{
		xos_mutex_lock(&ctx->cwdlock);
		ds_clear(ctx->cwd);
		ds_append(ctx->cwd, fpath);
		xos_mutex_unlock(&ctx->cwdlock);
		return 0;
	}
	return -1;
}

uint32_t * shell_history_prev(void)
{
	struct shell_context_t * ctx = shell_context_get();

	if(list_empty_careful(&ctx->history_list))
	{
		ctx->history_current = &ctx->history_list;
		return NULL;
	}

	struct list_head_t * list = ctx->history_current->prev;
	if(list != &ctx->history_list)
	{
		ctx->history_current = list;
		return ((struct shell_history_t *)list_entry(list, struct shell_history_t, head))->ucs4;
	}
	return NULL;
}

uint32_t * shell_history_next(void)
{
	struct shell_context_t * ctx = shell_context_get();

	if(list_empty_careful(&ctx->history_list))
	{
		ctx->history_current = &ctx->history_list;
		return NULL;
	}
	if(ctx->history_current == &ctx->history_list)
		return NULL;

	struct list_head_t * list = ctx->history_current->next;
	ctx->history_current = list;
	if(list != &ctx->history_list)
		return ((struct shell_history_t *)list_entry(list, struct shell_history_t, head))->ucs4;
	return NULL;
}

void shell_history_add(uint32_t * ucs4, int len)
{
	struct shell_context_t * ctx = shell_context_get();

	ctx->history_current = &ctx->history_list;
	if(!ucs4 || (len <= 0))
		return;

	if(!list_empty_careful(&ctx->history_list))
	{
		struct shell_history_t * entry = list_last_entry(&ctx->history_list, struct shell_history_t, head);
		uint32_t * sc = entry->ucs4;
		for(sc = entry->ucs4; *sc != '\0'; ++sc);
		if(sc - entry->ucs4 == len)
		{
			if(xos_memcmp(entry->ucs4, ucs4, len * sizeof(uint32_t)) == 0)
				return;
		}
	}

	if(!list_empty_careful(&ctx->history_list))
	{
		struct shell_history_t * pos;
		int count = 0;
		list_for_each_entry(pos, &ctx->history_list, head)
		{
			count++;
		}
		if(count >= 32)
		{
			struct shell_history_t * entry = (struct shell_history_t *)list_first_entry_or_null(&ctx->history_list, struct shell_history_t, head);
			if(entry)
			{
				list_del(&entry->head);
				xos_mem_free(entry->ucs4);
				xos_mem_free(entry);
			}
			ctx->history_current = &ctx->history_list;
		}
	}

	struct shell_history_t * entry = xos_mem_malloc(sizeof(struct shell_history_t));
	if(!entry)
		return;

	entry->ucs4 = xos_mem_malloc((len + 1) * sizeof(uint32_t));
	if(!entry->ucs4)
	{
		xos_mem_free(entry);
		return;
	}
	for(int i = 0; i < len; i++)
		entry->ucs4[i] = ucs4[i];
	entry->ucs4[len] = '\0';
	list_add_tail(&entry->head, &ctx->history_list);
}

int shell_getchar(void)
{
	struct shell_context_t * ctx = shell_context_get();
	unsigned char ch;

	if(ctx->read && (ctx->read(&ch, 1) == 1))
		return (int)ch;
	return -1;
}

int shell_putchar(int c)
{
	struct shell_context_t * ctx = shell_context_get();
	unsigned char ch = c & 0xff;

	if(ctx->write && (ctx->write(&ch, 1) == 1))
		return ch;
	return -1;
}

int shell_vprintf(const char * fmt, va_list ap)
{
	struct shell_context_t * ctx = shell_context_get();
	char * p = NULL;
	int len;

	len = xos_vasprintf(&p, fmt, ap);
	if(p && (len > 0))
	{
		if(ctx->write)
			ctx->write(p, len);
		xos_mem_free(p);
	}
	return len;
}

int shell_printf(const char * fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = shell_vprintf(fmt, ap);
	va_end(ap);

	return len;
}
