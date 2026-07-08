#ifndef __V821_REG_CCU_H__
#define __V821_REG_CCU_H__

#define V821_CCU_AON_BASE			(0x4a010000)
#define V821_CCU_APP_BASE			(0x42001000)
#define V821_CCU_PRCM_BASE			(0x4a000000)

#define CCU_AON_PLL_CPU_CTRL_REG	(0x0000)
#define CCU_AON_PLL_PERI_CTRL0_REG	(0x0020)
#define CCU_AON_PLL_PERI_CTRL1_REG	(0x0024)
#define CCU_AON_PLL_VIDEO_CTRL_REG	(0x0040)
#define CCU_AON_PLL_CSI_CTRL_REG	(0x0048)
#define CCU_AON_PLL_AUDIO_CTRL_REG	(0x0078)
#define CCU_AON_PLL_DDR_CTRL_REG	(0x0080)

struct ccu_aon_reg_t {
	uint32_t pll_cpu_ctrl_reg;
	uint32_t pad_until_0x0020[7];
	uint32_t pll_peri_ctrl0_reg;
	uint32_t pll_peri_ctrl1_reg;
	uint32_t pad_until_0x0040[6];
	uint32_t pll_video_ctrl_reg;
	uint32_t pad_until_0x0048[1];
	uint32_t pll_csi_ctrl_reg;
	uint32_t pad_until_0x0078[11];
	uint32_t pll_audio_ctrl_reg;
	uint32_t pad_until_0x0080[1];
	uint32_t pll_ddr_ctrl_reg;
	uint32_t pad_until_0x0120[39];
	uint32_t pll_peri_pat0_ctrl_reg;
	uint32_t pll_peri_pat1_ctrl_reg;
	uint32_t pad_until_0x0140[6];
	uint32_t pll_video_pat0_ctrl_reg;
	uint32_t pll_video_pat1_ctrl_reg;
	uint32_t pll_csi_pat0_ctrl_reg;
	uint32_t pll_csi_pat1_ctrl_reg;
	uint32_t pad_until_0x0178[10];
	uint32_t pll_audio_pat0_ctrl_reg;
	uint32_t pll_audio_pat1_ctrl_reg;
	uint32_t pll_ddr_pat0_ctrl_reg;
	uint32_t pll_ddr_pat1_ctrl_reg;
	uint32_t pad_until_0x0300[94];
	uint32_t pll_cpu_bias_reg;
	uint32_t pad_until_0x0320[7];
	uint32_t pll_peri_bias_reg;
	uint32_t pad_until_0x0340[7];
	uint32_t pll_video_bias_reg;
	uint32_t pad_until_0x0348[1];
	uint32_t pll_csi_bias_reg;
	uint32_t pad_until_0x0378[11];
	uint32_t pll_audio_bias_reg;
	uint32_t pad_until_0x0380[1];
	uint32_t pll_ddr_bias_reg;
	uint32_t pad_until_0x0400[31];
	uint32_t pll_cpu_tun_reg;
	uint32_t pll_func_cfg_reg;
	uint32_t hosc_freq_det;
	uint32_t reg_clock_01;
	uint32_t pad_until_0x0500[60];
	uint32_t ahb_clk_reg;
	uint32_t apb_clk_reg;
	uint32_t rtc_apb_clk_reg;
	uint32_t pad_until_0x0510[1];
	uint32_t dcxo_cnt_reg;
	uint32_t pad_until_0x0518[1];
	uint32_t wlan_bus_rstn_reg;
	uint32_t pad_until_0x0550[13];
	uint32_t bus_clk_gating_reg;
	uint32_t pad_until_0x0570[7];
	uint32_t dcxo_cfg_reg;
	uint32_t dcxo_cfg1_reg;
	uint32_t pad_until_0x0580[2];
	uint32_t apb_spec_clk_reg;
	uint32_t e907_clk_reg;
	uint32_t a27l2_clk_reg;
	uint32_t ck_test_div_reg;
};

struct ccu_app_reg_t {
	uint32_t pad_until_0x0004[1];
	uint32_t dram_clk_reg;
	uint32_t pad_until_0x000c[1];
	uint32_t e907_ts_clock_reg;
	uint32_t a27l2_mt_clock_reg;
	uint32_t smhc_ctrl0_clk_reg;
	uint32_t ss_clk_reg;
	uint32_t spi_clk_reg;
	uint32_t spif_clk_reg;
	uint32_t mcsi_clk_reg;
	uint32_t csi_master0_clk_reg;
	uint32_t csi_master1_clk_reg;
	uint32_t spi2_clk_reg;
	uint32_t tcon_lcd_clk_reg;
	uint32_t de_clk_reg;
	uint32_t g2d_clk_reg;
	uint32_t gpadc_clk_reg;
	uint32_t ve_clk_reg;
	uint32_t audio_dac_clk_reg;
	uint32_t audio_adc_clk_reg;
	uint32_t dmic_clk_reg;
	uint32_t i2s0_clk_reg;
	uint32_t i2s1_clk_reg;
	uint32_t smhc_ctrl1_clk_reg;
	uint32_t audio_div_clk_reg;
	uint32_t spi1_clk_reg;
	uint32_t e907_r_clk_reg;
	uint32_t gmac_clk_fanout_reg;
	uint32_t gmac_clk_fanout_cfg_reg;
	uint32_t gmac_25m_clk_reg;
	uint32_t pad_until_0x007c[1];
	uint32_t ccu_app_clk_reg;
	uint32_t bus_clk_gating0_reg;
	uint32_t bus_clk_gating1_reg;
	uint32_t bus_clk_gating2_reg;
	uint32_t pad_until_0x0090[1];
	uint32_t bus_reset0_reg;
	uint32_t bus_reset1_reg;
	uint32_t rv_wdg_reset_reg;
	uint32_t e907_rstn_reg;
};

#endif /* __V821_REG_CCU_H__ */
