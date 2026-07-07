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
	shell_printf("    uniqueid [qrcode invert]\r\n");
}

static int do_uniqueid(int argc, char ** argv)
{
	struct sarg_t sarg;
	int invert = 1;
	char * id;
	char * s;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 0, 1))
	{
		usage();
		return -1;
	}
	const char * p0 = sarg_at(&sarg, 0);
	if(p0)
		invert = (xos_strtoul(p0, NULL, 0) != 0) ? 1 : 0;

	id = xos_copyright_uniqueid();
	shell_printf("ID: %s\r\n", id);
	s = qrcgen_tostring(id, invert);
	if(s)
	{
		shell_printf("%s", s);
		xos_mem_free(s);
	}
	return 0;
}

static struct command_t cmd_uniqueid = {
	.name	= "uniqueid",
	.desc	= "show qrcode for system uniqueid",
	.usage	= usage,
	.exec	= do_uniqueid,
};

static void uniqueid_cmd_init(void)
{
	register_command(&cmd_uniqueid);
}

static void uniqueid_cmd_exit(void)
{
	unregister_command(&cmd_uniqueid);
}

command_initcall(uniqueid_cmd_init);
command_exitcall(uniqueid_cmd_exit);
