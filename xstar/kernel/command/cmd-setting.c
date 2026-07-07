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

#include <kernel/core/setting.h>
#include <kernel/shell/context.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    setting                     - Print setting all key-value\r\n");
	shell_printf("    setting set <key> [<value>] - Set or clear setting key-value\r\n");
	shell_printf("    setting get <key> [...]     - Get setting key-value\r\n");
	shell_printf("    setting clear               - Clear setting all key-value\r\n");
}

static void cb(const char * key, const char * value)
{
	shell_printf("%s = %s\r\n", key, value);
}

static int do_setting(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, -1))
	{
		usage();
		return -1;
	}

	const char * cmd = sarg_at(&sarg, 0);
	if(!cmd)
	{
		setting_foreach(cb);
		return 0;
	}
	else if(!xos_strcmp(cmd, "set"))
	{
		const char * key = sarg_at(&sarg, 1);
		const char * val = sarg_at(&sarg, 2);
		if(key && val)
			setting_set(key, val);
		else if(key)
			setting_set(key, NULL);
		else
		{
			usage();
			return -1;
		}
	}
	else if(!xos_strcmp(cmd, "get"))
	{
		const char * key;
		int i = 1;
		if(!sarg_at(&sarg, 1))
		{
			usage();
			return -1;
		}
		while((key = sarg_at(&sarg, i++)) != NULL)
			shell_printf("%s = %s\r\n", key, setting_get(key, NULL));
	}
	else if(!xos_strcmp(cmd, "clear"))
	{
		setting_clear();
	}
	else
	{
		usage();
		return -1;
	}
	return 0;
}

static struct command_t cmd_setting = {
	.name	= "setting",
	.desc	= "the global setting tool",
	.usage	= usage,
	.exec	= do_setting,
};

static void setting_cmd_init(void)
{
	register_command(&cmd_setting);
}

static void setting_cmd_exit(void)
{
	unregister_command(&cmd_setting);
}

command_initcall(setting_cmd_init);
command_exitcall(setting_cmd_exit);
