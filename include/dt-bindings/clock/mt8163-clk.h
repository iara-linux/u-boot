/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2015 MediaTek Inc.
 * James Liao <jamesjj.liao@mediatek.com>
 * Copyright (c) 2025-2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#ifndef _DT_BINDINGS_CLK_MT8163_H
#define _DT_BINDINGS_CLK_MT8163_H

/* --------------------------------------------------------------------- */
/* APMIXEDSYS                                                            */
/* --------------------------------------------------------------------- */

#define CLK_APMIXED_ARMPLL          0
#define CLK_APMIXED_MAINPLL         1
#define CLK_APMIXED_UNIVPLL         2
#define CLK_APMIXED_MMPLL           3
#define CLK_APMIXED_MSDCPLL         4
#define CLK_APMIXED_VENCPLL         5
#define CLK_APMIXED_TVDPLL          6
#define CLK_APMIXED_MPLL            7
#define CLK_APMIXED_AUD1PLL         8
#define CLK_APMIXED_AUD2PLL         9
#define CLK_APMIXED_LVDSPLL         10

#define CLK_APMIXED_SSUSB_26M       11
#define CLK_APMIXED_ARMPLL_26M      12
#define CLK_APMIXED_MIPI_26M        13
#define CLK_APMIXED_REF2USB_TX      14

#define CLK_APMIXED_NR_CLK          15

/* --------------------------------------------------------------------- */
/* TOPCKGEN                                                              */
/* --------------------------------------------------------------------- */

/* TOPCKGEN fixed clocks */
#define CLK_TOP_DMPLL                   0
#define CLK_TOP_DSI0_LNTC_DSICK         1
#define CLK_TOP_F26M_MEM_CKGEN_OCC      2
#define CLK_TOP_LVDSTX_CLKDIG_CTS       3
#define CLK_TOP_HDMI_CTS                4

/* TOPCKGEN fixed factors / dividers */
#define CLK_TOP_MAIN_H546M              5
#define CLK_TOP_MAIN_H364M              6
#define CLK_TOP_MAIN_H218P4M            7
#define CLK_TOP_MAIN_H156M              8

#define CLK_TOP_UNIV_624M               9
#define CLK_TOP_UNIV_416M               10
#define CLK_TOP_UNIV_249P6M             11
#define CLK_TOP_UNIV_178P3M             12
#define CLK_TOP_UNIV_48M                13

#define CLK_TOP_CLKRTC_INT              14

#define CLK_TOP_HDMI_CTS_D2             15
#define CLK_TOP_HDMI_CTS_D3             16

#define CLK_TOP_LVDSPLL_D2              17
#define CLK_TOP_LVDSPLL_D4              18
#define CLK_TOP_LVDSPLL_D8              19
#define CLK_TOP_LVDSPLL_ETH             20

#define CLK_TOP_MSDCPLL_D2              21
#define CLK_TOP_MSDCPLL_D4              22

#define CLK_TOP_SYSPLL_D2P5             23
#define CLK_TOP_SYSPLL1_D2              24
#define CLK_TOP_SYSPLL1_D4              25
#define CLK_TOP_SYSPLL1_D8              26
#define CLK_TOP_SYSPLL1_D16             27
#define CLK_TOP_SYSPLL2_D2              28
#define CLK_TOP_SYSPLL2_D4              29
#define CLK_TOP_SYSPLL2_D8              30
#define CLK_TOP_SYSPLL3_D2              31
#define CLK_TOP_SYSPLL3_D4              32
#define CLK_TOP_SYSPLL4_D2              33
#define CLK_TOP_SYSPLL4_D4              34

#define CLK_TOP_TVDPLL_D2               35
#define CLK_TOP_TVDPLL_D4               36
#define CLK_TOP_TVDPLL_D8               37
#define CLK_TOP_TVDPLL_D16              38

#define CLK_TOP_UNIVPLL_D2P5            39
#define CLK_TOP_UNIVPLL1_D2             40
#define CLK_TOP_UNIVPLL1_D4             41
#define CLK_TOP_UNIVPLL2_D2             42
#define CLK_TOP_UNIVPLL2_D4             43
#define CLK_TOP_UNIVPLL2_D8             44
#define CLK_TOP_UNIVPLL3_D2             45
#define CLK_TOP_UNIVPLL3_D4             46
#define CLK_TOP_UNIVPLL3_D8             47

/* TOPCKGEN muxes */
#define CLK_TOP_AXI_SEL                 48
#define CLK_TOP_MEM_SEL                 49
#define CLK_TOP_DDRPHYCFG_SEL           50
#define CLK_TOP_MM_SEL                  51

#define CLK_TOP_PWM_SEL                 52
#define CLK_TOP_VDEC_SEL                53
#define CLK_TOP_MFG_SEL                 54

#define CLK_TOP_CAMTG_SEL               55
#define CLK_TOP_UART_SEL                56
#define CLK_TOP_SPI_SEL                 57

#define CLK_TOP_MSDC30_0_SEL            58
#define CLK_TOP_MSDC30_1_SEL            59
#define CLK_TOP_MSDC30_2_SEL            60

#define CLK_TOP_MSDC50_3_HSEL           61
#define CLK_TOP_MSDC50_3_SEL            62
#define CLK_TOP_AUDIO_SEL               63
#define CLK_TOP_AUD_INTBUS_SEL          64

#define CLK_TOP_PMICSPI_SEL             65
#define CLK_TOP_SCP_SEL                 66
#define CLK_TOP_ATB_SEL                 67
#define CLK_TOP_MJC_SEL                 68

#define CLK_TOP_DPI0_SEL                69
#define CLK_TOP_SCAM_SEL                70
#define CLK_TOP_AUD_1_SEL               71
#define CLK_TOP_AUD_2_SEL               72

#define CLK_TOP_DPI1_SEL                73
#define CLK_TOP_UFOENC_SEL              74
#define CLK_TOP_UFODEC_SEL              75
#define CLK_TOP_ETH_SEL                 76

#define CLK_TOP_ONFI_SEL                77
#define CLK_TOP_SNFI_SEL                78
#define CLK_TOP_HDMI_SEL                79
#define CLK_TOP_RTC_SEL                 80

/* TOPCKGEN gates */
#define CLK_TOP_ARMPLL_DIVIDER_PLL1_EN  81
#define CLK_TOP_ARMPLL_DIVIDER_PLL2_EN  82

#define CLK_TOP_NR_CLK                  83

/* --------------------------------------------------------------------- */
/* INFRACFG                                                              */
/* --------------------------------------------------------------------- */
#define CLK_INFRA_PMIC_TMR          0
#define CLK_INFRA_PMIC_AP           1
#define CLK_INFRA_PMIC_MD           2
#define CLK_INFRA_PMIC_CONN         3
#define CLK_INFRA_SCPSYS            4
#define CLK_INFRA_SEJ               5
#define CLK_INFRA_APXGPT            6
#define CLK_INFRA_USB               7
#define CLK_INFRA_ICUSB             8
#define CLK_INFRA_GCE               9
#define CLK_INFRA_THERM             10
#define CLK_INFRA_I2C0              11
#define CLK_INFRA_I2C1              12
#define CLK_INFRA_I2C2              13
#define CLK_INFRA_PWM_HCLK          14
#define CLK_INFRA_PWM1              15
#define CLK_INFRA_PWM2              16
#define CLK_INFRA_PWM3              17
#define CLK_INFRA_PWM               18
#define CLK_INFRA_UART0             19
#define CLK_INFRA_UART1             20
#define CLK_INFRA_UART2             21
#define CLK_INFRA_UART3             22
#define CLK_INFRA_USB_MCU           23
#define CLK_INFRA_NFI_ECC_66M       24
#define CLK_INFRA_NFI_66M           25
#define CLK_INFRA_BTIF              26

#define CLK_INFRA_SPI               27
#define CLK_INFRA_MSDC3             28
#define CLK_INFRA_MSDC1             29
#define CLK_INFRA_MSDC2             30
#define CLK_INFRA_MSDC0             31
#define CLK_INFRA_GCPU              32
#define CLK_INFRA_TRNG              33
#define CLK_INFRA_AUXADC            34
#define CLK_INFRA_CPUM              35
#define CLK_INFRA_IRRX              36
#define CLK_INFRA_UFO               37
#define CLK_INFRA_CEC               38
#define CLK_INFRA_CEC_26M           39
#define CLK_INFRA_NFI_BCLK          40
#define CLK_INFRA_NFI_ECC           41
#define CLK_INFRA_AP_DMA            42
#define CLK_INFRA_XIU               43
#define CLK_INFRA_DEVICE_APC        44
#define CLK_INFRA_XIU2AHB           45
#define CLK_INFRA_L2C_SRAM          46
#define CLK_INFRA_ETH_50M           47
#define CLK_INFRA_DEBUGSYS          48
#define CLK_INFRA_AUDIO             49
#define CLK_INFRA_ETH_25M           50
#define CLK_INFRA_NFI               51
#define CLK_INFRA_ONFI              52
#define CLK_INFRA_SNFI              53
#define CLK_INFRA_ETH               54
#define CLK_INFRA_DRAMC_F26M        55

#define CLK_INFRA_NR_CLK            56

#endif /* _DT_BINDINGS_CLK_MT8163_H */
