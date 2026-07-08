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

#include <baremetal/baremetal.h>

uint8_t baremetal_io_read8(io_addr_t addr)
{
	return (*((volatile uint8_t *)(addr)));
}

void baremetal_io_write8(io_addr_t addr, uint8_t value)
{
	*((volatile uint8_t *)(addr)) = value;
}

uint16_t baremetal_io_read16(io_addr_t addr)
{
	return (*((volatile uint16_t *)(addr)));
}

void baremetal_io_write16(io_addr_t addr, uint16_t value)
{
	*((volatile uint16_t *)(addr)) = value;
}

uint32_t baremetal_io_read32(io_addr_t addr)
{
	return (*((volatile uint32_t *)(addr)));
}

void baremetal_io_write32(io_addr_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

uint64_t baremetal_io_read64(io_addr_t addr)
{
	return (*((volatile uint64_t *)(addr)));
}

void baremetal_io_write64(io_addr_t addr, uint64_t value)
{
	*((volatile uint64_t *)(addr)) = value;
}
