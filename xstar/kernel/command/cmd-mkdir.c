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
	shell_printf("    mkdir -v <DIRECTORY> ...\r\n");
}

static int do_mkdir(int argc, char ** argv)
{
	struct sarg_t sarg;
	char fpath[CONFIG_XSTAR_MAX_PATH];
	char ** v;
	int verbose = 0;
	int c = 0;
	int i;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char * []){ "-v", NULL }, 1, -1))
	{
		usage();
		return -1;
	}
	if(!(v = xos_mem_malloc(sizeof(char *) * argc)))
		return -1;
	if(sarg_has(&sarg, "-v"))
		verbose = 1;
	const char * tok;
	sarg_iter_reset(&sarg);
	while((tok = sarg_iter_next(&sarg)))
		v[c++] = (char *)tok;
	for(i = 0; i < c; i++)
	{
		if(shell_realpath(v[i], fpath) < 0)
			continue;
		if(!xfs_mkdir(shell_getxfs(), fpath))
			shell_printf("mkdir: failed to create directory %s\r\n", fpath);
		else if(verbose)
			shell_printf("mkdir '%s'\r\n", fpath);
	}
	xos_mem_free(v);

	return 0;
}

static struct command_t cmd_mkdir = {
	.name	= "mkdir",
	.desc	= "make directories",
	.usage	= usage,
	.exec	= do_mkdir,
};

static void mkdir_cmd_init(void)
{
	register_command(&cmd_mkdir);
}

static void mkdir_cmd_exit(void)
{
	unregister_command(&cmd_mkdir);
}

command_initcall(mkdir_cmd_init);
command_exitcall(mkdir_cmd_exit);
