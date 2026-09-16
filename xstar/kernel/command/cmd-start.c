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
#include <kernel/shell/context.h>
#include <kernel/shell/shell.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    start <command> [args...]\r\n");
	shell_printf("    start delay 1000               run a command in background\r\n");
	shell_printf("    start \"delay 1; echo done\"     chain commands, quoted as one line\r\n");
}

static void start_thread_func(void * data)
{
	struct ds_t * ds = (struct ds_t *)data;
	if(ds)
	{
		shell_system(ds_cstr(ds));
		ds_free(ds);
	}
}

static int do_start(int argc, char ** argv)
{
	if(!xstar_feature_thread())
	{
		shell_printf("start: thread is not supported on this platform\r\n");
		return -1;
	}

	if(argc < 2)
	{
		usage();
		return -1;
	}
	struct ds_t * ds = ds_alloc();
	if(!ds)
		return -1;
	for(int i = 1; i < argc; i++)
	{
		ds_append(ds, argv[i]);
		if(i < (argc - 1))
			ds_append_char(ds, ' ');
	}
	if(!xos_thread_create("start", start_thread_func, ds, 0))
	{
		shell_printf("start: failed to create thread\r\n");
		ds_free(ds);
		return -1;
	}
	return 0;
}

static struct command_t cmd_start = {
	.name	= "start",
	.desc	= "run a command in a background thread",
	.usage	= usage,
	.exec	= do_start,
};

static void start_cmd_init(void)
{
	register_command(&cmd_start);
}

static void start_cmd_exit(void)
{
	unregister_command(&cmd_start);
}

command_initcall(start_cmd_init);
command_exitcall(start_cmd_exit);
