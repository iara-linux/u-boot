// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek clock driver for MT8163 SoC
 *
 * Copyright (c) 2015 MediaTek Inc.
 * James Liao <jamesjj.liao@mediatek.com>
 * Copyright (c) 2025-2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>
#include <dt-bindings/clock/mt8163-clk.h>
#include "clk-mtk.h"

enum {
	CLK_PAD_CLK32K,
	CLK_PAD_CLK26M,
};

static const ulong ext_clock_rates[] = {
	[CLK_PAD_CLK32K] = 32000,
	[CLK_PAD_CLK26M] = 26000000,
};

/* -------------------------------------------------------------------------
 * APMIXEDSYS CLOCKS
 * ------------------------------------------------------------------------- */
#define MT8163_PLL_FMAX		(2500UL * MHZ)
#define CON0_MT8163_RST_BAR	BIT(24)
#define PLL_AO			BIT(1)
#define REG_REF2USB		0x8

#define PLL(_id, _reg, _pwr_reg, _en_mask, _flags, _pcwbits, _pd_reg,	    \
		_pd_shift, _pcw_reg, _pcw_shift, _rst_bar_mask) {	    \
		.id = _id,						    \
		.reg = _reg,						    \
		.pwr_reg = _pwr_reg,					    \
		.en_mask = _en_mask,					    \
		.pd_reg = _pd_reg,					    \
		.pd_shift = _pd_shift,					    \
		.flags = _flags,					    \
		.rst_bar_mask = _rst_bar_mask,				    \
		.fmax = MT8163_PLL_FMAX,				    \
		.pcwbits = _pcwbits,					    \
		.pcwibits = 8,						    \
		.pcw_reg = _pcw_reg,					    \
		.pcw_shift = _pcw_shift,				    \
		.pcw_chg_reg = 0,					    \
	}

static const struct mtk_pll_data apmixed_plls[] = {
	PLL(CLK_APMIXED_ARMPLL, 0x210, 0x21c, 0x1, PLL_AO, 21, 0x214, 24, 0x214, 0, 0),
	PLL(CLK_APMIXED_MAINPLL, 0x220, 0x22c, 0x1, CLK_PLL_HAVE_RST_BAR, 21, 0x220, 4, 0x224, 0, CON0_MT8163_RST_BAR),
	PLL(CLK_APMIXED_UNIVPLL, 0x230, 0x23c, 0x1, CLK_PLL_HAVE_RST_BAR, 21, 0x230, 4, 0x234, 0, CON0_MT8163_RST_BAR),
	PLL(CLK_APMIXED_MMPLL, 0x240, 0x24c, 0x1, 0, 21, 0x244, 24, 0x244, 0, 0),
	PLL(CLK_APMIXED_MSDCPLL, 0x250, 0x25c, 0x1, 0, 21, 0x250, 4, 0x254, 0, 0),
	PLL(CLK_APMIXED_VENCPLL, 0x260, 0x26c, 0x1, 0, 21, 0x260, 4, 0x264, 0, 0),
	PLL(CLK_APMIXED_TVDPLL, 0x270, 0x27c, 0x1, 0, 21, 0x270, 4, 0x274, 0, 0),
	PLL(CLK_APMIXED_MPLL, 0x280, 0x28c, 0x1, 0, 21, 0x280, 4, 0x284, 0, 0),
	PLL(CLK_APMIXED_AUD1PLL, 0x2a0, 0x2ac, 0x1, 0, 31, 0x2a0, 4, 0x2a4, 0, 0),
	PLL(CLK_APMIXED_AUD2PLL, 0x2b0, 0x2bc, 0x1, 0, 31, 0x2b0, 4, 0x2b4, 0, 0),
	PLL(CLK_APMIXED_LVDSPLL, 0x2c0, 0x2cc, 0x1, 0, 21, 0x2c0, 4, 0x2c4, 0, 0),
};

static const struct mtk_gate_regs apmixed_cg_regs = {
	.set_ofs = 0x0,
	.clr_ofs = 0x0,
	.sta_ofs = 0x0,
};

#define GATE_APMIXED(_id, _parent, _shift) {			       \
		.id = _id,					       \
		.parent = _parent,				       \
		.regs = &apmixed_cg_regs,			       \
		.shift = _shift,				       \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_EXT,      \
	}

static const struct mtk_gate apmixed_clks[] = {
	GATE_APMIXED(CLK_APMIXED_SSUSB_26M, CLK_PAD_CLK26M, 4),
	GATE_APMIXED(CLK_APMIXED_ARMPLL_26M, CLK_PAD_CLK26M, 5),
	GATE_APMIXED(CLK_APMIXED_MIPI_26M, CLK_PAD_CLK26M, 6),
};

static const struct mtk_ref2usb_tx apmixed_ref2usb_clk = \
	REF2USB_TX(CLK_APMIXED_REF2USB_TX, CLK_PAD_CLK26M, REG_REF2USB,
		   CLK_PARENT_EXT);

static const struct mtk_clk_tree mt8163_apmixed_tree = {
	.type = MTK_CLK_TREE_APMIXED,
	.pll_parent = EXT_PARENT(CLK_PAD_CLK26M),
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.plls = apmixed_plls,
	.num_plls = ARRAY_SIZE(apmixed_plls),
	.gates_offs = CLK_APMIXED_SSUSB_26M,
	.gates = apmixed_clks,
	.num_gates = ARRAY_SIZE(apmixed_clks),
	.ref2usb_tx = &apmixed_ref2usb_clk,
	.ref2usb_tx_offs = CLK_APMIXED_REF2USB_TX,
};

/* -------------------------------------------------------------------------
 * TOPCKGEN CLOCKS
 * ------------------------------------------------------------------------- */
#define FIXED_CLK0(_id, _rate)						\
	FIXED_CLK(_id, CLK_PAD_CLK26M, CLK_PARENT_EXT, _rate)

static const struct mtk_fixed_clk top_fixed_clks[] = {
	FIXED_CLK0(CLK_TOP_DMPLL, 0),
	FIXED_CLK0(CLK_TOP_DSI0_LNTC_DSICK, 75000000),
	FIXED_CLK0(CLK_TOP_F26M_MEM_CKGEN_OCC, 26000000),
	FIXED_CLK0(CLK_TOP_LVDSTX_CLKDIG_CTS, 52500000),
	FIXED_CLK0(CLK_TOP_HDMI_CTS, 0),
};

#define PLL_FACTOR(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_APMIXED)

#define PLL_FACTOR1(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_TOPCKGEN)

#define PLL_FACTOR2(_id, _parent, _mult, _div)			\
	FACTOR(_id, _parent, _mult, _div, CLK_PARENT_EXT)

static const struct mtk_fixed_factor top_divs[] = {
	PLL_FACTOR(CLK_TOP_MAIN_H546M, CLK_APMIXED_MAINPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_MAIN_H364M, CLK_APMIXED_MAINPLL, 1, 3),
	PLL_FACTOR(CLK_TOP_MAIN_H218P4M, CLK_APMIXED_MAINPLL, 1, 5),
	PLL_FACTOR(CLK_TOP_MAIN_H156M, CLK_APMIXED_MAINPLL, 1, 7),

	PLL_FACTOR(CLK_TOP_UNIV_624M, CLK_APMIXED_UNIVPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_UNIV_416M, CLK_APMIXED_UNIVPLL, 1, 3),
	PLL_FACTOR(CLK_TOP_UNIV_249P6M, CLK_APMIXED_UNIVPLL, 1, 5),
	PLL_FACTOR(CLK_TOP_UNIV_178P3M, CLK_APMIXED_UNIVPLL, 1, 7),
	PLL_FACTOR(CLK_TOP_UNIV_48M, CLK_APMIXED_UNIVPLL, 1, 26),

	PLL_FACTOR2(CLK_TOP_CLKRTC_INT, CLK_PAD_CLK26M, 1, 793),

	PLL_FACTOR1(CLK_TOP_HDMI_CTS_D2, CLK_TOP_HDMI_CTS, 1, 3),
	PLL_FACTOR1(CLK_TOP_HDMI_CTS_D3, CLK_TOP_HDMI_CTS, 1, 2),

	PLL_FACTOR(CLK_TOP_LVDSPLL_D2, CLK_APMIXED_LVDSPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_LVDSPLL_D4, CLK_APMIXED_LVDSPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_LVDSPLL_D8, CLK_APMIXED_LVDSPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_LVDSPLL_ETH, CLK_APMIXED_LVDSPLL, 1, 4),

	PLL_FACTOR(CLK_TOP_MSDCPLL_D2, CLK_APMIXED_MSDCPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_MSDCPLL_D4, CLK_APMIXED_MSDCPLL, 1, 4),

	PLL_FACTOR1(CLK_TOP_SYSPLL_D2P5, CLK_TOP_MAIN_H218P4M, 2, 1),
	PLL_FACTOR1(CLK_TOP_SYSPLL1_D2, CLK_TOP_MAIN_H546M, 1, 2),
	PLL_FACTOR1(CLK_TOP_SYSPLL1_D4, CLK_TOP_MAIN_H546M, 1, 4),
	PLL_FACTOR1(CLK_TOP_SYSPLL1_D8, CLK_TOP_MAIN_H546M, 1, 8),
	PLL_FACTOR1(CLK_TOP_SYSPLL1_D16, CLK_TOP_MAIN_H546M, 1, 16),
	PLL_FACTOR1(CLK_TOP_SYSPLL2_D2, CLK_TOP_MAIN_H364M, 1, 2),
	PLL_FACTOR1(CLK_TOP_SYSPLL2_D4, CLK_TOP_MAIN_H364M, 1, 4),
	PLL_FACTOR1(CLK_TOP_SYSPLL2_D8, CLK_TOP_MAIN_H364M, 1, 8),
	PLL_FACTOR1(CLK_TOP_SYSPLL3_D2, CLK_TOP_MAIN_H218P4M, 1, 2),
	PLL_FACTOR1(CLK_TOP_SYSPLL3_D4, CLK_TOP_MAIN_H218P4M, 1, 4),
	PLL_FACTOR1(CLK_TOP_SYSPLL4_D2, CLK_TOP_MAIN_H156M, 1, 2),
	PLL_FACTOR1(CLK_TOP_SYSPLL4_D4, CLK_TOP_MAIN_H156M, 1, 4),

	PLL_FACTOR(CLK_TOP_TVDPLL_D2, CLK_APMIXED_TVDPLL, 1, 2),
	PLL_FACTOR(CLK_TOP_TVDPLL_D4, CLK_APMIXED_TVDPLL, 1, 4),
	PLL_FACTOR(CLK_TOP_TVDPLL_D8, CLK_APMIXED_TVDPLL, 1, 8),
	PLL_FACTOR(CLK_TOP_TVDPLL_D16, CLK_APMIXED_TVDPLL, 1, 16),

	PLL_FACTOR1(CLK_TOP_UNIVPLL_D2P5, CLK_TOP_UNIV_249P6M, 2, 1),
	PLL_FACTOR1(CLK_TOP_UNIVPLL1_D2, CLK_TOP_UNIV_624M, 1, 2),
	PLL_FACTOR1(CLK_TOP_UNIVPLL1_D4, CLK_TOP_UNIV_624M, 1, 4),
	PLL_FACTOR1(CLK_TOP_UNIVPLL2_D2, CLK_TOP_UNIV_416M, 1, 2),
	PLL_FACTOR1(CLK_TOP_UNIVPLL2_D4, CLK_TOP_UNIV_416M, 1, 4),
	PLL_FACTOR1(CLK_TOP_UNIVPLL2_D8, CLK_TOP_UNIV_416M, 1, 8),
	PLL_FACTOR1(CLK_TOP_UNIVPLL3_D2, CLK_TOP_UNIV_249P6M, 1, 2),
	PLL_FACTOR1(CLK_TOP_UNIVPLL3_D4, CLK_TOP_UNIV_249P6M, 1, 4),
	PLL_FACTOR1(CLK_TOP_UNIVPLL3_D8, CLK_TOP_UNIV_249P6M, 1, 8),
};

static const struct mtk_parent axi_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D4),
};

static const struct mtk_parent mem_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_DMPLL),
	TOP_PARENT(CLK_TOP_F26M_MEM_CKGEN_OCC),
};

static const struct mtk_parent ddrphycfg_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D8),
};

static const struct mtk_parent mm_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAIN_H364M),
	TOP_PARENT(CLK_TOP_UNIV_416M),
	TOP_PARENT(CLK_APMIXED_VENCPLL),
};

static const struct mtk_parent pwm_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D2),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D4),
};

static const struct mtk_parent vdec_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAIN_H364M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D2),
	TOP_PARENT(CLK_TOP_UNIVPLL1_D2),
	TOP_PARENT(CLK_TOP_SYSPLL1_D4),
};

static const struct mtk_parent mfg_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_APMIXED_MMPLL),
	TOP_PARENT(CLK_TOP_UNIV_416M),
	TOP_PARENT(CLK_TOP_MAIN_H364M),
};

static const struct mtk_parent camtg_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIV_48M),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D2),
};

static const struct mtk_parent uart_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D8),
};

static const struct mtk_parent spi_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL3_D2),
};

static const struct mtk_parent msdc30_0_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D2),
	TOP_PARENT(CLK_TOP_MSDCPLL_D2),
	TOP_PARENT(CLK_TOP_UNIVPLL1_D4),
	TOP_PARENT(CLK_TOP_SYSPLL2_D2),
	TOP_PARENT(CLK_TOP_MAIN_H156M),
	TOP_PARENT(CLK_TOP_UNIV_178P3M),
};

static const struct mtk_parent msdc50_3_h_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D2),
	TOP_PARENT(CLK_TOP_SYSPLL2_D2),
	TOP_PARENT(CLK_TOP_SYSPLL4_D2),
};

static const struct mtk_parent msdc50_3_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_APMIXED_MSDCPLL),
	TOP_PARENT(CLK_TOP_MSDCPLL_D2),
	TOP_PARENT(CLK_TOP_UNIVPLL1_D4),
	TOP_PARENT(CLK_TOP_SYSPLL2_D2),
	TOP_PARENT(CLK_TOP_MAIN_H156M),
	TOP_PARENT(CLK_TOP_MSDCPLL_D4),
	TOP_PARENT(CLK_TOP_UNIV_624M),
	TOP_PARENT(CLK_TOP_UNIVPLL1_D2),
};

static const struct mtk_parent audio_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL3_D4),
	TOP_PARENT(CLK_TOP_SYSPLL4_D4),
	TOP_PARENT(CLK_TOP_SYSPLL1_D16),
};

static const struct mtk_parent aud_intbus_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D4),
	TOP_PARENT(CLK_TOP_SYSPLL4_D2),
};

static const struct mtk_parent pmicspi_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIV_48M),
};

static const struct mtk_parent scp_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_APMIXED_MPLL),
	TOP_PARENT(CLK_TOP_SYSPLL1_D4),
};

static const struct mtk_parent atb_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D2),
};

static const struct mtk_parent mjc_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_MAIN_H218P4M),
	TOP_PARENT(CLK_TOP_UNIV_249P6M),
};

static const struct mtk_parent dpi0_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_APMIXED_LVDSPLL),
	TOP_PARENT(CLK_TOP_LVDSPLL_D2),
	TOP_PARENT(CLK_TOP_LVDSPLL_D4),
	TOP_PARENT(CLK_TOP_LVDSPLL_D8),
};

static const struct mtk_parent scam_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL3_D2),
};

static const struct mtk_parent aud_1_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_APMIXED_AUD1PLL),
};

static const struct mtk_parent aud_2_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_APMIXED_AUD2PLL),
};

static const struct mtk_parent dpi1_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_TVDPLL_D2),
	TOP_PARENT(CLK_TOP_TVDPLL_D4),
	TOP_PARENT(CLK_TOP_TVDPLL_D8),
	TOP_PARENT(CLK_TOP_TVDPLL_D16),
};

static const struct mtk_parent ufoenc_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIV_624M),
	TOP_PARENT(CLK_TOP_MAIN_H546M),
	TOP_PARENT(CLK_TOP_UNIVPLL_D2P5),
	TOP_PARENT(CLK_TOP_SYSPLL_D2P5),
	TOP_PARENT(CLK_TOP_UNIV_416M),
	TOP_PARENT(CLK_TOP_MAIN_H364M),
	TOP_PARENT(CLK_TOP_SYSPLL1_D2),
};

static const struct mtk_parent eth_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL3_D4),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D8),
	TOP_PARENT(CLK_TOP_LVDSPLL_ETH),
	TOP_PARENT(CLK_TOP_UNIV_48M),
	TOP_PARENT(CLK_TOP_SYSPLL2_D8),
	TOP_PARENT(CLK_TOP_SYSPLL4_D4),
	TOP_PARENT(CLK_TOP_UNIVPLL3_D8),
	EXT_PARENT(CLK_PAD_CLK26M),
};

static const struct mtk_parent onfi_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_SYSPLL2_D2),
	TOP_PARENT(CLK_TOP_MAIN_H156M),
	TOP_PARENT(CLK_TOP_UNIVPLL3_D2),
	TOP_PARENT(CLK_TOP_SYSPLL2_D4),
	TOP_PARENT(CLK_TOP_UNIVPLL3_D4),
	TOP_PARENT(CLK_TOP_SYSPLL4_D4),
};

static const struct mtk_parent snfi_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D8),
	TOP_PARENT(CLK_TOP_UNIVPLL3_D4),
	TOP_PARENT(CLK_TOP_SYSPLL4_D2),
	TOP_PARENT(CLK_TOP_UNIVPLL2_D4),
	TOP_PARENT(CLK_TOP_UNIVPLL3_D2),
	TOP_PARENT(CLK_TOP_SYSPLL1_D4),
	TOP_PARENT(CLK_TOP_UNIVPLL1_D4),
	EXT_PARENT(CLK_PAD_CLK26M),
};

static const struct mtk_parent hdmi_parents[] = {
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_HDMI_CTS),
	TOP_PARENT(CLK_TOP_HDMI_CTS_D2),
	TOP_PARENT(CLK_TOP_HDMI_CTS_D3),
};

static const struct mtk_parent rtc_parents[] = {
	TOP_PARENT(CLK_TOP_CLKRTC_INT),
	EXT_PARENT(CLK_PAD_CLK32K),
	EXT_PARENT(CLK_PAD_CLK26M),
	TOP_PARENT(CLK_TOP_UNIVPLL3_D8),
};

static const struct mtk_composite top_muxes[] = {
	/* CLK_CFG_0 */
	MUX_GATE(CLK_TOP_AXI_SEL, axi_parents, 0x0040, 0, 1, 7),
	MUX_GATE(CLK_TOP_MEM_SEL, mem_parents, 0x0040, 8, 2, 15),
	MUX_GATE(CLK_TOP_DDRPHYCFG_SEL, ddrphycfg_parents, 0x0040, 16, 1, 23),
	MUX_GATE(CLK_TOP_MM_SEL, mm_parents, 0x0040, 24, 2, 31),

	/* CLK_CFG_1 */
	MUX_GATE(CLK_TOP_PWM_SEL, pwm_parents, 0x0050, 0, 2, 7),
	MUX_GATE(CLK_TOP_VDEC_SEL, vdec_parents, 0x0050, 8, 3, 15),
	MUX_GATE(CLK_TOP_MFG_SEL, mfg_parents, 0x0050, 24, 2, 31),

	/* CLK_CFG_2 */
	MUX_GATE(CLK_TOP_CAMTG_SEL, camtg_parents, 0x0060, 0, 2, 7),
	MUX_GATE(CLK_TOP_UART_SEL, uart_parents, 0x0060, 8, 1, 15),
	MUX_GATE(CLK_TOP_SPI_SEL, spi_parents, 0x0060, 16, 1, 23),

	/* CLK_CFG_3 */
	MUX_GATE(CLK_TOP_MSDC30_0_SEL, msdc30_0_parents, 0x0070, 0, 3, 7),
	MUX_GATE(CLK_TOP_MSDC30_1_SEL, msdc30_0_parents, 0x0070, 8, 3, 15),
	MUX_GATE(CLK_TOP_MSDC30_2_SEL, msdc30_0_parents, 0x0070, 16, 3, 23),

	/* CLK_CFG_4 */
	MUX_GATE(CLK_TOP_MSDC50_3_HSEL, msdc50_3_h_parents, 0x0080, 0, 2, 7),
	MUX_GATE(CLK_TOP_MSDC50_3_SEL, msdc50_3_parents, 0x0080, 8, 4, 15),
	MUX_GATE(CLK_TOP_AUDIO_SEL, audio_parents, 0x0080, 16, 2, 23),
	MUX_GATE(CLK_TOP_AUD_INTBUS_SEL, aud_intbus_parents, 0x0080, 24, 2, 31),

	/* CLK_CFG_5 */
	MUX_GATE(CLK_TOP_PMICSPI_SEL, pmicspi_parents, 0x0090, 0, 1, 7),
	MUX_GATE(CLK_TOP_SCP_SEL, scp_parents, 0x0090, 8, 2, 15),
	MUX_GATE(CLK_TOP_ATB_SEL, atb_parents, 0x0090, 16, 1, 23),
	MUX_GATE(CLK_TOP_MJC_SEL, mjc_parents, 0x0090, 24, 2, 31),

	/* CLK_CFG_6 */
	MUX_GATE(CLK_TOP_DPI0_SEL, dpi0_parents, 0x00a0, 0, 3, 7),
	MUX_GATE(CLK_TOP_SCAM_SEL, scam_parents, 0x00a0, 8, 1, 15),
	MUX_GATE(CLK_TOP_AUD_1_SEL, aud_1_parents, 0x00a0, 16, 1, 23),
	MUX_GATE(CLK_TOP_AUD_2_SEL, aud_2_parents, 0x00a0, 24, 1, 31),

	/* CLK_CFG_7 */
	MUX_GATE(CLK_TOP_DPI1_SEL, dpi1_parents, 0x00b0, 0, 3, 7),
	MUX_GATE(CLK_TOP_UFOENC_SEL, ufoenc_parents, 0x00b0, 8, 3, 15),
	MUX_GATE(CLK_TOP_UFODEC_SEL, ufoenc_parents, 0x00b0, 16, 3, 23),
	MUX_GATE(CLK_TOP_ETH_SEL, eth_parents, 0x00b0, 24, 4, 31),

	/* CLK_CFG_8 */
	MUX_GATE(CLK_TOP_ONFI_SEL, onfi_parents, 0x00c0, 0, 3, 7),
	MUX_GATE(CLK_TOP_SNFI_SEL, snfi_parents, 0x00c0, 8, 2, 15),
	MUX_GATE(CLK_TOP_HDMI_SEL, hdmi_parents, 0x00c0, 16, 2, 23),
	MUX_GATE(CLK_TOP_RTC_SEL, rtc_parents, 0x00c0, 24, 2, 31),
};

static const struct mtk_gate_regs top_cg_regs = {
	.set_ofs = 0x104,
	.clr_ofs = 0x104,
	.sta_ofs = 0x104,
};

#define GATE_TOP(_id, _parent, _shift) {			       \
		.id = _id,					       \
		.parent = _parent,				       \
		.regs = &top_cg_regs,				       \
		.shift = _shift,				       \
		.flags = CLK_GATE_NO_SETCLR_INV | CLK_PARENT_APMIXED,  \
	}

static const struct mtk_gate top_clk_gates[] = {
	GATE_TOP(CLK_TOP_ARMPLL_DIVIDER_PLL1_EN, CLK_APMIXED_MAINPLL, 4),
	GATE_TOP(CLK_TOP_ARMPLL_DIVIDER_PLL2_EN, CLK_APMIXED_UNIVPLL, 5),
};

static const struct mtk_clk_tree mt8163_topckgen_tree = {
	.type = MTK_CLK_TREE_TOPCKGEN,
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.fdivs_offs = CLK_TOP_MAIN_H546M,
	.muxes_offs = CLK_TOP_AXI_SEL,
	.gates_offs = CLK_TOP_ARMPLL_DIVIDER_PLL1_EN,
	.fclks = top_fixed_clks,
	.fdivs = top_divs,
	.muxes = top_muxes,
	.gates = top_clk_gates,
	.num_fclks = ARRAY_SIZE(top_fixed_clks),
	.num_fdivs = ARRAY_SIZE(top_divs),
	.num_muxes = ARRAY_SIZE(top_muxes),
	.num_gates = ARRAY_SIZE(top_clk_gates),
};

/* -------------------------------------------------------------------------
 * INFRACFG CLOCKS
 * ------------------------------------------------------------------------- */
static const struct mtk_gate_regs infra0_cg_regs = {
	.set_ofs = 0x0080,
	.clr_ofs = 0x0084,
	.sta_ofs = 0x0090,
};

static const struct mtk_gate_regs infra1_cg_regs = {
	.set_ofs = 0x0088,
	.clr_ofs = 0x008c,
	.sta_ofs = 0x0094,
};

#define GATE_IFR0(_id, _parent, _shift)				\
	{							\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra0_cg_regs,			\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_IFR1(_id, _parent, _shift)				\
	{							\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra1_cg_regs,			\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_TOPCKGEN,	\
	}

#define GATE_IFR0_EXT(_id, _parent, _shift)			\
	{							\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra0_cg_regs,			\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_EXT,	\
	}

#define GATE_IFR1_EXT(_id, _parent, _shift)			\
	{							\
		.id = _id,					\
		.parent = _parent,				\
		.regs = &infra1_cg_regs,			\
		.shift = _shift,				\
		.flags = CLK_GATE_SETCLR | CLK_PARENT_EXT,	\
	}

static const struct mtk_gate ifr_clks[] = {
	/* INFRA0 */
	GATE_IFR0_EXT(CLK_INFRA_PMIC_TMR, CLK_PAD_CLK26M, 0),
	GATE_IFR0_EXT(CLK_INFRA_PMIC_AP, CLK_PAD_CLK26M, 1),
	GATE_IFR0_EXT(CLK_INFRA_PMIC_MD, CLK_PAD_CLK26M, 2),
	GATE_IFR0_EXT(CLK_INFRA_PMIC_CONN, CLK_PAD_CLK26M, 3),
	GATE_IFR0(CLK_INFRA_SCPSYS, CLK_TOP_SCP_SEL, 4),
	GATE_IFR0(CLK_INFRA_SEJ, CLK_TOP_AXI_SEL, 5),
	GATE_IFR0(CLK_INFRA_APXGPT, CLK_TOP_AXI_SEL, 6),
	GATE_IFR0(CLK_INFRA_USB, CLK_TOP_AXI_SEL, 7),
	GATE_IFR0(CLK_INFRA_ICUSB, CLK_TOP_AXI_SEL, 8),
	GATE_IFR0(CLK_INFRA_GCE, CLK_TOP_AXI_SEL, 9),
	GATE_IFR0(CLK_INFRA_THERM, CLK_TOP_AXI_SEL, 10),
	GATE_IFR0(CLK_INFRA_I2C0, CLK_TOP_AXI_SEL, 11),
	GATE_IFR0(CLK_INFRA_I2C1, CLK_TOP_AXI_SEL, 12),
	GATE_IFR0(CLK_INFRA_I2C2, CLK_TOP_AXI_SEL, 13),
	GATE_IFR0(CLK_INFRA_PWM_HCLK, CLK_TOP_AXI_SEL, 15),
	GATE_IFR0(CLK_INFRA_PWM1, CLK_TOP_AXI_SEL, 16),
	GATE_IFR0(CLK_INFRA_PWM2, CLK_TOP_AXI_SEL, 17),
	GATE_IFR0(CLK_INFRA_PWM3, CLK_TOP_AXI_SEL, 18),
	GATE_IFR0(CLK_INFRA_PWM, CLK_TOP_AXI_SEL, 21),
	GATE_IFR0(CLK_INFRA_UART0, CLK_TOP_UART_SEL, 22),
	GATE_IFR0(CLK_INFRA_UART1, CLK_TOP_UART_SEL, 23),
	GATE_IFR0(CLK_INFRA_UART2, CLK_TOP_UART_SEL, 24),
	GATE_IFR0(CLK_INFRA_UART3, CLK_TOP_UART_SEL, 25),
	GATE_IFR0(CLK_INFRA_USB_MCU, CLK_TOP_AXI_SEL, 26),
	GATE_IFR0(CLK_INFRA_NFI_ECC_66M, CLK_TOP_AXI_SEL, 27),
	GATE_IFR0(CLK_INFRA_NFI_66M, CLK_TOP_AXI_SEL, 28),
	GATE_IFR0(CLK_INFRA_BTIF, CLK_TOP_AXI_SEL, 31),

	/* INFRA1 */
	GATE_IFR1(CLK_INFRA_SPI, CLK_TOP_SPI_SEL, 1),
	GATE_IFR1(CLK_INFRA_MSDC3, CLK_TOP_MSDC50_3_SEL, 2),
	GATE_IFR1(CLK_INFRA_MSDC1, CLK_TOP_MSDC30_1_SEL, 4),
	GATE_IFR1(CLK_INFRA_MSDC2, CLK_TOP_MSDC30_2_SEL, 5),
	GATE_IFR1(CLK_INFRA_MSDC0, CLK_TOP_MSDC30_0_SEL, 6),
	GATE_IFR1(CLK_INFRA_GCPU, CLK_TOP_AXI_SEL, 8),
	GATE_IFR1(CLK_INFRA_TRNG, CLK_TOP_AXI_SEL, 9),
	GATE_IFR1_EXT(CLK_INFRA_AUXADC, CLK_PAD_CLK26M, 10),
	GATE_IFR1(CLK_INFRA_CPUM, CLK_TOP_AXI_SEL, 11),
	GATE_IFR1(CLK_INFRA_IRRX, CLK_TOP_RTC_SEL, 12),
	GATE_IFR1(CLK_INFRA_UFO, CLK_TOP_AXI_SEL, 13),
	GATE_IFR1(CLK_INFRA_CEC, CLK_TOP_RTC_SEL, 14),
	GATE_IFR1_EXT(CLK_INFRA_CEC_26M, CLK_PAD_CLK26M, 15),
	GATE_IFR1(CLK_INFRA_NFI_BCLK, CLK_TOP_SPI_SEL, 16),
	GATE_IFR1(CLK_INFRA_NFI_ECC, CLK_TOP_SPI_SEL, 17),
	GATE_IFR1(CLK_INFRA_AP_DMA, CLK_TOP_AXI_SEL, 18),
	GATE_IFR1(CLK_INFRA_XIU, CLK_TOP_AXI_SEL, 19),
	GATE_IFR1(CLK_INFRA_DEVICE_APC, CLK_TOP_AXI_SEL, 20),
	GATE_IFR1(CLK_INFRA_XIU2AHB, CLK_TOP_AXI_SEL, 21),
	GATE_IFR1(CLK_INFRA_L2C_SRAM, CLK_TOP_AXI_SEL, 22),
	GATE_IFR1(CLK_INFRA_ETH_50M, CLK_TOP_ETH_SEL, 23),
	GATE_IFR1(CLK_INFRA_DEBUGSYS, CLK_TOP_AXI_SEL, 24),
	GATE_IFR1(CLK_INFRA_AUDIO, CLK_TOP_AXI_SEL, 25),
	GATE_IFR1(CLK_INFRA_ETH_25M, CLK_TOP_ETH_SEL, 26),
	GATE_IFR1(CLK_INFRA_NFI, CLK_TOP_AXI_SEL, 27),
	GATE_IFR1(CLK_INFRA_ONFI, CLK_TOP_ONFI_SEL, 28),
	GATE_IFR1(CLK_INFRA_SNFI, CLK_TOP_SNFI_SEL, 29),
	GATE_IFR1(CLK_INFRA_ETH, CLK_TOP_AXI_SEL, 30),
	GATE_IFR1_EXT(CLK_INFRA_DRAMC_F26M, CLK_PAD_CLK26M, 31),
};

static const struct mtk_clk_tree mt8163_infracfg_tree = {
	.type = MTK_CLK_TREE_INFRASYS,
	.ext_clk_rates = ext_clock_rates,
	.num_ext_clks = ARRAY_SIZE(ext_clock_rates),
	.gates = ifr_clks,
	.num_gates = ARRAY_SIZE(ifr_clks),
};

/* -------------------------------------------------------------------------
 * PROBE / DRIVER IMPLEMENTATION
 * ------------------------------------------------------------------------- */
static const struct udevice_id mt8163_apmixed_compat[] = {
	{
		.compatible = "mediatek,mt8163-apmixedsys",
		.data = (ulong)&mt8163_apmixed_tree,
	},
	{ }
};

static const struct udevice_id mt8163_topckgen_compat[] = {
	{
		.compatible = "mediatek,mt8163-topckgen",
		.data = (ulong)&mt8163_topckgen_tree,
	},
	{ }
};

static const struct udevice_id mt8163_infracfg_compat[] = {
	{
		.compatible = "mediatek,mt8163-infracfg",
		.data = (ulong)&mt8163_infracfg_tree,
	},
	{ }
};

U_BOOT_DRIVER(mtk_clk_apmixedsys) = {
	.name = "mt8163-apmixedsys",
	.id = UCLASS_CLK,
	.of_match = mt8163_apmixed_compat,
	.probe = mtk_clk_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_apmixedsys_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_topckgen) = {
	.name = "mt8163-topckgen",
	.id = UCLASS_CLK,
	.of_match = mt8163_topckgen_compat,
	.probe = mtk_clk_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

U_BOOT_DRIVER(mtk_clk_infracfg) = {
	.name = "mt8163-infracfg",
	.id = UCLASS_CLK,
	.of_match = mt8163_infracfg_compat,
	.probe = mtk_clk_probe,
	.priv_auto = sizeof(struct mtk_clk_priv),
	.ops = &mtk_clk_topckgen_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
