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

#include <freertos/freertos.h>

enum {
	SID_PRCTL		= 0x03006000 + 0x00,
	SID_PR_ADDR		= 0x03006000 + 0x04,
	SID_PRKEY		= 0x03006000 + 0x08,
	SID_RDKEY		= 0x03006000 + 0x0C,
	EFUSE_HV_SWITCH	= 0x03090000 + 0x204,
};

static uint32_t efuse_read(uint32_t offset)
{
	uint32_t val;

	xos_io_write32(SID_PR_ADDR, (offset >> 2));
	val = xos_io_read32(SID_PRCTL);
	val &= ~((0xffff << 16) | 0x3);
	val |= (0xadbf << 16) | 0x2;
	xos_io_write32(SID_PRCTL, val);
	while(xos_io_read32(SID_PRCTL) & 0x2);
	val &= ~((0xffff << 16) | 0x3);
	xos_io_write32(SID_PRCTL, val);
	val = xos_io_read32(SID_RDKEY);
	return val;
}

static const char * platform_uniqueid(void)
{
	static char uniqueid[32 + 1] = { 0 };
	uint32_t sid[4];

	sid[0] = efuse_read(0x0);
	sid[1] = efuse_read(0x4);
	sid[2] = efuse_read(0x8);
	sid[3] = efuse_read(0xc);
	xos_snprintf(uniqueid, sizeof(uniqueid), "%08x%08x%08x%08x",sid[0], sid[1], sid[2], sid[3]);
	return uniqueid;
}

static int platform_verify(void)
{
	return 1;
}

static int platform_keygen(const char * msg, void * key, int maxlen)
{
	return 0;
}

static struct copyright_t copyright_platform = {
	.uniqueid	= platform_uniqueid,
	.keygen		= platform_keygen,
	.verify		= platform_verify,
};

static void copyright_platform_init(void)
{
	register_copyright(&copyright_platform);
}
core_initcall(copyright_platform_init);
