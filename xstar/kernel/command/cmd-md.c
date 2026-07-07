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
	shell_printf("    md [-b|-w|-l|-q] address [-c=count]\r\n");
}

static int do_md(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, (const char *[]){ "-b", "-w", "-l", "-q", "-c", NULL }, 1, 1))
	{
		usage();
		return -1;
	}

	io_addr_t addr = 0;
	char buf[16];
	int n = 64, size = 1;
	int i, len;
	uint8_t b; uint16_t w; uint32_t l; uint64_t q;

	if(sarg_has(&sarg, "-b"))
		size = 1;
	if(sarg_has(&sarg, "-w"))
		size = 2;
	if(sarg_has(&sarg, "-l"))
		size = 4;
	if(sarg_has(&sarg, "-q"))
		size = 8;
	addr = xos_strtoul(sarg_at(&sarg, 0), NULL, 0);
	n = sarg_get_uint(&sarg, "-c", 64);
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

	while(n > 0)
	{
		len = (n > 16) ? 16 : n;
		shell_printf("%08x: ", addr);
		if(size == 1)
		{
			for(i = 0; i < len; i += size)
			{
				xos_io_write8((io_addr_t)(&buf[i]), (b = xos_io_read8(addr + i)));
				shell_printf(" %02x", b);
			}
		}
		else if(size == 2)
		{
			for(i = 0; i < len; i += size)
			{
				xos_io_write16((io_addr_t)(&buf[i]), (w = xos_io_read16(addr + i)));
				shell_printf(" %04x", w);
			}
		}
		else if(size == 4)
		{
			for(i = 0; i < len; i += size)
			{
				xos_io_write32((io_addr_t)(&buf[i]), (l = xos_io_read32(addr + i)));
				shell_printf(" %08x", l);
			}
		}
		else if(size == 8)
		{
			for(i = 0; i < len; i += size)
			{
				xos_io_write64((io_addr_t)(&buf[i]), (q = xos_io_read64(addr + i)));
				shell_printf(" %016x", q);
			}
		}
		shell_printf("%*s", (16 - len) * 2 + (16 - len) / size + 4, "");
		for(i = 0; i < len; i++)
		{
			if((buf[i] < 0x20) || (buf[i] > 0x7e))
				shell_printf(".");
			else
				shell_printf("%c", buf[i]);
		}
		addr += len;
		n -= len;
		shell_printf("\r\n");

		if(shell_ctrlc())
			return -1;
	}
	return 0;
}

static struct command_t cmd_md = {
	.name	= "md",
	.desc	= "dump a memory region",
	.usage	= usage,
	.exec	= do_md,
};

static void md_cmd_init(void)
{
	register_command(&cmd_md);
}

static void md_cmd_exit(void)
{
	unregister_command(&cmd_md);
}

command_initcall(md_cmd_init);
command_exitcall(md_cmd_exit);
