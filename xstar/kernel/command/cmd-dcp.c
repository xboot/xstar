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

#include <driver/clocksource/clocksource.h>
#include <driver/block/block.h>
#include <kernel/shell/context.h>
#include <kernel/command/command.h>

enum devtype_t {
	DEVTYPE_BLOCK	= 0,
	DEVTYPE_FILE	= 1,
	DEVTYPE_MEM		= 2,
};

static void usage(void)
{
	shell_printf("usage:\r\n");
	shell_printf("    dcp <input@offset:size> <output@offset:size>\r\n");
}

static int do_dcp(int argc, char ** argv)
{
	struct sarg_t sarg;

	sarg_init(&sarg, argc, argv);
	if(!sarg_valid(&sarg, NULL, 2, 2))
	{
		usage();
		return -1;
	}

	uint64_t start = ktime_to_ns(ktime_get());
	uint64_t end;
	char sbyte[32];
	char sspeed[32];
	enum devtype_t itype, otype;
	struct block_t * iblk = NULL, * oblk = NULL;
	struct xfs_file_t * ifile = NULL;
	struct xfs_file_t * ofile = NULL;
	char ipath[CONFIG_XSTAR_MAX_PATH];
	char opath[CONFIG_XSTAR_MAX_PATH];
	char * iname, * oname;
	uint64_t ioff, isize;
	uint64_t ooff, osize;
	int64_t n, s, l;
	char * buf;
	char * p, * offset, * size;

	buf = xos_mem_malloc(64 * 1024);
	if(!buf)
		return -1;

	p = (char *)sarg_at(&sarg, 0);
	iname = xos_strsep(&p, "@");
	offset = xos_strsep(&p, ":");
	size = p;
	iname = (iname && xos_strcmp(iname, "") != 0) ? iname : NULL;
	offset = (offset && xos_strcmp(offset, "") != 0) ? offset : NULL;
	size = (size && xos_strcmp(size, "") != 0) ? size : NULL;
	ioff = offset ? xos_strtoull(offset, NULL, 0) : 0;
	isize = size ? xos_strtoull(size, NULL, 0) : ~0ULL;
	if(!iname)
	{
		itype = DEVTYPE_MEM;
	}
	else if((iblk = search_block(iname)))
	{
		l = block_capacity(iblk);
		if(ioff >= l)
		{
			shell_printf("Offset too large of input device '%s'\r\n", iname);
			xos_mem_free(buf);
			return -1;
		}
		isize = isize < (l - ioff) ? isize : (l - ioff);
		if(isize <= 0)
		{
			shell_printf("Don't need to copy of input device '%s'\r\n", iname);
			xos_mem_free(buf);
			return -1;
		}
		itype = DEVTYPE_BLOCK;
	}
	else if(((shell_realpath(iname, ipath)) >= 0) && ((ifile = xfs_open_read(shell_getxfs(), ipath)) != NULL))
	{
		l = xfs_length(ifile);
		if(ioff >= l)
		{
			shell_printf("Offset too large of input file '%s'\r\n", iname);
			xos_mem_free(buf);
			return -1;
		}
		isize = isize < (l - ioff) ? isize : (l - ioff);
		if(isize <= 0)
		{
			shell_printf("Don't need to copy of input file '%s'\r\n", iname);
			xos_mem_free(buf);
			return -1;
		}
		itype = DEVTYPE_FILE;
	}
	else
	{
		shell_printf("Can't find any input device '%s'\r\n", iname);
		xos_mem_free(buf);
		return -1;
	}

	p = (char *)sarg_at(&sarg, 1);
	oname = xos_strsep(&p, "@");
	offset = xos_strsep(&p, ":");
	size = p;
	oname = (oname && xos_strcmp(oname, "") != 0) ? oname : NULL;
	offset = (offset && xos_strcmp(offset, "") != 0) ? offset : NULL;
	size = (size && xos_strcmp(size, "") != 0) ? size : NULL;
	ooff = offset ? xos_strtoull(offset, NULL, 0) : 0;
	osize = size ? xos_strtoull(size, NULL, 0) : ~0ULL;
	if(!oname)
	{
		otype = DEVTYPE_MEM;
	}
	else if((oblk = search_block(oname)))
	{
		l = block_capacity(oblk);
		if(ooff >= l)
		{
			shell_printf("Offset too large of output device '%s'\r\n", oname);
			xos_mem_free(buf);
			return -1;
		}
		osize = osize < (l - ooff) ? osize : (l - ooff);
		if(osize <= 0)
		{
			shell_printf("Don't need copy to output device '%s'\r\n", oname);
			xos_mem_free(buf);
			return -1;
		}
		otype = DEVTYPE_BLOCK;
	}
	else if(((shell_realpath(oname, opath)) >= 0) && ((ofile = xfs_open_write(shell_getxfs(), opath)) != NULL))
	{
		ooff = 0;
		otype = DEVTYPE_FILE;
	}
	else
	{
		shell_printf("Can't find any output device '%s'\r\n", oname);
		xos_mem_free(buf);
		return -1;
	}

	s = isize < osize ? isize : osize;
	l = 0;
	if(itype == DEVTYPE_BLOCK)
		block_sync(iblk);

	while(l < s)
	{
		n = (s - l) < 64 * 1024 ? (s - l) : 64 * 1024;

		switch(itype)
		{
		case DEVTYPE_BLOCK:
			n = block_read(iblk, (uint8_t *)buf, ioff + l, n);
			break;
		case DEVTYPE_FILE:
			n = xfs_read(ifile, buf, n);
			break;
		case DEVTYPE_MEM:
			xos_memcpy(buf, (void *)((io_addr_t)(ioff + l)), n);
			break;
		default:
			break;
		}

		switch(otype)
		{
		case DEVTYPE_BLOCK:
			block_write(oblk, (uint8_t *)buf, ooff + l, n);
			break;
		case DEVTYPE_FILE:
			xfs_write(ofile, buf, n);
			break;
		case DEVTYPE_MEM:
			xos_memcpy((void *)((io_addr_t)(ooff + l)), buf, n);
			break;
		default:
			break;
		}

		l += n;
	}

	if(itype == DEVTYPE_FILE)
		xfs_close(ifile);
	if(otype == DEVTYPE_FILE)
	{
		xfs_flush(ofile);
		xfs_close(ofile);
	}
	else if(otype == DEVTYPE_BLOCK)
	{
		block_sync(oblk);
	}
	xos_mem_free(buf);

	end = ktime_to_ns(ktime_get());
	shell_printf("Copyed %s at %s/S - %s@0x%llx:0x%llx -> %s@0x%llx:0x%llx\r\n", xos_ssize(sbyte, s), xos_ssize(sspeed, (double)s * 1000000000.0 / (double)(end - start)), iname ? iname : "", ioff, s, oname ? oname : "", ooff, s);
	return 0;
}

static struct command_t cmd_dcp = {
	.name	= "dcp",
	.desc	= "device copy for file or block or memory",
	.usage	= usage,
	.exec	= do_dcp,
};

static void dcp_cmd_init(void)
{
	register_command(&cmd_dcp);
}

static void dcp_cmd_exit(void)
{
	unregister_command(&cmd_dcp);
}

command_initcall(dcp_cmd_init);
command_exitcall(dcp_cmd_exit);
