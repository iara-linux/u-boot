// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: YH Huang <yh.huang@mediatek.com>
 */

#include <clk.h>
#include <linux/math64.h>
#include <dm.h>
#include <pwm.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/time.h>

#define DISP_PWM_EN		0x00

#define PWM_CLKDIV_SHIFT	16
#define PWM_CLKDIV_MAX		0x3ff
#define PWM_CLKDIV_MASK		(PWM_CLKDIV_MAX << PWM_CLKDIV_SHIFT)

#define PWM_PERIOD_BIT_WIDTH	12
#define PWM_PERIOD_MASK		((1 << PWM_PERIOD_BIT_WIDTH) - 1)

#define PWM_HIGH_WIDTH_SHIFT	16
#define PWM_HIGH_WIDTH_MASK	(0x1fff << PWM_HIGH_WIDTH_SHIFT)

struct mtk_disp_pwm_data {
	u32 enable_mask;
	unsigned int con0;
	u32 con0_sel;
	unsigned int con1;

	bool has_commit;
	unsigned int commit;
	unsigned int commit_mask;

	unsigned int bls_debug;
	u32 bls_debug_mask;
};

struct mtk_disp_pwm_priv {
	const struct mtk_disp_pwm_data *data;
	struct clk clk_main;
	struct clk clk_mm;
	void __iomem *base;
	bool clk_enabled;
};

static void mtk_disp_pwm_update_bits(struct mtk_disp_pwm_priv *priv, u32 offset,
				     u32 mask, u32 data)
{
	void __iomem *address = priv->base + offset;
	u32 value;

	value = readl(address);
	value &= ~mask;
	value |= data;
	writel(value, address);
}

static int mtk_disp_pwm_clk_enable(struct udevice *dev)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	if (priv->clk_enabled)
		return 0;

	ret = clk_enable(&priv->clk_main);
	if (ret) {
		dev_err(dev, "Can't enable main clock: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk_mm);
	if (ret) {
		dev_err(dev, "Can't enable mm clock: %d\n", ret);
		clk_disable(&priv->clk_main);
		return ret;
	}

	priv->clk_enabled = true;

	return 0;
}

static int mtk_disp_pwm_set_config(struct udevice *dev, uint channel,
				   uint period_ns, uint duty_ns)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);
	u32 clk_div, period, high_width, value;
	u64 div, rate;
	int ret;

	ret = mtk_disp_pwm_clk_enable(dev);
	if (ret)
		return ret;

	/*
	 * Find period, high_width and clk_div to suit duty_ns and period_ns.
	 * Calculate proper div value to keep period value in the bound.
	 *
	 * period_ns = 10^9 * (clk_div + 1) * (period + 1) / PWM_CLK_RATE
	 * duty_ns = 10^9 * (clk_div + 1) * high_width / PWM_CLK_RATE
	 *
	 * period = (PWM_CLK_RATE * period_ns) / (10^9 * (clk_div + 1)) - 1
	 * high_width = (PWM_CLK_RATE * duty_ns) / (10^9 * (clk_div + 1))
	 */
	rate = clk_get_rate(&priv->clk_main);

	clk_div = div_u64((u64)period_ns * rate, NSEC_PER_SEC) >>
		  PWM_PERIOD_BIT_WIDTH;
	if (clk_div > PWM_CLKDIV_MAX)
		return -EINVAL;

	div = (u64)NSEC_PER_SEC * (clk_div + 1);
	period = div64_u64((u64)period_ns * rate, div);
	if (period > 0)
		period--;

	high_width = div64_u64((u64)duty_ns * rate, div);
	value = period | (high_width << PWM_HIGH_WIDTH_SHIFT);

	if (priv->data->bls_debug && !priv->data->has_commit) {
		/*
		 * For MT2701, disable double buffer before writing register
		 * and select manual mode and use PWM_PERIOD/PWM_HIGH_WIDTH.
		 */
		mtk_disp_pwm_update_bits(priv, priv->data->bls_debug,
					 priv->data->bls_debug_mask,
					 priv->data->bls_debug_mask);
		mtk_disp_pwm_update_bits(priv, priv->data->con0,
					 priv->data->con0_sel,
					 priv->data->con0_sel);
	}

	mtk_disp_pwm_update_bits(priv, priv->data->con0, PWM_CLKDIV_MASK,
				 clk_div << PWM_CLKDIV_SHIFT);
	mtk_disp_pwm_update_bits(priv, priv->data->con1,
				 PWM_PERIOD_MASK | PWM_HIGH_WIDTH_MASK, value);

	if (priv->data->has_commit) {
		mtk_disp_pwm_update_bits(priv, priv->data->commit,
					 priv->data->commit_mask,
					 priv->data->commit_mask);
		mtk_disp_pwm_update_bits(priv, priv->data->commit,
					 priv->data->commit_mask, 0x0);
	}

	return 0;
}

static int mtk_disp_pwm_set_enable(struct udevice *dev, uint channel,
				   bool enable)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	if (!enable) {
		mtk_disp_pwm_update_bits(priv, DISP_PWM_EN,
					 priv->data->enable_mask, 0x0);

		if (priv->clk_enabled) {
			clk_disable(&priv->clk_mm);
			clk_disable(&priv->clk_main);
			priv->clk_enabled = false;
		}

		return 0;
	}

	ret = mtk_disp_pwm_clk_enable(dev);
	if (ret)
		return ret;

	mtk_disp_pwm_update_bits(priv, DISP_PWM_EN, priv->data->enable_mask,
				 priv->data->enable_mask);

	return 0;
}

static int mtk_disp_pwm_probe(struct udevice *dev)
{
	struct mtk_disp_pwm_priv *priv = dev_get_priv(dev);
	int ret;

	priv->data = (const struct mtk_disp_pwm_data *)dev_get_driver_data(dev);

	priv->base = dev_remap_addr(dev);
	if (!priv->base)
		return -EINVAL;

	ret = clk_get_by_name(dev, "main", &priv->clk_main);
	if (ret) {
		dev_err(dev, "Failed to get main clock: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "mm", &priv->clk_mm);
	if (ret) {
		dev_err(dev, "Failed to get mm clock: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct pwm_ops mtk_disp_pwm_ops = {
	.set_config	= mtk_disp_pwm_set_config,
	.set_enable	= mtk_disp_pwm_set_enable,
};

static const struct mtk_disp_pwm_data mt2701_disp_pwm_data = {
	.enable_mask = BIT(16),
	.con0 = 0xa8,
	.con0_sel = 0x2,
	.con1 = 0xac,
	.has_commit = false,
	.bls_debug = 0xb0,
	.bls_debug_mask = 0x3,
};

static const struct mtk_disp_pwm_data mt8173_disp_pwm_data = {
	.enable_mask = BIT(0),
	.con0 = 0x10,
	.con0_sel = 0x0,
	.con1 = 0x14,
	.has_commit = true,
	.commit = 0x8,
	.commit_mask = 0x1,
};

static const struct mtk_disp_pwm_data mt8183_disp_pwm_data = {
	.enable_mask = BIT(0),
	.con0 = 0x18,
	.con0_sel = 0x0,
	.con1 = 0x1c,
	.has_commit = false,
	.bls_debug = 0x80,
	.bls_debug_mask = 0x3,
};

static const struct udevice_id mtk_disp_pwm_ids[] = {
	{
		.compatible = "mediatek,mt2701-disp-pwm",
		.data = (ulong)&mt2701_disp_pwm_data,
	}, {
		.compatible = "mediatek,mt6595-disp-pwm",
		.data = (ulong)&mt8173_disp_pwm_data,
	}, {
		.compatible = "mediatek,mt8173-disp-pwm",
		.data = (ulong)&mt8173_disp_pwm_data,
	}, {
		.compatible = "mediatek,mt8183-disp-pwm",
		.data = (ulong)&mt8183_disp_pwm_data,
	}, {
		/* sentinel */
	}
};

U_BOOT_DRIVER(mtk_disp_pwm) = {
	.name = "mtk_disp_pwm",
	.id = UCLASS_PWM,
	.of_match = mtk_disp_pwm_ids,
	.ops = &mtk_disp_pwm_ops,
	.probe = mtk_disp_pwm_probe,
	.priv_auto = sizeof(struct mtk_disp_pwm_priv),
};
