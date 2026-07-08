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

static inline uint32_t read32(io_addr_t addr)
{
	return (*((volatile uint32_t *)(addr)));
}

static inline void write32(io_addr_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

void sys_uart_init(void)
{
	io_addr_t addr;
	uint32_t val, udiv;

	/* Config GPIO1_B2 and GPIO1_B3 to txd2 and rxd2 */
	addr = 0xff538000 + 0x00000008;
	if(42 & 0x4)
		addr += 0x4;
	val = (((0xf << 16) | (0x2 & 0xf)) << ((42 & 0x3) << 2));
	write32(addr, val);
	addr = 0xff538000 + 0x00000008;
	if(43 & 0x4)
		addr += 0x4;
	val = (((0xf << 16) | (0x2 & 0xf)) << ((43 & 0x3) << 2));
	write32(addr, val);

	/* Config uart2 to 115200-8-1-0 */
	addr = 0xff4c0000;
	write32(addr + 0x88, (1 << 0) | (1 << 1) | (1 << 2));
	write32(addr + 0x04, 0x0);
	write32(addr + 0x10, 0x0);
	write32(addr + 0x98, 0x1);
	write32(addr + 0x9c, 0x3);
	write32(addr + 0xa0, 0x1);

	val = read32(addr + 0x0c);
	val |= (1 << 7);
	write32(addr + 0x0c, val);
	udiv = 24000000 / (16 * 115200);
	write32(addr + 0x00, udiv & 0xff);
	write32(addr + 0x04, (udiv >> 8) & 0xff);
	val = read32(addr + 0x0c);
	val &= ~(1 << 7);
	write32(addr + 0x0c, val);
	val = read32(addr + 0x0c);
	val &= ~0x1f;
	val |= (0x3 << 0) | (0x0 << 2) | (0x0 << 3);
	write32(addr + 0x0c, val);
}

void sys_uart_putc(char c)
{
	io_addr_t addr = 0xff4c0000;

	while((read32(addr + 0x7c) & (0x1 << 1)) == 0);
	write32(addr + 0x00, c);
}

ssize_t sys_uart_write(void * buf, size_t count)
{
	char * p = buf;
	int i;

	for(i = 0; i < count; i++)
		sys_uart_putc(p[i]);
	return count;
}
