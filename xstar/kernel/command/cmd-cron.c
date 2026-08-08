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

#include <kernel/core/cron.h>
#include <kernel/core/setting.h>
#include <kernel/shell/shell.h>
#include <kernel/shell/context.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    5-field cron: \"<minute> <hour> <day> <month> <week>\"\r\n");
	shell_printf("    cron                             list all cron jobs\r\n");
	shell_printf("    cron add <name> <expr> <cmd>     add a periodic cron job\r\n");
	shell_printf("    cron oneshot <name> <expr> <cmd> add a oneshot cron job\r\n");
	shell_printf("    cron remove <name>               remove a cron job\r\n");
}

static struct cron_t * cron_get(void)
{
	static struct cron_t * cron = NULL;
	if(!cron)
		cron = cron_alloc(setting_get("timezone", "Asia/Shanghai"));
	return cron;
}

static void cron_list_cb(char * name, int oneshot, void * data)
{
	int * count = (int *)data;
	shell_printf("    %-16s %s\r\n", name, oneshot ? "oneshot" : "periodic");
	if(count)
		(*count)++;
}

static void cron_exec_cb(void * data)
{
	if(data)
		shell_system((const char *)data);
}

static int do_cron(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, -1))
	{
		usage();
		return -1;
	}
	if(!cron_get())
	{
		shell_printf("cron: failed to initialize cron subsystem\r\n");
		return -1;
	}

	const char * sub = sarg_at(&sarg, 0);
	if(!sub || (xos_strcmp(sub, "list") == 0))
	{
		int count = 0;
		cron_foreach(cron_get(), cron_list_cb, &count);
		shell_printf("%d cron job(s)\r\n", count);
		return 0;
	}
	else if((xos_strcmp(sub, "add") == 0) || (xos_strcmp(sub, "oneshot") == 0))
	{
		int oneshot = (xos_strcmp(sub, "oneshot") == 0) ? 1 : 0;
		const char * name = sarg_at(&sarg, 1);
		const char * expr = sarg_at(&sarg, 2);
		if(!name || !expr)
		{
			usage();
			return -1;
		}
		char * cmd = xos_strdup(sarg_at(&sarg, 3));
		if(!cmd)
		{
			usage();
			return -1;
		}
		if(!cron_add(cron_get(), name, expr, oneshot, cron_exec_cb, cmd))
		{
			shell_printf("cron: failed to add job '%s'\r\n", name);
			xos_mem_free(cmd);
			return -1;
		}
		shell_printf("cron: job '%s' added\r\n", name);
		return 0;
	}
	else if(xos_strcmp(sub, "remove") == 0)
	{
		const char * name = sarg_at(&sarg, 1);
		if(!name)
		{
			usage();
			return -1;
		}
		if(!cron_remove(cron_get(), name))
		{
			shell_printf("cron: job '%s' not found\r\n", name);
			return -1;
		}
		shell_printf("cron: job '%s' removed\r\n", name);
		return 0;
	}
	usage();
	return -1;
}

static struct command_t cmd_cron = {
	.name	= "cron",
	.desc	= "schedule commands with cron expressions",
	.usage	= usage,
	.exec	= do_cron,
};

static void cron_cmd_init(void)
{
	register_command(&cmd_cron);
}

static void cron_cmd_exit(void)
{
	unregister_command(&cmd_cron);
}

command_initcall(cron_cmd_init);
command_exitcall(cron_cmd_exit);
