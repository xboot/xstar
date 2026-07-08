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
#include <barrier.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    fel\r\n");
}

static int do_fel(int argc, char ** argv)
{
	io_addr_t addr = 0x4a000000;

	do {
		xos_io_write32(addr + 0x200 + 0x8, 0x5aa5a55a);
		mb();
		smp_mb();
	} while(xos_io_read32(addr + 0x200 + 0x8) != 0x5aa5a55a);
	xstar_reboot();

	return 0;
}

static struct command_t cmd_fel = {
	.name	= "fel",
	.desc	= "reboot and enter to fel mode",
	.usage	= usage,
	.exec	= do_fel,
};

static void fel_cmd_init(void)
{
	register_command(&cmd_fel);
}

static void fel_cmd_exit(void)
{
	unregister_command(&cmd_fel);
}

command_initcall(fel_cmd_init);
command_exitcall(fel_cmd_exit);
