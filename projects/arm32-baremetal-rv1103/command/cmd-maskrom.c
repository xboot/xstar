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
#include <rv1103/reg-cru.h>

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    maskrom\r\n");
}

static int do_maskrom(int argc, char ** argv)
{
	xos_io_write32(0xff020200, 0xef08a53c);
	xos_io_write32(RV1103_CRU_BASE + offsetof(struct rv1103_cru_t, glb_srst_fst), 0xfdb9);
	return 0;
}

static struct command_t cmd_maskrom = {
	.name	= "maskrom",
	.desc	= "reboot and enter to maskrom mode",
	.usage	= usage,
	.exec	= do_maskrom,
};

static void maskrom_cmd_init(void)
{
	register_command(&cmd_maskrom);
}

static void maskrom_cmd_exit(void)
{
	unregister_command(&cmd_maskrom);
}

command_initcall(maskrom_cmd_init);
command_exitcall(maskrom_cmd_exit);
