// SPDX-License-Identifier: GPL-2.0

#include <dm/lists.h>
#include <linux/math64.h>
#include <clk-uclass.h>
#include <dm.h>
#include <generic-phy.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/bitops.h>

#define MIPITX_DSI_CON		0x00
#define RG_DSI_LDOCORE_EN		BIT(0)
#define RG_DSI_CKG_LDOOUT_EN		BIT(1)
#define RG_DSI_BCLK_SEL			GENMASK(3, 2)
#define RG_DSI_LD_IDX_SEL		GENMASK(6, 4)
#define RG_DSI_PHYCLK_SEL		GENMASK(9, 8)
#define RG_DSI_DSICLK_FREQ_SEL		BIT(10)
#define RG_DSI_LPTX_CLMP_EN		BIT(11)

#define MIPITX_DSI_CLOCK_LANE	0x04
#define MIPITX_DSI_DATA_LANE0	0x08
#define MIPITX_DSI_DATA_LANE1	0x0c
#define MIPITX_DSI_DATA_LANE2	0x10
#define MIPITX_DSI_DATA_LANE3	0x14
#define RG_DSI_LNTx_LDOOUT_EN		BIT(0)
#define RG_DSI_LNTx_CKLANE_EN		BIT(1)
#define RG_DSI_LNTx_LPTX_IPLUS1		BIT(2)
#define RG_DSI_LNTx_LPTX_IPLUS2		BIT(3)
#define RG_DSI_LNTx_LPTX_IMINUS		BIT(4)
#define RG_DSI_LNTx_LPCD_IPLUS		BIT(5)
#define RG_DSI_LNTx_LPCD_IMINUS		BIT(6)
#define RG_DSI_LNTx_RT_CODE		GENMASK(11, 8)

#define MIPITX_DSI_TOP_CON	0x40
#define RG_DSI_LNT_INTR_EN		BIT(0)
#define RG_DSI_LNT_HS_BIAS_EN		BIT(1)
#define RG_DSI_LNT_IMP_CAL_EN		BIT(2)
#define RG_DSI_LNT_TESTMODE_EN		BIT(3)
#define RG_DSI_LNT_IMP_CAL_CODE		GENMASK(7, 4)
#define RG_DSI_LNT_AIO_SEL		GENMASK(10, 8)
#define RG_DSI_PAD_TIE_LOW_EN		BIT(11)
#define RG_DSI_DEBUG_INPUT_EN		BIT(12)
#define RG_DSI_PRESERVE			GENMASK(15, 13)

#define MIPITX_DSI_BG_CON	0x44
#define RG_DSI_BG_CORE_EN		BIT(0)
#define RG_DSI_BG_CKEN			BIT(1)
#define RG_DSI_BG_DIV			GENMASK(3, 2)
#define RG_DSI_BG_FAST_CHARGE		BIT(4)

#define RG_DSI_V12_SEL			GENMASK(7, 5)
#define RG_DSI_V10_SEL			GENMASK(10, 8)
#define RG_DSI_V072_SEL			GENMASK(13, 11)
#define RG_DSI_V04_SEL			GENMASK(16, 14)
#define RG_DSI_V032_SEL			GENMASK(19, 17)
#define RG_DSI_V02_SEL			GENMASK(22, 20)
#define RG_DSI_VOUT_MSK			\
		(RG_DSI_V12_SEL | RG_DSI_V10_SEL | RG_DSI_V072_SEL | \
		 RG_DSI_V04_SEL | RG_DSI_V032_SEL | RG_DSI_V02_SEL)
#define RG_DSI_BG_R1_TRIM		GENMASK(27, 24)
#define RG_DSI_BG_R2_TRIM		GENMASK(31, 28)

#define MIPITX_DSI_PLL_CON0	0x50
#define RG_DSI_MPPLL_PLL_EN		BIT(0)
#define RG_DSI_MPPLL_PREDIV		GENMASK(2, 1)
#define RG_DSI_MPPLL_TXDIV0		GENMASK(4, 3)
#define RG_DSI_MPPLL_TXDIV1		GENMASK(6, 5)
#define RG_DSI_MPPLL_POSDIV		GENMASK(9, 7)
#define RG_DSI_MPPLL_DIV_MSK		\
		(RG_DSI_MPPLL_PREDIV | RG_DSI_MPPLL_TXDIV0 | \
		 RG_DSI_MPPLL_TXDIV1 | RG_DSI_MPPLL_POSDIV)
#define RG_DSI_MPPLL_MONVC_EN		BIT(10)
#define RG_DSI_MPPLL_MONREF_EN		BIT(11)
#define RG_DSI_MPPLL_VOD_EN		BIT(12)

#define MIPITX_DSI_PLL_CON1	0x54
#define RG_DSI_MPPLL_SDM_FRA_EN		BIT(0)
#define RG_DSI_MPPLL_SDM_SSC_PH_INIT	BIT(1)
#define RG_DSI_MPPLL_SDM_SSC_EN		BIT(2)
#define RG_DSI_MPPLL_SDM_SSC_PRD	GENMASK(31, 16)

#define MIPITX_DSI_PLL_CON2	0x58

#define MIPITX_DSI_PLL_TOP	0x64
#define RG_DSI_MPPLL_PRESERVE		GENMASK(15, 8)

#define MIPITX_DSI_PLL_PWR	0x68
#define RG_DSI_MPPLL_SDM_PWR_ON		BIT(0)
#define RG_DSI_MPPLL_SDM_ISO_EN		BIT(1)
#define RG_DSI_MPPLL_SDM_PWR_ACK	BIT(8)

#define MIPITX_DSI_SW_CTRL	0x80
#define SW_CTRL_EN			BIT(0)

#define MIPITX_DSI_SW_CTRL_CON0	0x84
#define SW_LNTC_LPTX_PRE_OE		BIT(0)
#define SW_LNTC_LPTX_OE			BIT(1)
#define SW_LNTC_LPTX_P			BIT(2)
#define SW_LNTC_LPTX_N			BIT(3)
#define SW_LNTC_HSTX_PRE_OE		BIT(4)
#define SW_LNTC_HSTX_OE			BIT(5)
#define SW_LNTC_HSTX_ZEROCLK		BIT(6)
#define SW_LNT0_LPTX_PRE_OE		BIT(7)
#define SW_LNT0_LPTX_OE			BIT(8)
#define SW_LNT0_LPTX_P			BIT(9)
#define SW_LNT0_LPTX_N			BIT(10)
#define SW_LNT0_HSTX_PRE_OE		BIT(11)
#define SW_LNT0_HSTX_OE			BIT(12)
#define SW_LNT0_LPRX_EN			BIT(13)
#define SW_LNT1_LPTX_PRE_OE		BIT(14)
#define SW_LNT1_LPTX_OE			BIT(15)
#define SW_LNT1_LPTX_P			BIT(16)
#define SW_LNT1_LPTX_N			BIT(17)
#define SW_LNT1_HSTX_PRE_OE		BIT(18)
#define SW_LNT1_HSTX_OE			BIT(19)
#define SW_LNT2_LPTX_PRE_OE		BIT(20)
#define SW_LNT2_LPTX_OE			BIT(21)
#define SW_LNT2_LPTX_P			BIT(22)
#define SW_LNT2_LPTX_N			BIT(23)
#define SW_LNT2_HSTX_PRE_OE		BIT(24)
#define SW_LNT2_HSTX_OE			BIT(25)

struct mtk_mipi_tx {
	void __iomem *regs;
	ulong data_rate;
	u32 mppll_preserve;
	struct clk clk;
};

static ulong mtk_dsi_clk_set_rate(struct clk *clk, ulong rate)
{
	struct mtk_mipi_tx *priv = dev_get_priv(clk->dev->parent);

	priv->data_rate = rate;
	return priv->data_rate;
}

static ulong mtk_dsi_clk_get_rate(struct clk *clk)
{
	struct mtk_mipi_tx *priv = dev_get_priv(clk->dev->parent);

	return priv->data_rate;
}

static int mtk_dsi_clk_enable(struct clk *clk)
{
	struct mtk_mipi_tx *priv = dev_get_priv(clk->dev->parent);
	void __iomem *base = priv->regs;
	u8 txdiv0, txdiv1, txdiv;
	u64 pcw;

	if (priv->data_rate >= 500000000) {
		txdiv = 1;  txdiv0 = 0; txdiv1 = 0;
	} else if (priv->data_rate >= 250000000) {
		txdiv = 2;  txdiv0 = 1; txdiv1 = 0;
	} else if (priv->data_rate >= 125000000) {
		txdiv = 4;  txdiv0 = 2; txdiv1 = 0;
	} else if (priv->data_rate > 62000000) {
		txdiv = 8;  txdiv0 = 2; txdiv1 = 1;
	} else if (priv->data_rate >= 50000000) {
		txdiv = 16; txdiv0 = 2; txdiv1 = 2;
	} else {
		return -EINVAL;
	}

	/* Power up BG Core */
	clrsetbits_le32(base + MIPITX_DSI_BG_CON,
			RG_DSI_VOUT_MSK | RG_DSI_BG_CKEN | RG_DSI_BG_CORE_EN,
			(0x4 << 5) | (0x4 << 8) | (0x4 << 11) | (0x4 << 14) |
			(0x4 << 17) | (0x4 << 20) | RG_DSI_BG_CKEN | RG_DSI_BG_CORE_EN);
	udelay(30);

	/* Calibration & Core setup */
	clrsetbits_le32(base + MIPITX_DSI_TOP_CON,
			RG_DSI_LNT_IMP_CAL_CODE | RG_DSI_LNT_HS_BIAS_EN,
			(8 << 4) | RG_DSI_LNT_HS_BIAS_EN);

	setbits_le32(base + MIPITX_DSI_CON, RG_DSI_CKG_LDOOUT_EN | RG_DSI_LDOCORE_EN);

	/* PLL Power On */
	clrsetbits_le32(base + MIPITX_DSI_PLL_PWR,
			RG_DSI_MPPLL_SDM_PWR_ON | RG_DSI_MPPLL_SDM_ISO_EN,
			RG_DSI_MPPLL_SDM_PWR_ON);

	clrbits_le32(base + MIPITX_DSI_PLL_CON0, RG_DSI_MPPLL_PLL_EN);

	clrsetbits_le32(base + MIPITX_DSI_PLL_CON0,
			RG_DSI_MPPLL_TXDIV0 | RG_DSI_MPPLL_TXDIV1 | RG_DSI_MPPLL_PREDIV,
			(txdiv0 << 3) | (txdiv1 << 5));

	/* PCW Calculation (Ref clk = 26 MHz) */
	pcw = div_u64(((u64)priv->data_rate * 2 * txdiv) << 24, 26000000);
	writel(pcw, base + MIPITX_DSI_PLL_CON2);

	setbits_le32(base + MIPITX_DSI_PLL_CON1, RG_DSI_MPPLL_SDM_FRA_EN);
	setbits_le32(base + MIPITX_DSI_PLL_CON0, RG_DSI_MPPLL_PLL_EN);
	udelay(20);

	clrbits_le32(base + MIPITX_DSI_PLL_CON1, RG_DSI_MPPLL_SDM_SSC_EN);
	clrsetbits_le32(base + MIPITX_DSI_PLL_TOP, RG_DSI_MPPLL_PRESERVE,
			priv->mppll_preserve << 8);

	return 0;
}

static int mtk_dsi_clk_disable(struct clk *clk)
{
	struct mtk_mipi_tx *priv = dev_get_priv(clk->dev->parent);
	void __iomem *base = priv->regs;

	clrbits_le32(base + MIPITX_DSI_PLL_CON0, RG_DSI_MPPLL_PLL_EN);
	clrbits_le32(base + MIPITX_DSI_PLL_TOP, RG_DSI_MPPLL_PRESERVE);
	clrsetbits_le32(base + MIPITX_DSI_PLL_PWR,
			RG_DSI_MPPLL_SDM_ISO_EN | RG_DSI_MPPLL_SDM_PWR_ON,
			RG_DSI_MPPLL_SDM_ISO_EN);

	clrbits_le32(base + MIPITX_DSI_TOP_CON, RG_DSI_LNT_HS_BIAS_EN);
	clrbits_le32(base + MIPITX_DSI_CON, RG_DSI_CKG_LDOOUT_EN | RG_DSI_LDOCORE_EN);
	clrbits_le32(base + MIPITX_DSI_BG_CON, RG_DSI_BG_CKEN | RG_DSI_BG_CORE_EN);
	clrbits_le32(base + MIPITX_DSI_PLL_CON0, RG_DSI_MPPLL_DIV_MSK);

	return 0;
}

static const struct clk_ops mtk_mipi_tx_clk_ops = {
	.set_rate = mtk_dsi_clk_set_rate,
	.get_rate = mtk_dsi_clk_get_rate,
	.enable = mtk_dsi_clk_enable,
	.disable = mtk_dsi_clk_disable,
};

static int mtk_mipi_tx_power_on(struct phy *phy)
{
	struct mtk_mipi_tx *priv = dev_get_priv(phy->dev);
	u32 reg;
	int ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		log_ret(ret);

	for (reg = MIPITX_DSI_CLOCK_LANE; reg <= MIPITX_DSI_DATA_LANE3; reg += 4)
		setbits_le32(priv->regs + reg, RG_DSI_LNTx_LDOOUT_EN);

	clrbits_le32(priv->regs + MIPITX_DSI_TOP_CON, RG_DSI_PAD_TIE_LOW_EN);

	return 0;
}

static int mtk_mipi_tx_power_off(struct phy *phy)
{
	struct mtk_mipi_tx *priv = dev_get_priv(phy->dev);
	u32 reg;

	setbits_le32(priv->regs + MIPITX_DSI_TOP_CON, RG_DSI_PAD_TIE_LOW_EN);

	for (reg = MIPITX_DSI_CLOCK_LANE; reg <= MIPITX_DSI_DATA_LANE3; reg += 4)
		clrbits_le32(priv->regs + reg, RG_DSI_LNTx_LDOOUT_EN);

	clk_disable(&priv->clk);

	return 0;
}

static const struct phy_ops mtk_mipi_tx_ops = {
	.power_on  = mtk_mipi_tx_power_on,
	.power_off = mtk_mipi_tx_power_off,
};

static int mtk_mipi_tx_bind(struct udevice *dev)
{
	return device_bind_driver_to_node(dev, "mtk_mipi_tx_clk", "mtk_mipi_tx_clk",
	                                  dev_ofnode(dev), NULL);
}

static int mtk_mipi_tx_probe(struct udevice *dev)
{
	struct mtk_mipi_tx *priv = dev_get_priv(dev);
	struct udevice *clk_dev;
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->mppll_preserve = (u32)dev_get_driver_data(dev);


	ret = device_get_child(dev, 0, &clk_dev);
	if (ret)
		return ret;

	priv->clk.dev = clk_dev;

	return 0;
}

static const struct udevice_id mtk_mipi_tx_ids[] = {
	{ .compatible = "mediatek,mt2701-mipi-tx", .data = 3 },
	{ }
};

U_BOOT_DRIVER(mtk_mipi_tx_clk) = {
	.name = "mtk_mipi_tx_clk",
	.id   = UCLASS_CLK,
	.ops  = &mtk_mipi_tx_clk_ops,
};

U_BOOT_DRIVER(mtk_mipi_tx) = {
	.name = "mtk_mipi_tx",
	.id = UCLASS_PHY,
	.of_match = mtk_mipi_tx_ids,
	.ops = &mtk_mipi_tx_ops,
	.bind = mtk_mipi_tx_bind,
	.probe = mtk_mipi_tx_probe,
	.priv_auto = sizeof(struct mtk_mipi_tx),
};
