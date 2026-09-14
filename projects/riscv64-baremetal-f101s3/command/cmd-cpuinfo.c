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
#include <riscv64.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    cpuinfo\r\n");
}

static int do_cpuinfo(int argc, char ** argv)
{
	uint64_t v;

	v = csr_read(mvendorid);
	shell_printf("mvendorid:  0x%016lx\r\n", v);
	v = csr_read(marchid);
	shell_printf("marchid:    0x%016lx\r\n", v);
	v = csr_read(mimpid);
	shell_printf("mimpid:     0x%016lx\r\n", v);

	v = csr_read(mstatus);
	shell_printf("mstatus:    0x%016lx\r\n", v);
	v = csr_read(misa);
	shell_printf("misa:       0x%016lx\r\n", v);
	v = csr_read(medeleg);
	shell_printf("medeleg:    0x%016lx\r\n", v);
	v = csr_read(mideleg);
	shell_printf("mideleg:    0x%016lx\r\n", v);
	v = csr_read(mie);
	shell_printf("mie:        0x%016lx\r\n", v);
	v = csr_read(mtvec);
	shell_printf("mtvec:      0x%016lx\r\n", v);
	v = csr_read(mcounteren);
	shell_printf("mcounteren: 0x%016lx\r\n", v);

	v = csr_read(mscratch);
	shell_printf("mscratch:   0x%016lx\r\n", v);
	v = csr_read(mepc);
	shell_printf("mepc:       0x%016lx\r\n", v);
	v = csr_read(mcause);
	shell_printf("mcause:     0x%016lx\r\n", v);
	v = csr_read(mtval);
	shell_printf("mtval:      0x%016lx\r\n", v);
	v = csr_read(mip);
	shell_printf("mip:        0x%016lx\r\n", v);

	return 0;
}

static struct command_t cmd_cpuinfo = {
	.name	= "cpuinfo",
	.desc	= "show information about CPU",
	.usage	= usage,
	.exec	= do_cpuinfo,
};

static void cpuinfo_cmd_init(void)
{
	register_command(&cmd_cpuinfo);
}

static void cpuinfo_cmd_exit(void)
{
	unregister_command(&cmd_cpuinfo);
}

command_initcall(cpuinfo_cmd_init);
command_exitcall(cpuinfo_cmd_exit);
