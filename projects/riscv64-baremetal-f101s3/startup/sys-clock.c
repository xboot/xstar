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
#include <f101/reg-ccu.h>

static inline uint32_t read32(io_addr_t addr)
{
	return (*((volatile uint32_t *)(addr)));
}

static inline void write32(io_addr_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

static inline void clrsetbits32(io_addr_t addr, uint32_t clr, uint32_t set)
{
	write32(addr, (read32(addr) & ~clr) | set);
}

static void sdelay(int cnt)
{
	for(int i = 0; i < cnt; i++)
		__asm__ __volatile__("nop");
}

static void set_vccio_detect(void)
{
	io_addr_t addr;
	uint32_t val;

	addr = 0x03090000 + 0x1f4;
	val = read32(addr);
	val &= ~(1 << 0);
	write32(addr, val);

	addr = 0x03000000 + 0x150;
	val = read32(addr);
	val &= ~(0x7 << 24);
	val |= (0x2 << 24);
	val |= (0x1 << 27);
	val &= ~(0x1 << 28);
	write32(addr, val);
}

static void set_pll_cpux_axi(void)
{
	io_addr_t addr;
	uint32_t val;

	addr = F101_CCU_BASE + CCU_PLL_CPU_CTRL_REG;
	val = read32(addr);
	val &= ~((0xff << 8) | (0x7 << 16) | (0x3 << 20) | (0xf << 0));
	val |= (0x2a << 8) | (0x0 << 16) | (0x0 << 20) | (0x0 << 0);
	write32(addr, val);
	sdelay(10);

	val = read32(addr);
	val &= ~(0x1 << 29);
	write32(addr, val);
	sdelay(10);
	val |= (0x1 << 29);
	write32(addr, val);

	val = read32(addr);
	val |= (0x1 << 26);
	write32(addr, val);
	do {
		val = read32(addr);
		val &= (0x1 << 26);
	} while(val);

	uint32_t judge = 0;
	do {
		val = read32(addr);
		val &= (0x1 << 28);
		judge = (val) ? judge + 1 : 0;
	} while(judge < 3);
	sdelay(20);

	addr = F101_CCU_BASE + 0xd00;
	val = read32(addr);
	val &= ~(0x1f << 0);
	val &= ~(0x300);
	write32(addr, val);
	sdelay(10);

	addr = F101_CCU_BASE + 0xd00;
	val = read32(addr);
	val &= ~(0x7 << 24);
	val |= (0x5 << 24);
	write32(addr, val);
	sdelay(10);
}

static void set_pll_periph0(void)
{
}

static void set_ahb(void)
{
}

static void set_apb(void)
{
}

static void set_pll_mbus(void)
{
	io_addr_t addr;
	uint32_t val;

	addr = F101_CCU_BASE + CCU_MBUS_CLK_REG;
	val = read32(addr);
	val &= ~(0x1f);
	val |= (1 << 0);
	write32(addr, val);
	sdelay(10);

	addr = F101_CCU_BASE + CCU_MBUS_CLK_REG;
	val = read32(addr);
	val &= ~(0x7 << 24);
	val |= (0x3 << 24);
	write32(addr, val);
	sdelay(10);
}

static void set_sramc_mode(void)
{
	io_addr_t addr = 0x03000004;

	/* VE */
	clrsetbits32(addr, 0x1 << 24, 0);
	/* DE */
	clrsetbits32(addr, 0x1 << 26, 0);
	/* USB */
	clrsetbits32(addr, (0x1 << 25) | (0x1 << 27), 0);
}

void sys_clock_init(void)
{
	set_vccio_detect();
	set_pll_cpux_axi();
	set_pll_periph0();
	set_ahb();
	set_apb();
	set_pll_mbus();
	set_sramc_mode();
}
