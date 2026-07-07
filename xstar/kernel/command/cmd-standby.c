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
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    standby\r\n");
}

static int do_standby(int argc, char ** argv)
{
	xstar_standby();
	return 0;
}

static struct command_t cmd_standby = {
	.name	= "standby",
	.desc	= "let's system enter standby mode",
	.usage	= usage,
	.exec	= do_standby,
};

static void standby_cmd_init(void)
{
	register_command(&cmd_standby);
}

static void standby_cmd_exit(void)
{
	unregister_command(&cmd_standby);
}

command_initcall(standby_cmd_init);
command_exitcall(standby_cmd_exit);
