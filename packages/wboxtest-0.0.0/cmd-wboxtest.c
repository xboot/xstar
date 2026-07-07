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

#include <wboxtest.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    wboxtest [group] [name] [-c=count]\r\n");
	shell_printf("    wboxtest -l\r\n");
}

static int do_wboxtest(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char *[]){ "-l", "-c", NULL }, 0, 2))
	{
		usage();
		return -1;
	}

	if(sarg_has(&sarg, "-l"))
	{
		wboxtest_list();
		return 0;
	}

	const char * group = sarg_at(&sarg, 0);
	const char * name = sarg_at(&sarg, 1);
	int count = sarg_get_uint(&sarg, "-c", 1);
	if(count > 0)
	{
		if(!group && !name)
			wboxtest_run_all(count);
		else if(group && !name)
			wboxtest_run_group(group, count);
		else if(group && name)
			wboxtest_run_group_name(group, name, count);
	}
	return 0;
}

static struct command_t cmd_wboxtest = {
	.name	= "wboxtest",
	.desc	= "whitebox test for some modules",
	.usage	= usage,
	.exec	= do_wboxtest,
};

static void wboxtest_cmd_init(void)
{
	register_command(&cmd_wboxtest);
}

static void wboxtest_cmd_exit(void)
{
	unregister_command(&cmd_wboxtest);
}

command_initcall(wboxtest_cmd_init);
command_exitcall(wboxtest_cmd_exit);
