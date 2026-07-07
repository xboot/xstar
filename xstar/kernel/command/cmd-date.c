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
#include <kernel/time/wallclock.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    date [FORMAT]\r\n");
}

static int do_date(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, 1))
	{
		usage();
		return -1;
	}
	struct wallclock_time_t tm;
	const char * tz = sarg_at(&sarg, 0);
	if(!tz)
		tz = "Asia/Shanghai";
	wallclock_gettime(&tm, tz);
	shell_printf("%04u-%02u-%02u %02u:%02u:%02u %01u\r\n", (uint32_t)tm.year, (uint32_t)tm.month, (uint32_t)tm.day, (uint32_t)tm.hour, (uint32_t)tm.minute, (uint32_t)tm.second, (uint32_t)tm.week);

	return 0;
}

static struct command_t cmd_date = {
	.name	= "date",
	.desc	= "print the system date and time",
	.usage	= usage,
	.exec	= do_date,
};

static void date_cmd_init(void)
{
	register_command(&cmd_date);
}

static void date_cmd_exit(void)
{
	unregister_command(&cmd_date);
}

command_initcall(date_cmd_init);
command_exitcall(date_cmd_exit);
