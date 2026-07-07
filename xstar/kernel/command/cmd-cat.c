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

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    cat <file> ...\r\n");
}

static int cat_file(const char * filename)
{
	struct xfs_file_t * file;
	char fpath[CONFIG_XSTAR_MAX_PATH];
	char * buf;
	int64_t i, n;

	if(shell_realpath(filename, fpath) < 0)
	{
		shell_printf("cat: %s: Can not convert to realpath\r\n", filename);
		return -1;
	}
	if(xfs_isdir(shell_getxfs(), fpath))
	{
		shell_printf("cat: %s: Is a directory\r\n", fpath);
		return -1;
	}
	if(!xfs_isfile(shell_getxfs(), fpath))
	{
		shell_printf("cat: %s: No such file\r\n", fpath);
		return -1;
	}
	file = xfs_open_read(shell_getxfs(), fpath);
	if(!file)
	{
		shell_printf("cat: %s: Can not open\r\n", fpath);
		return -1;
	}
	buf = xos_mem_malloc(64 * 1024);
	if(!buf)
	{
		shell_printf("cat: Can not alloc memory\r\n");
		xfs_close(file);
		return -1;
	}
	while((n = xfs_read(file, buf, 64 * 1024)) > 0)
	{
		for(i = 0; i < n; i++)
		{
			if(xos_isprint(buf[i]) || (buf[i] == '\r') || (buf[i] == '\n') || (buf[i] == '\t') || (buf[i] == '\f'))
				shell_putchar(buf[i]);
			else
				shell_putchar('.');
		}
	}
	shell_printf("\r\n");
	xos_mem_free(buf);
	xfs_close(file);
	return 0;
}

static int do_cat(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 1, -1))
	{
		usage();
		return -1;
	}
	const char * f;
	sarg_iter_reset(&sarg);
	while((f = sarg_iter_next(&sarg)) != NULL)
	{
		if(cat_file(f) != 0)
			return -1;
	}
	return 0;
}

static struct command_t cmd_cat = {
	.name	= "cat",
	.desc	= "show the contents of a file",
	.usage	= usage,
	.exec	= do_cat,
};

static void cat_cmd_init(void)
{
	register_command(&cmd_cat);
}

static void cat_cmd_exit(void)
{
	unregister_command(&cmd_cat);
}

command_initcall(cat_cmd_init);
command_exitcall(cat_cmd_exit);
