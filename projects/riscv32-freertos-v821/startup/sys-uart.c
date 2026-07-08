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
	uint32_t val;

	/* Config GPIOL4 and GPIOL5 to txd0 and rxd0 */
	addr = 0x42000540 + 0x0;
	val = read32(addr);
	val &= ~(0xf << ((4 & 0x7) << 2));
	val |= ((0x3 & 0xf) << ((4 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((5 & 0x7) << 2));
	val |= ((0x3 & 0xf) << ((5 & 0x7) << 2));
	write32(addr, val);

	/* Open the clock gate for uart0 */
	addr = 0x42001080;
	val = read32(addr);
	val |= 1 << 15;
	write32(addr, val);

	/* Deassert uart0 reset */
	addr = 0x42001090;
	val = read32(addr);
	val |= 1 << 15;
	write32(addr, val);

	/* Config uart0 to 115200-8-1-0 */
	addr = 0x42500000;
	write32(addr + 0x04, 0x0);
	write32(addr + 0x08, 0xf7);
	write32(addr + 0x10, 0x0);
	val = read32(addr + 0x0c);
	val |= (1 << 7);
	write32(addr + 0x0c, val);
	write32(addr + 0x00, 0x68 & 0xff);
	write32(addr + 0x04, (0x68 >> 8) & 0xff);
	val = read32(addr + 0x0c);
	val &= ~(1 << 7);
	write32(addr + 0x0c, val);
	val = read32(addr + 0x0c);
	val &= ~0x1f;
	val |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
	write32(addr + 0x0c, val);
}

void sys_uart_putc(char c)
{
	io_addr_t addr = 0x42500000;

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
