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

void sys_jtag_init(void)
{
	io_addr_t addr;
	uint32_t val;

	/* Config GPIOC0 and GPIOC5 to jtag mode for andes a27l2 cpu core */
	addr = 0x42000060 + 0x0;
	val = read32(addr);
	val &= ~(0xf << ((0 & 0x7) << 2));
	val |= ((0x3 & 0xf) << ((0 & 0x7) << 2));
	write32(addr, val);

	val = read32(addr);
	val &= ~(0xf << ((5 & 0x7) << 2));
	val |= ((0x3 & 0xf) << ((5 & 0x7) << 2));
	write32(addr, val);
}
