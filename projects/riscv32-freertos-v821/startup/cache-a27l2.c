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
#include <cache.h>
#include <riscv32.h>

/*
 * Control and Status Registers
 */
#define CSR_MCACHE_CTL			0x7ca
#define CSR_MMSC_CFG			0xfc2
#define CSR_MXSTATUS			0x7c4
#define CSR_MMISC_CTL			0x7d0
#define CSR_UCODE				0x801
#define CSR_UCCTLBEGINADDR		0x80b
#define CSR_UCCTLCOMMAND		0x80c

/*
 * Value of x rounded up to the next multiple of align, align must be power of 2
 */
#define ROUND_UP(x, align)		(((x) + (align) - 1) & ~((align) - 1))

/*
 * Value of x rounded down to the prev multiple of align, align must be power of 2
 */
#define ROUND_DOWN(x, align)	((x) & ~((align) - 1))

/*
 * L2 cache
 */
#define V821_L2_CACHE_BASE		(0x49000000)
#define CACHE_LINE_SIZE			(64)
#define CACHE_MAX_SIZE			(128 * 1024)

enum {
	L2_CFG			= 0x00,
	L2_CTL			= 0x08,
	L2_HPM_C0_CTL	= 0x10,
	L2_HPM_C1_CTL	= 0x18,
	L2_HPM_C2_CTL	= 0x20,
	L2_HPM_C3_CTL	= 0x28,
	L2_C0_CMD		= 0x40,
	L2_C0_ACC		= 0x48,
	L2_C1_CMD		= 0x50,
	L2_C1_ACC		= 0x58,
	L2_C2_CMD		= 0x60,
	L2_C2_ACC		= 0x68,
	L2_C3_CMD		= 0x70,
	L2_C3_ACC		= 0x78,
	L2_C0_STATUS	= 0x80,
	L2_C0_HPM		= 0x200,
};

static inline uint32_t read32(io_addr_t addr)
{
	return (*((volatile uint32_t *)(addr)));
}

static inline void write32(io_addr_t addr, uint32_t value)
{
	*((volatile uint32_t *)(addr)) = value;
}

static inline unsigned long l2_get_cctl_status(void)
{
	return read32((V821_L2_CACHE_BASE + L2_C0_STATUS));
}

void dcache_flush_all(void)
{
	csr_write(CSR_UCCTLCOMMAND, 6);

	write32((V821_L2_CACHE_BASE + L2_C0_CMD), 0x12);
	while((l2_get_cctl_status() & 0xf) != 0);
}

static inline void __dcache_wb_line(unsigned long start)
{
	csr_write(CSR_UCCTLBEGINADDR, start);
	csr_write(CSR_UCCTLCOMMAND, 1);

	write32(V821_L2_CACHE_BASE + L2_C0_ACC, start);
	write32(V821_L2_CACHE_BASE + L2_C0_CMD, 0x9);
	while((l2_get_cctl_status() & 0xf) != 0);
}

static inline void __dcache_wb_range(unsigned long start, unsigned long end)
{
	if((end - start) >= CACHE_MAX_SIZE)
	{
		dcache_flush_all();
		return;
	}
	while(end > start)
	{
		__dcache_wb_line(start);
		start += CACHE_LINE_SIZE;
	}
}

void dcache_wb_range(unsigned long start, unsigned long end)
{
	start = start & ~(CACHE_LINE_SIZE - 1);
	__dcache_wb_range(start, end);
}

static inline void __dcache_inv_line(unsigned long start)
{
	csr_write(CSR_UCCTLBEGINADDR, start);
	csr_write(CSR_UCCTLCOMMAND, 0);

	write32(V821_L2_CACHE_BASE + L2_C0_ACC, start);
	write32(V821_L2_CACHE_BASE + L2_C0_CMD, 0x8);
	while((l2_get_cctl_status() & 0xf) != 0);
}

static inline void __dcache_inv_range(unsigned long start, unsigned long end)
{
	if((end - start) >= CACHE_MAX_SIZE)
	{
		dcache_flush_all();
		return;
	}
	while(end > start)
	{
		__dcache_inv_line(start);
		start += CACHE_LINE_SIZE;
	}
}

void dcache_inv_range(unsigned long start, unsigned long end)
{
	start = start & (~(CACHE_LINE_SIZE - 1));
	end = ((end + CACHE_LINE_SIZE - 1) & (~(CACHE_LINE_SIZE - 1)));
	__dcache_inv_range(start, end);
}

static inline void l2cache_enable(void)
{
	unsigned long val;

	val = read32(V821_L2_CACHE_BASE + L2_CTL);
	val |= 1 << 13;
	val |= 1 << 10;
	val &= ~(0x3 << 3);
	val |= 0x3 << 3;
	val &= ~(0x3 << 5);
	val |= 0x3 << 5;
	val |= 1 << 0;
	write32(V821_L2_CACHE_BASE + L2_CTL, val);
}

void cache_enable(void)
{
	/*
	 * Enable I/D cache with HW pre-fetcher,
	 * D-cache write-around (threshold: 4 cache lines),
	 * and CM (Coherence Manager).
	 */
	csr_clear(CSR_MCACHE_CTL, (0x3 << 21) | (0xf << 13));
	csr_set(CSR_MCACHE_CTL, (2 << 21) | (1 << 19) | (1 << 15) | (1 << 13) | (1 << 10) | (1 << 9) | (1 << 1) | (1 << 0));

	/*
	 * Check if CPU support CM or not
	 */
	if(csr_read(CSR_MCACHE_CTL) & (1 << 19))
	{
		/*
		 * Wait for cache coherence enabling completed
		 */
		while((csr_read(CSR_MCACHE_CTL) & (1 << 20)) == 0);
	}

	/*
	 * Enable misaligned access and non-blocking load
	 */
	csr_set(CSR_MMISC_CTL, (1 << 8) | (1 << 6));

	/*
	 * Enable L2 cache
	 */
	l2cache_enable();
}
