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

char * freertos_copyright_uniqueid(void)
{
	static char uniqueid[32 + 1] = { 0 };
	io_addr_t addr = 0x43006200;
	uint32_t sid[4];

	sid[0] = freertos_io_read32(addr + 0 * 4);
	sid[1] = freertos_io_read32(addr + 1 * 4);
	sid[2] = freertos_io_read32(addr + 2 * 4);
	sid[3] = freertos_io_read32(addr + 3 * 4);
	xos_snprintf(uniqueid, sizeof(uniqueid), "%08x%08x%08x%08x",sid[0], sid[1], sid[2], sid[3]);
	return uniqueid;
}

int freertos_copyright_verify(void)
{
	return 1;
}
