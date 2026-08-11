// SPDX-License-Identifier: GPL-2.0+
/*
 * Regulator driver for the outputs of the TI TPS65132 power supply.
 *
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>
#include <log.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65132.h>

/*
 * The charge pump current limit is a single chip wide bit rather than a per
 * output setting, and only these two values exist in hardware.
 */
#define TPS65132_CURRENT_40MA		40000
#define TPS65132_CURRENT_80MA		80000

/**
 * struct tps65132_out_priv - state of one TPS65132 output
 * @enable_gpio: per output enable pin, the chip has no enable register
 * @uV: output voltage, applied once the output is enabled
 * @uA: charge pump current limit, zero to leave it at the default
 * @vout_reg: output voltage register
 * @discharge_mask: active discharge bit for this output
 * @discharge: leave active discharge on for this output
 * @enabled: an output is on, so the chip answers on I2C
 */
struct tps65132_out_priv {
	struct gpio_desc enable_gpio;
	int uV;
	int uA;
	u8 vout_reg;
	u8 discharge_mask;
	bool discharge;
	bool enabled;
};

static int tps65132_out_get_value(struct udevice *dev)
{
	struct tps65132_out_priv *priv = dev_get_priv(dev);
	int ret;

	ret = pmic_reg_read(dev->parent, priv->vout_reg);
	if (ret < 0)
		return ret;

	return TPS65132_VOUT_VMIN +
	       (ret & TPS65132_VOUT_MASK) * TPS65132_VOUT_STEP;
}

static int tps65132_out_write_value(struct udevice *dev, int uV)
{
	struct tps65132_out_priv *priv = dev_get_priv(dev);

	return pmic_clrsetbits(dev->parent, priv->vout_reg, TPS65132_VOUT_MASK,
			       (uV - TPS65132_VOUT_VMIN) / TPS65132_VOUT_STEP);
}

static int tps65132_out_set_value(struct udevice *dev, int uV)
{
	struct tps65132_out_priv *priv = dev_get_priv(dev);

	if (uV < TPS65132_VOUT_VMIN || uV > TPS65132_VOUT_VMAX)
		return -EINVAL;

	priv->uV = uV;

	/* Remember it for later if the chip is still powered down */
	if (!priv->enabled)
		return 0;

	return tps65132_out_write_value(dev, uV);
}

static int tps65132_out_get_current(struct udevice *dev)
{
	int ret;

	ret = pmic_reg_read(dev->parent, TPS65132_REG_APPS_DISP_DISN);
	if (ret < 0)
		return ret;

	return ret & TPS65132_APPS_APPS ? TPS65132_CURRENT_80MA :
					  TPS65132_CURRENT_40MA;
}

static int tps65132_out_write_current(struct udevice *dev, int uA)
{
	return pmic_clrsetbits(dev->parent, TPS65132_REG_APPS_DISP_DISN,
			       TPS65132_APPS_APPS,
			       uA == TPS65132_CURRENT_80MA ?
			       TPS65132_APPS_APPS : 0);
}

static int tps65132_out_set_current(struct udevice *dev, int uA)
{
	struct tps65132_out_priv *priv = dev_get_priv(dev);

	if (uA != TPS65132_CURRENT_40MA && uA != TPS65132_CURRENT_80MA)
		return -EINVAL;

	priv->uA = uA;

	if (!priv->enabled)
		return 0;

	return tps65132_out_write_current(dev, uA);
}

static int tps65132_out_get_enable(struct udevice *dev)
{
	struct tps65132_out_priv *priv = dev_get_priv(dev);

	if (!dm_gpio_is_valid(&priv->enable_gpio))
		return true;

	return dm_gpio_get_value(&priv->enable_gpio);
}

static int tps65132_out_set_enable(struct udevice *dev, bool enable)
{
	struct tps65132_out_priv *priv = dev_get_priv(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->enable_gpio))
		return enable ? 0 : -EACCES;

	ret = dm_gpio_set_value(&priv->enable_gpio, enable);
	if (ret)
		return ret;

	if (!enable) {
		priv->enabled = false;
		return 0;
	}

	/*
	 * With both outputs disabled the chip does not ack on I2C, so hold
	 * back everything that needs configuring until one is up.
	 */
	udelay(TPS65132_ENABLE_TIME_US);
	priv->enabled = true;

	if (priv->uA) {
		ret = tps65132_out_write_current(dev, priv->uA);
		if (ret)
			return ret;
	}

	if (priv->uV) {
		ret = tps65132_out_write_value(dev, priv->uV);
		if (ret)
			return ret;
	}

	/* Hardware automatically enable discharge bit in enable */
	if (!priv->discharge)
		return pmic_clrsetbits(dev->parent,
				       TPS65132_REG_APPS_DISP_DISN,
				       priv->discharge_mask, 0);

	return 0;
}

/*
 * The current limit applies to the whole chip, so every output has to ask
 * for the same one, and only the two values the hardware implements are
 * accepted. Returns the limit in uA, or zero to leave it at its default.
 */
static int tps65132_out_parse_current(struct udevice *dev)
{
	bool first = true;
	ofnode node;
	int uA = 0;

	ofnode_for_each_subnode(node, dev_ofnode(dev->parent)) {
		u32 min_uA, max_uA;
		int out_uA = 0;

		if (!ofnode_read_u32(node, "regulator-min-microamp", &min_uA)) {
			if (ofnode_read_u32(node, "regulator-max-microamp",
					    &max_uA) || min_uA != max_uA) {
				dev_err(dev, "%s: current limit is not a fixed value\n",
					ofnode_get_name(node));
				return -EINVAL;
			}

			if (min_uA != TPS65132_CURRENT_40MA &&
			    min_uA != TPS65132_CURRENT_80MA) {
				dev_err(dev, "%s: current limit must be %u or %u uA\n",
					ofnode_get_name(node),
					TPS65132_CURRENT_40MA,
					TPS65132_CURRENT_80MA);
				return -EINVAL;
			}

			out_uA = min_uA;
		}

		if (first) {
			uA = out_uA;
			first = false;
		} else if (out_uA != uA) {
			dev_err(dev, "outputs disagree on the current limit\n");
			return -EINVAL;
		}
	}

	return uA;
}

static int tps65132_out_of_to_plat(struct udevice *dev)
{
	struct dm_regulator_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	struct tps65132_out_priv *priv = dev_get_priv(dev);
	int ret;

	uc_pdata->type = REGULATOR_TYPE_OTHER;

	if (!strcmp(dev->name, "outn")) {
		priv->vout_reg = TPS65132_REG_VNEG;
		priv->discharge_mask = TPS65132_APPS_DIS_VNEG;
	} else {
		priv->vout_reg = TPS65132_REG_VPOS;
		priv->discharge_mask = TPS65132_APPS_DIS_VPOS;
	}

	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable_gpio,
				   GPIOD_IS_OUT);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get enable GPIO: %d\n", ret);
		return ret;
	}

	ret = tps65132_out_parse_current(dev);
	if (ret < 0)
		return ret;

	priv->uA = ret;
	priv->uV = uc_pdata->min_uV > 0 ? uc_pdata->min_uV : 0;
	priv->discharge = dev_read_u32_default(dev,
					       "regulator-active-discharge", 0);

	return 0;
}

static const struct dm_regulator_ops tps65132_out_ops = {
	.get_value	= tps65132_out_get_value,
	.set_value	= tps65132_out_set_value,
	.get_current	= tps65132_out_get_current,
	.set_current	= tps65132_out_set_current,
	.get_enable	= tps65132_out_get_enable,
	.set_enable	= tps65132_out_set_enable,
};

U_BOOT_DRIVER(tps65132_regulator) = {
	.name		= TPS65132_OUT_DRIVER,
	.id		= UCLASS_REGULATOR,
	.ops		= &tps65132_out_ops,
	.of_to_plat	= tps65132_out_of_to_plat,
	.priv_auto	= sizeof(struct tps65132_out_priv),
};
