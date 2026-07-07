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
	shell_printf("    go address [args ...]\r\n");
}

static int do_go(int argc, char ** argv)
{
	if(argc < 2)
	{
		usage();
		return -1;
	}

	void * addr = (void *)((io_addr_t)xos_strtoul(argv[1], NULL, 0));
	shell_printf("Starting application at %p ...\r\n", addr);
	int ret = ((int(*)(int, char **))((void *)addr))(--argc, &argv[1]);
	shell_printf("Application terminated, (ret = %d)\r\n", ret);

	return ret;
}

static struct command_t cmd_go = {
	.name	= "go",
	.desc	= "start application at address",
	.usage	= usage,
	.exec	= do_go,
};

static void go_cmd_init(void)
{
	register_command(&cmd_go);
}

static void go_cmd_exit(void)
{
	unregister_command(&cmd_go);
}

command_initcall(go_cmd_init);
command_exitcall(go_cmd_exit);
