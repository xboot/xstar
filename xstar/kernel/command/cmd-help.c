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
	shell_printf("    help [command ...]\r\n");
}

static void cmdcb(struct command_t * cmd, void * data)
{
	struct {
		struct slist_t * sl;
		int maxlen;
	} * pdat = data;

	int len = xos_strlen(cmd->name);
	if(len > pdat->maxlen)
		pdat->maxlen = len;
	slist_add(pdat->sl, cmd, "%s", cmd->name);
}

static int do_help(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, -1))
	{
		usage();
		return -1;
	}
	if(sarg_at(&sarg, 0) == NULL)
	{
		struct slist_t * e;
		struct {
			struct slist_t * sl;
			int maxlen;
		} pdat = { slist_alloc(), 0 };
		command_foreach(cmdcb, &pdat);

		slist_sort(pdat.sl);
		slist_for_each_entry(e, pdat.sl)
		{
			struct command_t * c = (struct command_t *)e->priv;
			shell_printf(" %s%*s - %s\r\n", c->name, pdat.maxlen - xos_strlen(c->name), "", c->desc);
		}
		slist_free(pdat.sl);
	}
	else
	{
		const char * name;
		sarg_iter_reset(&sarg);
		while((name = sarg_iter_next(&sarg)) != NULL)
		{
			struct command_t * c = search_command(name);
			if(c)
			{
				shell_printf("%s - %s\r\n", c->name, c->desc);
				if(c->usage)
					c->usage();
			}
			else
			{
				shell_printf("unknown command '%s' - try 'help' for list all of commands\r\n", name);
			}
		}
	}
	return 0;
}

static struct command_t cmd_help = {
	.name	= "help",
	.desc	= "show online help about command",
	.usage	= usage,
	.exec	= do_help,
};

static void help_cmd_init(void)
{
	register_command(&cmd_help);
}

static void help_cmd_exit(void)
{
	unregister_command(&cmd_help);
}

command_initcall(help_cmd_init);
command_exitcall(help_cmd_exit);
