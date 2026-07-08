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

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    mmuinfo <ADDRESS>\r\n");
}

static char * inner_attr[] = {
	"0b000 Non-cacheable",
	"0b001 Strongly-ordered",
	"0b010 (reserved)",
	"0b011 Device",
	"0b100 (reserved)",
	"0b101 Write-Back, Write-Allocate",
	"0b110 Write-Through",
	"0b111 Write-Back, no Write-Allocate",
};

static char * outer_attr[] = {
	"0b00 Non-cacheable",
	"0b01 Write-Back, Write-Allocate",
	"0b10 Write-Through, no Write-Allocate",
	"0b11 Write-Back, no Write-Allocate",
};

static void decode_par(unsigned long par)
{
	shell_printf("    Physical Address [31:12]: 0x%08lx\r\n", par & 0xFFFFF000);
	shell_printf("    Reserved [11]:            0x%lx\r\n", (par >> 11) & 0x1);
	shell_printf("    Not Outer Shareable [10]: 0x%lx\r\n", (par >> 10) & 0x1);
	shell_printf("    Non-Secure [9]:           0x%lx\r\n", (par >> 9) & 0x1);
	shell_printf("    Impl. def. [8]:           0x%lx\r\n", (par >> 8) & 0x1);
	shell_printf("    Shareable [7]:            0x%lx\r\n", (par >> 7) & 0x1);
	shell_printf("    Inner mem. attr. [6:4]:   0x%lx (%s)\r\n", (par >> 4) & 0x7, inner_attr[(par >> 4) & 0x7]);
	shell_printf("    Outer mem. attr. [3:2]:   0x%lx (%s)\r\n", (par >> 2) & 0x3, outer_attr[(par >> 2) & 0x3]);
	shell_printf("    SuperSection [1]:         0x%lx\r\n", (par >> 1) & 0x1);
	shell_printf("    Failure [0]:              0x%lx\r\n", (par >> 0) & 0x1);
}

static int do_mmuinfo(int argc, char ** argv)
{
	unsigned long addr = 0, priv_read, priv_write;

	if(argc < 2)
	{
		usage();
		return -1;
	}

	addr = strtoul(argv[1], NULL, 0);

	__asm__ __volatile__(
		"mcr p15, 0, %0, c7, c8, 0\n"
		:
		: "r" (addr)
		: "memory");

	__asm__ __volatile__(
		"mrc p15, 0, %0, c7, c4, 0\n"
		: "=r" (priv_read)
		:
		: "memory");

	__asm__ __volatile__(
		"mcr p15, 0, %0, c7, c8, 1\n"
		:
		: "r" (addr)
		: "memory");

	__asm__ __volatile__(
		"mrc p15, 0, %0, c7, c4, 0\n"
		: "=r" (priv_write)
		:
		: "memory");

	shell_printf("PAR result for 0x%08lx: \r\n", addr);
	shell_printf("  privileged read: 0x%08lx\r\n", priv_read);
	decode_par(priv_read);
	shell_printf("  privileged write: 0x%08lx\r\n", priv_write);
	decode_par(priv_write);

	return 0;
}

static struct command_t cmd_mmuinfo = {
	.name	= "mmuinfo",
	.desc	= "show mmu/cache information of an address",
	.usage	= usage,
	.exec	= do_mmuinfo,
};

static void mmuinfo_cmd_init(void)
{
	register_command(&cmd_mmuinfo);
}

static void mmuinfo_cmd_exit(void)
{
	unregister_command(&cmd_mmuinfo);
}

command_initcall(mmuinfo_cmd_init);
command_exitcall(mmuinfo_cmd_exit);
