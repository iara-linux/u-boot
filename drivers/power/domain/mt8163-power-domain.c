// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025-2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>
#include <dt-bindings/power/mediatek,mt8163-power-controller.h>
#include <linux/bitops.h>

#include "mtk-power-domain.h"

/* Infra TOPAXI bus protection bits */
#define MT8163_TOP_AXI_PROT_EN_MM_M0		BIT(1)
#define MT8163_TOP_AXI_PROT_EN_CCI_M2		BIT(13)
#define MT8163_TOP_AXI_PROT_EN_CONN2EMI		BIT(14)
#define MT8163_TOP_AXI_PROT_EN_CONN2PERI	BIT(15)
#define MT8163_TOP_AXI_PROT_EN_MFG_M0		BIT(21)
#define MT8163_TOP_AXI_PROT_EN_MFG_SNOOP_OUT	BIT(23)

static const struct mtk_scp_domain_data mt8163_scp_domain[] = {
	[MT8163_POWER_DOMAIN_MM] = {
		.sta_mask = PWR_STATUS_DISP,
		.ctl_offs = SPM_DIS_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(12, 12),
		.bus_prot_mask = MT8163_TOP_AXI_PROT_EN_MM_M0,
	},
	[MT8163_POWER_DOMAIN_VDEC] = {
		.sta_mask = PWR_STATUS_VDEC,
		.ctl_offs = SPM_VDE_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(12, 12),
	},
	[MT8163_POWER_DOMAIN_VENC] = {
		.sta_mask = PWR_STATUS_VENC,
		.ctl_offs = SPM_VEN_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
	},
	[MT8163_POWER_DOMAIN_ISP] = {
		.sta_mask = PWR_STATUS_ISP,
		.ctl_offs = SPM_ISP_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(13, 12),
	},
	[MT8163_POWER_DOMAIN_CONN] = {
		.sta_mask = PWR_STATUS_CONN,
		.ctl_offs = SPM_CONN_PWR_CON,
		.sram_pdn_bits = GENMASK(8, 8),
		.sram_pdn_ack_bits = 0,
		.bus_prot_mask = MT8163_TOP_AXI_PROT_EN_CCI_M2 |
				 MT8163_TOP_AXI_PROT_EN_CONN2EMI |
				 MT8163_TOP_AXI_PROT_EN_CONN2PERI,
	},
	[MT8163_POWER_DOMAIN_AUDIO] = {
		.sta_mask = PWR_STATUS_AUDIO,
		.ctl_offs = SPM_AUDIO_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = GENMASK(15, 12),
	},
	[MT8163_POWER_DOMAIN_MFG_ASYNC] = {
		.sta_mask = PWR_STATUS_MFG_ASYNC,
		.ctl_offs = SPM_MFG_ASYNC_PWR_CON,
		.sram_pdn_bits = GENMASK(11, 8),
		.sram_pdn_ack_bits = 0,
		.bus_prot_mask = MT8163_TOP_AXI_PROT_EN_MFG_M0 |
				 MT8163_TOP_AXI_PROT_EN_MFG_SNOOP_OUT,
	},
	[MT8163_POWER_DOMAIN_MFG] = {
		.sta_mask = PWR_STATUS_MFG,
		.ctl_offs = SPM_MFG_PWR_CON,
		.sram_pdn_bits = GENMASK(13, 8),
		.sram_pdn_ack_bits = GENMASK(16, 16),
	},
};

MTK_SCP_SOC_DATA(mt8163, mt8163_scp_domain);

static const struct udevice_id mt8163_power_domain_ids[] = {
	{
		.compatible = "mediatek,mt8163-power-controller",
		.data = (ulong)&mt8163_scp_soc_data,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mt8163_power_domain) = {
	.name = "mt8163_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.ops = &mtk_power_domain_ops,
	.probe = mtk_power_controller_probe,
	.of_match = mt8163_power_domain_ids,
	.priv_auto = sizeof(struct mtk_scpsys),
};
