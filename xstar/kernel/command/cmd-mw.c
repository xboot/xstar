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
#include <kernel/shell/ctrlc.h>
#include <kernel/command/command.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    mw [-b|-w|-l|-q] address value [-c=count]\r\n");
}

static int do_mw(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char *[]){ "-b", "-w", "-l", "-q", "-c", NULL }, 2, 2))
	{
		usage();
		return -1;
	}

	io_addr_t addr = 0;
	uint64_t value = 0;
	int size = 1, n = 1;
	int i;

	if(sarg_has(&sarg, "-b"))
		size = 1;
	if(sarg_has(&sarg, "-w"))
		size = 2;
	if(sarg_has(&sarg, "-l"))
		size = 4;
	if(sarg_has(&sarg, "-q"))
		size = 8;
	addr = xos_strtoul(sarg_at(&sarg, 0), NULL, 0);
	value = xos_strtoull(sarg_at(&sarg, 1), NULL, 0);
	const char * cs = sarg_get(&sarg, "-c", NULL);
	if(cs)
	{
		n = xos_strtoul(cs, NULL, 0);
		if(n == 0)
		{
			shell_printf("mw: the writing count is zero by '-c=%s'", cs);
			return -1;
		}
	}
	if(size == 1)
	{
		addr &= ~((io_addr_t)0x0);
	}
	else if(size == 2)
	{
		addr &= ~((io_addr_t)0x1);
	}
	else if(size == 4)
	{
		addr &= ~((io_addr_t)0x3);
	}
	else if(size == 8)
	{
		addr &= ~((io_addr_t)0x7);
	}
	n = n * size;

	for(i = 0; i < n; i += size)
	{
		if(size == 1)
			xos_io_write8((io_addr_t)(addr + i), (uint8_t)value);
		else if(size == 2)
			xos_io_write16((io_addr_t)(addr + i), (uint16_t)value);
		else if(size == 4)
			xos_io_write32((io_addr_t)(addr + i), (uint32_t)value);
		else if(size == 8)
			xos_io_write64((io_addr_t)(addr + i), (uint64_t)value);
		if(shell_ctrlc())
			return -1;
	}
	shell_printf("write done.\r\n");

	return 0;
}

static struct command_t cmd_mw = {
	.name	= "mw",
	.desc	= "write values to memory region",
	.usage	= usage,
	.exec	= do_mw,
};

static void mw_cmd_init(void)
{
	register_command(&cmd_mw);
}

static void mw_cmd_exit(void)
{
	unregister_command(&cmd_mw);
}

command_initcall(mw_cmd_init);
command_exitcall(mw_cmd_exit);
