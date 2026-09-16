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
#include <kernel/core/logger.h>
#include <kernel/shell/context.h>
#include <kernel/shell/ctrlc.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    dmesg [option]\r\n");
	shell_printf("    -c    clear the log buffer after printing\r\n");
	shell_printf("    -w    wait for new messages and print them, ctrl-c to exit\r\n");
}

static int dmesg_write(const char * buf, int len, void * data)
{
	for(int i = 0; i < len; i++)
		shell_putchar(buf[i]);
	return len;
}

static int do_dmesg(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char * []){ "-c", "-w", NULL }, 0, -1))
	{
		usage();
		return -1;
	}
	if(sarg_has(&sarg, "-w"))
	{
		int pos = 0;
		while(1)
		{
			logger_dump(&pos, dmesg_write, NULL);
			if(shell_ctrlc())
				break;
			if(xstar_feature_thread())
				xos_thread_msleep(50);
			else
				mdelay(50);
		}
	}
	else
	{
		logger_dump(NULL, dmesg_write, NULL);
		if(sarg_has(&sarg, "-c"))
			logger_clear();
	}
	return 0;
}

static struct command_t cmd_dmesg = {
	.name	= "dmesg",
	.desc	= "print the message log buffer",
	.usage	= usage,
	.exec	= do_dmesg,
};

static void dmesg_cmd_init(void)
{
	register_command(&cmd_dmesg);
}

static void dmesg_cmd_exit(void)
{
	unregister_command(&cmd_dmesg);
}

command_initcall(dmesg_cmd_init);
command_exitcall(dmesg_cmd_exit);
