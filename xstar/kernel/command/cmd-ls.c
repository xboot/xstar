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

#include <kernel/shell/context.h>
#include <kernel/command/command.h>

enum {
	LSFLAG_DOT	= (1 << 0),
	LSFLAG_LONG	= (1 << 1),
};
static const char rwx[8][4] = { "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx" };

static void ls_cb_all(const char * dir, const char * name, void * data)
{
	struct slist_t * sl = (struct slist_t *)data;
	slist_add(sl, NULL, "%s", name);
}

static void ls_cb(const char * dir, const char * name, void * data)
{
	struct slist_t * sl = (struct slist_t *)data;
	if(name && (name[0] != '.'))
		slist_add(sl, NULL, "%s", name);
}

static void ls(const char * path, int flag)
{
	struct slist_t * sl, * e;
	int isdir;

	sl = slist_alloc();
	if(xfs_isdir(shell_getxfs(), path))
	{
		if(flag & LSFLAG_DOT)
			xfs_walk(shell_getxfs(), path, ls_cb_all, sl);
		else
			xfs_walk(shell_getxfs(), path, ls_cb, sl);
		isdir = 1;
	}
	else if(xfs_isfile(shell_getxfs(), path))
	{
		slist_add(sl, NULL, "%s", path);
		isdir = 0;
	}
	else
	{
		shell_printf("ls: cannot access %s: No such file or directory\r\n", path);
		slist_free(sl);
		return;
	}
	slist_natsort(sl);

	if(flag & LSFLAG_LONG)
	{
		char buf[CONFIG_XSTAR_MAX_PATH + 16];

		slist_for_each_entry(e, sl)
		{
			if(isdir)
				xos_snprintf(buf, sizeof(buf), "%s/%s", path, e->key);
			else
				xos_snprintf(buf, sizeof(buf), "%s", e->key);
			int mode = xfs_mode(shell_getxfs(), buf);
			int64_t len = 0;
			if(xfs_isdir(shell_getxfs(), buf))
			{
				shell_printf("d");
			}
			else if(xfs_isfile(shell_getxfs(), buf))
			{
				struct xfs_file_t * file = xfs_open_read(shell_getxfs(), buf);
				if(file)
				{
					len = xfs_length(file);
					xfs_close(file);
				}
				shell_printf("-");
			}
			else
			{
				shell_printf(" ");
			}
			shell_printf("%s%s%s", rwx[(mode >> 6) & 0x7], rwx[(mode >> 3) & 0x7], rwx[(mode >> 0) & 0x7]);
			shell_printf(" %8lld %s\r\n", len, e->key);
		}
	}
	else
	{
		int len = 0, n, i, l;

		slist_for_each_entry(e, sl)
		{
			l = xos_strlen(e->key) + 4;
			if(l > len)
				len = l;
		}
		if(!len)
			return;
		n = 80 / (len + 1);
		if(n == 0)
			n = 1;
		i = 0;
		slist_for_each_entry(e, sl)
		{
			if(!(++i % n))
				shell_printf("%s\r\n", e->key);
			else
				shell_printf("%-*s", len, e->key);
		}
		if(i % n)
			shell_printf("\r\n");
	}
	slist_free(sl);
}

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    ls [-l] [-a] [FILE]...\r\n");
}

static int do_ls(int argc, char ** argv)
{
	struct sarg_t sarg;
	char fpath[CONFIG_XSTAR_MAX_PATH];
	char ** v;
	int flag = 0;
	int c = 0;
	int i;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char * []){ "-l", "-a", NULL }, 0, -1))
	{
		usage();
		return -1;
	}
	if(!(v = xos_mem_malloc(sizeof(char *) * argc)))
		return -1;
	if(sarg_has(&sarg, "-l"))
		flag |= LSFLAG_LONG;
	if(sarg_has(&sarg, "-a"))
		flag |= LSFLAG_DOT;
	const char * tok;
	sarg_iter_reset(&sarg);
	while((tok = sarg_iter_next(&sarg)))
		v[c++] = (char *)tok;
	if(c == 0)
		v[c++] = ".";
	for(i = 0; i < c; i++)
	{
		if(shell_realpath(v[i], fpath) >= 0)
		{
			ls(fpath, flag);
		}
	}
	xos_mem_free(v);

	return 0;
}

static struct command_t cmd_ls = {
	.name	= "ls",
	.desc	= "list directory contents",
	.usage	= usage,
	.exec	= do_ls,
};

static void ls_cmd_init(void)
{
	register_command(&cmd_ls);
}

static void ls_cmd_exit(void)
{
	unregister_command(&cmd_ls);
}

command_initcall(ls_cmd_init);
command_exitcall(ls_cmd_exit);
