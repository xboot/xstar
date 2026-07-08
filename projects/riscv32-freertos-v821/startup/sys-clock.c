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
#include <v821/reg-ccu.h>

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

/*
 * Detect hosc freq, 0 for 40mhz, 1 for 24mhz
 */
static int detect_hosc(void)
{
	/*
	 * hosc dectect enable bit in efuse is bit 613
	 */
	uint32_t hosc_det_en = (read32(0x43006200 + 76) >> 5) & 0x1;
	uint32_t val = read32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, hosc_freq_det));

	if(hosc_det_en || (val & (1 << 1)))
	{
		if(read32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_func_cfg_reg)) & (1 << 31))
			return 1;
		else
			return 0;
	}
	write32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, hosc_freq_det), val & ~(1 << 0));
	write32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, hosc_freq_det), val | (1 << 0));
	do {
		val = read32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, hosc_freq_det));
	} while(!(val & (1 << 1)));
	val = (val & 0x00fffff0) >> 4;
	if(val < ((46875 + 78125) / 2))
		return 1;
	else
		return 0;
}

/*
 * Set pll peri
 */
static void set_pll_peri_ctrl0(uint32_t en, uint32_t gate, uint32_t n, uint32_t m)
{
	uint32_t pll_en;
	uint32_t pll_ldo_en;
	uint32_t pll_lock_en;
	uint32_t pll_output_gate;

	if(en)
	{
		pll_en = 1 << 31;
		pll_ldo_en = 1 << 30;
		pll_lock_en = 1 << 29;
	}
	else
	{
		pll_en = 0 << 31;
		pll_ldo_en = 0 << 30;
		pll_lock_en = 0 << 29;
	}
	pll_output_gate = 0 << 27;
	n = n << 8;
	m = m << 0;
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg), 0x7 << 0, m);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg), 0xff << 8, n);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg), (1 << 31) | (1 << 30) | (1 << 27), pll_en | pll_ldo_en | pll_output_gate);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg), 1 << 29, pll_lock_en);
	while(!(read32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg)) & (1 << 28)));
	if(gate)
	{
		pll_output_gate = 1 << 27;
		clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg), (1 << 27), pll_output_gate);
	}
}

/*
 * Enable peri cko
 */
static void set_pll_peri_ctrl1(void)
{
	uint32_t val;

	val = read32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl1_reg));
	val |= 0xffff;
	write32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl1_reg), val);
}

/*
 * Set peri to hosc * 2n / m = 3072mhz
 */
static void set_pll_peri(void)
{
	if(!(read32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_peri_ctrl0_reg)) & (1 << 31)))
	{
		if(detect_hosc())
			set_pll_peri_ctrl0(1, 1, 191, 2);
		else
			set_pll_peri_ctrl0(1, 1, 191, 4);
	}
	set_pll_peri_ctrl1();
}

/*
 * Set ahb to 768mhz / 4 = 192mhz
 */
static void set_ahb(void)
{
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, ahb_clk_reg), 0x1f << 0, 3 << 0);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, ahb_clk_reg), 0x3 << 24, 1 << 24);
}

/*
 * Set apb to 384mhz / 4 = 96mhz
 */
static void set_apb(void)
{
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, apb_clk_reg), 0x1f << 0, 3 << 0);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, apb_clk_reg), 0x3 << 24, 1 << 24);
}

/*
 * Set apb-spc to 192mhz / 1 = 192mhz
 */
static void set_apb_spc(void)
{
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, apb_spec_clk_reg), 0x1f << 0, 0 << 0);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, apb_spec_clk_reg), 0x3 << 24, 3 << 24);
}

/*
 * Set pll peri
 */
static void set_pll_general(uint32_t pll_addr, uint32_t en, uint32_t output_gate_en, uint32_t pll_d, uint32_t pll_d_off, uint32_t pll_n)
{
	uint32_t pll_en;
	uint32_t pll_ldo_en;
	uint32_t pll_lock_en;

	if(en)
	{
		pll_en = 1 << 31;
		pll_ldo_en = 1 << 30;
		pll_lock_en = 1 << 29;
	}
	else
	{
		pll_en = 0 << 31;
		pll_ldo_en = 0 << 30;
		pll_lock_en = 0 << 29;
	}
	clrsetbits32(pll_addr, 3 << 2, pll_d << pll_d_off);
	clrsetbits32(pll_addr, 0xff << 8, pll_n << 8);
	clrsetbits32(pll_addr, 1 << 30 | 1 << 27 | 1 << 31, pll_en | pll_ldo_en | pll_lock_en);
	clrsetbits32(pll_addr, 1 << 29, 1 << 29);
	while(!(read32(pll_addr) & 1 << 28));
	clrsetbits32(pll_addr, 1 << 27, 1 << 27);
}

static void set_pll_e90x(void)
{
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, e907_clk_reg), 0x1f << 0, 0x0 << 0);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, e907_clk_reg), 0x7 << 24, 0x6 << 24);
}

static void set_pll_a27l2(void)
{
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, a27l2_clk_reg), 0x1f << 0, 0x0 << 0);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, a27l2_clk_reg), 0x7 << 24, 0x4 << 24);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, a27l2_clk_reg), 1 << 31, 1 << 31);
}

/*
 * Set csi rate, hosc / 4 * N , N = N(INT) + N(FRAC) = 675mhz
 */
static void set_pll_csi(void)
{
	uint32_t n, input_div, wave_bot;

	if(detect_hosc())
	{
		n = 55;
		wave_bot = 0xc0008000;
		input_div = 1;
	}
	else
	{
		n = 66;
		wave_bot = 0xc0010000;
		input_div = 2;
	}
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 0xff << 8, n << 8);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 0x3 << 1, input_div << 1);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 1 << 31, 1 << 31);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_PERI_CTRL0_REG, 1 << 30, 1 << 30);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 1 << 24, 1 << 24);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_csi_pat0_ctrl_reg), 0x1ffff << 0, wave_bot << 0);
	clrsetbits32(V821_CCU_AON_BASE + offsetof(struct ccu_aon_reg_t, pll_csi_pat1_ctrl_reg), 1 << 31, 0x1 << 31);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 1 << 27, 0 << 27);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 1 << 29, 1 << 29);
	while(!(read32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG) & (1 << 28)));
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_CSI_CTRL_REG, 1 << 27, 1 << 27);
}

/*
 * Set pll video rate, hosc * N = 1200mhz
 */
static void set_pll_video(void)
{
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 1 << 29, 0 << 29);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 1 << 27, 0 << 27);
	if(detect_hosc())
		clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 0xff << 8, 49 << 8);
	else
		clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 0xff << 8, 29 << 8);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 0x3 << 1, 0x0 << 1);
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 1 << 29, 1 << 29);
	while(!(read32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG) & (1 << 28)));
	clrsetbits32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 1 << 27, 1 << 27);
}

void sys_clock_init(void)
{
	set_pll_peri();
	set_ahb();
	set_apb();
	set_apb_spc();

	if(detect_hosc())
	{
		set_pll_general(V821_CCU_AON_BASE + CCU_AON_PLL_CPU_CTRL_REG, 1, 1, 0, 2, 44);
		if(!(read32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG) & (1 << 31)))
			set_pll_general(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 1, 1, 1, 1, 98);
	}
	else
	{
		set_pll_general(V821_CCU_AON_BASE + CCU_AON_PLL_CPU_CTRL_REG, 1, 1, 0, 2, 26);
		if(!(read32(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG) & (1 << 31)))
			set_pll_general(V821_CCU_AON_BASE + CCU_AON_PLL_VIDEO_CTRL_REG, 1, 1, 3, 1, 117);
	}
	set_pll_e90x();
	set_pll_a27l2();
	set_pll_video();
	set_pll_csi();
}
