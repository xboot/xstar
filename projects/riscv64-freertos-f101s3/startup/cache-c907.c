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
#include <riscv64.h>
#include <cache.h>

/*
 * L1 cache line size in bytes on the C907.
 */
#define L1_CACHE_BYTES	(64)

/*
 * T-Head XuanTie custom Control & Status Registers (C906/C907).
 *
 *   CSR_MXSTATUS (0x7c0) : extended status / feature control
 *   CSR_MHCR     (0x7c1) : hardware cache control register
 *   CSR_MCOR     (0x7c2) : cache operation register (invalidate ...)
 *   CSR_MHINT    (0x7c5) : cache hint register (prefetch ...)
 *   CSR_MSMPR    (0x7f3) : smp / cache coherency register
 *
 * MHCR bits:
 *   bit0  IE    - enable i-cache
 *   bit1  DE    - enable d-cache
 *   bit2  WA    - write allocate
 *   bit3  WB    - write back
 *   bit4  RS    - return stack
 *   bit5  BPE   - branch prediction enable
 *   bit6  BTB   - branch target buffer
 *   bit8  WBR   - write burst
 *   bit12 L0BTB - level-0 branch target buffer
 */
#define CSR_MXSTATUS	0x7c0
#define CSR_MHCR		0x7c1
#define CSR_MCOR		0x7c2
#define CSR_MHINT		0x7c5
#define CSR_MSMPR		0x7f3

/*
 * MCOR value to invalidate the whole L1 i/d cache together with the
 * branch history table (BHT) and branch target buffer (BTB).
 *   bit0..1 CACHE_SEL = 0x3  -> select i + d cache
 *   bit4   INV        = 0x1  -> invalidate
 *   bit16  BHT_INV
 *   bit17  TB_INV
 */
#define MCOR_INV_ALL	(0x70013)

/*
 * MHCR value that enables both i-cache and d-cache with write-back mode,
 * return stack, branch prediction, branch target buffer, write burst and
 * the level-0 branch target buffer.
 */
#define MHCR_CACHE_EN	(0x11ff)

/*
 * MXSTATUS bits:
 *   bit15 MM          - hardware misaligned access
 *   bit16 UCME        - user mode cache operation
 *   bit17 CLINTEE     - clint super user interrupt / timer
 *   bit21 MAEE        - mmu pte address attribute extension
 *   bit22 THEADISAEE  - t-head extended instruction set
 */
#define MXSTATUS_EN		(0x638000)

/*
 * MHINT bits:
 *   bit2   DPLD       - d-cache prefetch
 *   bit8   IPLD       - i-cache prefetch
 *   bit9   LPE        - loop accelerator
 *   bit3..7 AMR       - store stream optimization
 *   bit13..14 DPLD    - prefetch 8 cache lines
 */
#define MHINT_EN		(0x16e30c)

/*
 * Enable the L1 i-cache and d-cache on the C907.
 *
 * This mirrors what the Allwinner F101 spl does:
 *   1. invalidate the whole L1 cache, BTB and BHT
 *   2. enable smp / cache coherency
 *   3. turn on the i-cache and d-cache (write-back) via MHCR
 *   4. enable misaligned access, maee and the t-head isa extension
 *   5. enable i/d cache prefetch and the loop accelerator
 *   6. synchronize the instruction stream
 */
void cache_enable(void)
{
	/* Invalidate L1 i/d cache, BTB and BHT */
	csr_write(CSR_MCOR, MCOR_INV_ALL);

	/* Enable smp / cache coherency */
	csr_write(CSR_MSMPR, 0x1);

	/* Enable i-cache and d-cache with write-back mode */
	csr_write(CSR_MHCR, MHCR_CACHE_EN);

	/* Enable misaligned access, maee and the t-head isa extension */
	csr_set(CSR_MXSTATUS, MXSTATUS_EN);

	/* Enable i/d cache prefetch and loop accelerator */
	csr_write(CSR_MHINT, MHINT_EN);

	/* Synchronize the instruction stream */
	__asm__ __volatile__("fence.i" ::: "memory");
}

/*
 * Write back (clean) a range of the d-cache to memory.
 */
void dcache_wb_range(unsigned long start, unsigned long end)
{
	unsigned long i = start & ~(L1_CACHE_BYTES - 1);

	for(; i < end; i += L1_CACHE_BYTES)
		__asm__ __volatile__("dcache.cpa %0\n" :: "r"(i) : "memory");
	__asm__ __volatile__("sync.is" ::: "memory");
}

/*
 * Invalidate a range of the d-cache. Dirty lines are discarded, so this
 * must only be used for buffers that are going to be (re)filled by a dma
 * master, never for buffers that still hold unwritten cpu data.
 */
void dcache_inv_range(unsigned long start, unsigned long end)
{
	unsigned long i = start & ~(L1_CACHE_BYTES - 1);

	for(; i < end; i += L1_CACHE_BYTES)
		__asm__ __volatile__("dcache.ipa %0\n" :: "r"(i) : "memory");
	__asm__ __volatile__("sync.is" ::: "memory");
}

/*
 * Write back and invalidate a range of the d-cache.
 */
void dcache_wbinv_range(unsigned long start, unsigned long end)
{
	unsigned long i = start & ~(L1_CACHE_BYTES - 1);

	for(; i < end; i += L1_CACHE_BYTES)
		__asm__ __volatile__("dcache.cipa %0\n" :: "r"(i) : "memory");
	__asm__ __volatile__("sync.is" ::: "memory");
}

/*
 * Flush (write back and invalidate) the whole d-cache.
 */
void dcache_flush_all(void)
{
	__asm__ __volatile__("dcache.ciall" ::: "memory");
	__asm__ __volatile__("sync.is" ::: "memory");
}
