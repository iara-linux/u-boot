// SPDX-License-Identifier: GPL-2.0+
/*
 * TI TPS65132 dual output power supply.
 *
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <dm/device_compat.h>
#include <power/pmic.h>
#include <power/regulator.h>
#include <power/tps65132.h>

static const struct pmic_child_info tps65132_children_info[] = {
	{ .prefix = "outp", .driver = TPS65132_OUT_DRIVER },
	{ .prefix = "outn", .driver = TPS65132_OUT_DRIVER },
	{ },
};

static int tps65132_write(struct udevice *dev, uint reg, const u8 *buff,
			  int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "write error to register %#x: %d\n", reg, ret);

	return ret;
}

static int tps65132_read(struct udevice *dev, uint reg, u8 *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret)
		dev_err(dev, "read error from register %#x: %d\n", reg, ret);

	return ret;
}

static int tps65132_bind(struct udevice *dev)
{
	if (!pmic_bind_children(dev, dev_ofnode(dev), tps65132_children_info))
		dev_warn(dev, "no outputs found\n");

	return 0;
}

static struct dm_pmic_ops tps65132_ops = {
	.read = tps65132_read,
	.write = tps65132_write,
};

static const struct udevice_id tps65132_ids[] = {
	{ .compatible = "ti,tps65132" },
	{ }
};

U_BOOT_DRIVER(pmic_tps65132) = {
	.name = "tps65132_pmic",
	.id = UCLASS_PMIC,
	.of_match = tps65132_ids,
	.bind = tps65132_bind,
	.ops = &tps65132_ops,
};
