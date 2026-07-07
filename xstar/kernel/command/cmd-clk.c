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

#include <driver/clk/clk.h>
#include <kernel/shell/context.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    clk [name ...]\r\n");
}

static void clk_dump(char * parent, int level)
{
	struct device_t * pos, * n;
	struct clk_t * clk;
	const char * pname;
	char * name;
	uint64_t rate;
	int i;

	list_for_each_entry_safe(pos, n, &__device_head[DEVICE_TYPE_CLK], head)
	{
		if((clk = (struct clk_t *)(pos->priv)))
		{
			name = clk->name;
			pname = clk_get_parent(name);
			if((parent && pname && (xos_strcmp(parent, pname) == 0)) || (!parent && !pname))
			{
				rate = clk_get_rate(name);
				for(i = 0; i < level; i++)
					shell_printf("    ");
				shell_printf("%*s %6Ld.%06LdMHZ %4d\r\n", -(64 - level * 4), name, rate / (uint64_t)(1000 * 1000), rate % (uint64_t)(1000 * 1000), clk_status(name) ? 1 : 0);
				clk_dump(clk->name, level + 1);
			}
		}
	}
}

static int do_clk(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, -1))
	{
		usage();
		return -1;
	}
	uint64_t rate;
	char * name;

	if(sarg_at(&sarg, 0) == NULL)
	{
		clk_dump(NULL, 0);
	}
	else
	{
		sarg_iter_reset(&sarg);
		while((name = (char *)sarg_iter_next(&sarg)) != NULL)
		{
			if(search_clk(name))
			{
				rate = clk_get_rate(name);
				shell_printf("%*s %6Ld.%06LdMHZ %4d\r\n", -(64 - 0 * 4), name, rate / (uint64_t)(1000 * 1000), rate % (uint64_t)(1000 * 1000), clk_status(name) ? 1 : 0);
				clk_dump(name, 1);
			}
		}
	}
	return 0;
}

static struct command_t cmd_clk = {
	.name	= "clk",
	.desc	= "dump clock in a tree-like format",
	.usage	= usage,
	.exec	= do_clk,
};

static void clk_cmd_init(void)
{
	register_command(&cmd_clk);
}

static void clk_cmd_exit(void)
{
	unregister_command(&cmd_clk);
}

command_initcall(clk_cmd_init);
command_exitcall(clk_cmd_exit);
