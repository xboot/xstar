#ifndef __F133_REG_G2D_H__
#define __F133_REG_G2D_H__

#include <xstarcfg.h>

#define F133_G2D_BASE		(0x05410000)

#define F133_G2D_TOP		(0x00000000)
#define F133_G2D_MIXER	(0x00000100)
#define F133_G2D_BLD		(0x00000400)
#define F133_G2D_V0		(0x00000800)
#define F133_G2D_UI0		(0x00001000)
#define F133_G2D_UI1		(0x00001800)
#define F133_G2D_UI2		(0x00002000)
#define F133_G2D_WB		(0x00003000)
#define F133_G2D_VSU		(0x00008000)
#define F133_G2D_ROT		(0x00028000)
#define F133_G2D_GSU		(0x00030000)

struct g2d_top_t {
	uint32_t G2D_SCLK_GATE;
	uint32_t G2D_HCLK_GATE;
	uint32_t G2D_AHB_RESET;
	uint32_t G2D_SCLK_DIV;
	uint32_t G2D_VERSION;
};

struct g2d_mixer_t {
	uint32_t G2D_MIXER_CTL;
	uint32_t G2D_MIXER_INT;
	uint32_t G2D_MIXER_CLK;
};

struct g2d_v0_t {
	uint32_t V0_ATTR;
	uint32_t V0_MBSIZE;
	uint32_t V0_COOR;
	uint32_t V0_PITCH0;
	uint32_t V0_PITCH1;
	uint32_t V0_PITCH2;
	uint32_t V0_LADD0;
	uint32_t V0_LADD1;
	uint32_t V0_LADD2;
	uint32_t V0_FILLC;
	uint32_t V0_HADD;
	uint32_t V0_SIZE;
	uint32_t V0_HDS_CTL0;
	uint32_t V0_HDS_CTL1;
	uint32_t V0_VDS_CTL0;
	uint32_t V0_VDS_CTL1;
};

struct g2d_ui_t {
	uint32_t UI_ATTR;
	uint32_t UI_MBSIZE;
	uint32_t UI_COOR;
	uint32_t UI_PITCH;
	uint32_t UI_LADD;
	uint32_t UI_FILLC;
	uint32_t UI_HADD;
	uint32_t UI_SIZE;
};

struct g2d_vsu_t {
	uint32_t VS_CTRL;
	uint32_t reserved_0x004[15];
	uint32_t VS_OUT_SIZE;
	uint32_t VS_GLB_ALPHA;
	uint32_t reserved_0x048[14];
	uint32_t VS_Y_SIZE;
	uint32_t reserved_0x084;
	uint32_t VS_Y_HSTEP;
	uint32_t VS_Y_VSTEP;
	uint32_t VS_Y_HPHASE;
	uint32_t reserved_0x094;
	uint32_t VS_Y_VPHASE0;
	uint32_t reserved_0x09C[9];
	uint32_t VS_C_SIZE;
	uint32_t reserved_0x0C4;
	uint32_t VS_C_HSTEP;
	uint32_t VS_C_VSTEP;
	uint32_t VS_C_HPHASE;
	uint32_t reserved_0x0D4;
	uint32_t VS_C_VPHASE0;
	uint32_t reserved_0x0DC[73];
	uint32_t VS_Y_HCOEF[32];
	uint32_t reserved_0x220[32];
	uint32_t VS_Y_VCOEF[32];
	uint32_t reserved_0x320[32];
	uint32_t VS_C_HCOEF[32];
};

struct g2d_bld_t {
	uint32_t BLD_EN_CTL;
	uint32_t reserved_0x004[3];
	uint32_t BLD_FILL_COLOR[2];
	uint32_t reserved_0x018[2];
	uint32_t BLD_CH_ISIZE[2];
	uint32_t reserved_0x028[2];
	uint32_t BLD_CH_OFFSET[2];
	uint32_t reserved_0x038[2];
	uint32_t BLD_PREMUL_CTL;
	uint32_t BLD_BG_COLOR;
	uint32_t BLD_SIZE;
	uint32_t BLD_CTL;
	uint32_t BLD_KEY_CTL;
	uint32_t BLD_KEY_CFG;
	uint32_t BLD_KEY_MAX;
	uint32_t BLD_KEY_MIN;
	uint32_t BLD_OUT_COLOR;
	uint32_t reserved_0x064[7];
	uint32_t ROP_CTL;
	uint32_t ROP_INDEX[2];
	uint32_t reserved_0x08C[29];
	uint32_t BLD_CSC_CTL;
	uint32_t reserved_0x104[3];
	uint32_t BLD_CSC0_COEF00;
	uint32_t BLD_CSC0_COEF01;
	uint32_t BLD_CSC0_COEF02;
	uint32_t BLD_CSC0_CONST0;
	uint32_t BLD_CSC0_COEF10;
	uint32_t BLD_CSC0_COEF11;
	uint32_t BLD_CSC0_COEF12;
	uint32_t BLD_CSC0_CONST1;
	uint32_t BLD_CSC0_COEF20;
	uint32_t BLD_CSC0_COEF21;
	uint32_t BLD_CSC0_COEF22;
	uint32_t BLD_CSC0_CONST2;
	uint32_t BLD_CSC1_COEF00;
	uint32_t BLD_CSC1_COEF01;
	uint32_t BLD_CSC1_COEF02;
	uint32_t BLD_CSC1_CONST0;
	uint32_t BLD_CSC1_COEF10;
	uint32_t BLD_CSC1_COEF11;
	uint32_t BLD_CSC1_COEF12;
	uint32_t BLD_CSC1_CONST1;
	uint32_t BLD_CSC1_COEF20;
	uint32_t BLD_CSC1_COEF21;
	uint32_t BLD_CSC1_COEF22;
	uint32_t BLD_CSC1_CONST2;
	uint32_t BLD_CSC2_COEF00;
	uint32_t BLD_CSC2_COEF01;
	uint32_t BLD_CSC2_COEF02;
	uint32_t BLD_CSC2_CONST0;
	uint32_t BLD_CSC2_COEF10;
	uint32_t BLD_CSC2_COEF11;
	uint32_t BLD_CSC2_COEF12;
	uint32_t BLD_CSC2_CONST1;
	uint32_t BLD_CSC2_COEF20;
	uint32_t BLD_CSC2_COEF21;
	uint32_t BLD_CSC2_COEF22;
	uint32_t BLD_CSC2_CONST2;
};

struct g2d_wb_t {
	uint32_t WB_ATTR;
	uint32_t WB_SIZE;
	uint32_t WB_PITCH0;
	uint32_t WB_PITCH1;
	uint32_t WB_PITCH2;
	uint32_t WB_LADD0;
	uint32_t WB_HADD0;
	uint32_t WB_LADD1;
	uint32_t WB_HADD1;
	uint32_t WB_LADD2;
	uint32_t WB_HADD2;
};

struct g2d_rot_t {
	uint32_t ROT_CTL;
	uint32_t ROT_INT;
	uint32_t ROT_TIMEOUT;
	uint32_t reserved_0x00C[5];
	uint32_t ROT_IFMT;
	uint32_t ROT_ISIZE;
	uint32_t reserved_0x028[2];
	uint32_t ROT_IPITCH0;
	uint32_t ROT_IPITCH1;
	uint32_t ROT_IPITCH2;
	uint32_t reserved_0x03C;
	uint32_t ROT_ILADD0;
	uint32_t ROT_IHADD0;
	uint32_t ROT_ILADD1;
	uint32_t ROT_IHADD1;
	uint32_t ROT_ILADD2;
	uint32_t ROT_IHADD2;
	uint32_t reserved_0x058[11];
	uint32_t ROT_OSIZE;
	uint32_t reserved_0x088[2];
	uint32_t ROT_OPITCH0;
	uint32_t ROT_OPITCH1;
	uint32_t ROT_OPITCH2;
	uint32_t reserved_0x09C;
	uint32_t ROT_OLADD0;
	uint32_t ROT_OHADD0;
	uint32_t ROT_OLADD1;
	uint32_t ROT_OHADD1;
	uint32_t ROT_OLADD2;
	uint32_t ROT_OHADD2;
};

#endif /* __F133_REG_G2D_H__ */
