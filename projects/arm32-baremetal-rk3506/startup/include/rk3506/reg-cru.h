#ifndef __RK3506_REG_CRU_H__
#define __RK3506_REG_CRU_H__

#define RK3506_CRU_BASE		(0xff9a0000)

struct rk3506_cru_t {
	/* cru */
	uint32_t reserved0000[160];   /* offset 0x0   */
	uint32_t mode_con;            /* offset 0x280 */
	uint32_t reserved0284[31];    /* offset 0x284 */
	uint32_t clksel_con[62];      /* offset 0x300 */
	uint32_t reserved03f8[258];   /* offset 0x3F8 */
	uint32_t gate_con[23];        /* offset 0x800 */
	uint32_t reserved085c[105];   /* offset 0x85C */
	uint32_t softrst_con[23];     /* offset 0xA00 */
	uint32_t reserved0a5c[105];   /* offset 0xA5C */
	uint32_t glb_cnt_th;          /* offset 0xC00 */
	uint32_t glb_rst_st;          /* offset 0xC04 */
	uint32_t glb_srst_fst;        /* offset 0xC08 */
	uint32_t glb_srst_snd;        /* offset 0xC0C */
	uint32_t glb_rst_con;         /* offset 0xC10 */
	uint32_t reserved0c14[6];     /* offset 0xC14 */
	uint32_t corewfi_con;         /* offset 0xC2C */
	uint32_t reserved0c30[15604]; /* offset 0xC30 */

	/* pmu cru */
	uint32_t gpll_con[5];         /* offset 0x10000 */
	uint32_t reserved10014[3];    /* offset 0x10014 */
	uint32_t v0pll_con[5];        /* offset 0x10020 */
	uint32_t reserved10034[3];    /* offset 0x10034 */
	uint32_t v1pll_con[5];        /* offset 0x10040 */
	uint32_t reserved10074[171];  /* offset 0x10054 */
	uint32_t pmuclksel_con[7];    /* offset 0x10300 */
	uint32_t reserved1031c[313];  /* offset 0x1031C */
	uint32_t pmugate_con[3];      /* offset 0x10800 */
	uint32_t reserved1080c[125];  /* offset 0x1080C */
	uint32_t pmusoftrst_con[2];   /* offset 0x10A00 */
	uint32_t reserved10a08[7583]; /* offset 0x10A08 */
};

#endif /* __RK3506_REG_CRU_H__ */
