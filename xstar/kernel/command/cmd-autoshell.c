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

#include <driver/clocksource/clocksource.h>
#include <kernel/shell/context.h>
#include <kernel/shell/shell.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    autoshell [millisecond]\r\n");
}

static int do_autoshell(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, 1))
	{
		usage();
		return -1;
	}
	const char * p0 = sarg_at(&sarg, 0);
	int delay = p0 ? xos_strtol(p0, NULL, 0) : 1000;

	do {
		if(shell_getchar() >= 0)
		{
			shell_printf("\r\n");
			return shell_system("shell");
		}
		ktime_t timeout = ktime_add_ms(ktime_get(), 10);
		do {
			xos_thread_msleep(1);
		} while(ktime_before(ktime_get(), timeout));
		delay -= 10;
		if(delay < 0)
			delay = 0;
		shell_printf("\rPress any key to enter shell mode:%3d.%03d%s", delay / 1000, delay % 1000, (delay > 0) ? "" : "\r\n");
	} while(delay > 0);

	return 0;
}

static struct command_t cmd_autoshell = {
	.name	= "autoshell",
	.desc	= "wait and enter shell mode",
	.usage	= usage,
	.exec	= do_autoshell,
};

static void autoshell_cmd_init(void)
{
	register_command(&cmd_autoshell);
}

static void autoshell_cmd_exit(void)
{
	unregister_command(&cmd_autoshell);
}

command_initcall(autoshell_cmd_init);
command_exitcall(autoshell_cmd_exit);
