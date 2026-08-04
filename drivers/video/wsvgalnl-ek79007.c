// SPDX-License-Identifier: GPL-2.0+

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <asm/gpio.h>

struct ek79007_priv {
	struct udevice *vcc;
	struct udevice *iovcc;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;
	struct gpio_desc enable_gpio;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= (1024 + 60 + 10 + 60) * (600 + 10 + 10 + 50) * 60,
	.hactive.typ		= 1024,
	.hfront_porch.typ	= 60,
	.hback_porch.typ	= 10,
	.hsync_len.typ		= 60,
	.vactive.typ		= 600,
	.vfront_porch.typ	= 10,
	.vback_porch.typ	= 50,
	.vsync_len.typ		= 10,
};

#define dsi_generic_write_seq(dsi, cmd, seq...) do {			\
		static const u8 b[] = { cmd, seq };			\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, b, ARRAY_SIZE(b));	\
		if (ret < 0)						\
			return log_ret(ret);				\
	} while (0)

static int ek79007_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	dsi_generic_write_seq(dsi, 0xb2, 0x10);
	dsi_generic_write_seq(dsi, 0x80, 0xbb);
	dsi_generic_write_seq(dsi, 0x81, 0xbb);
	dsi_generic_write_seq(dsi, 0x82, 0xbb);
	dsi_generic_write_seq(dsi, 0x83, 0xbb);
	dsi_generic_write_seq(dsi, 0x84, 0xbb);
	dsi_generic_write_seq(dsi, 0x85, 0xbb);
	dsi_generic_write_seq(dsi, 0x86, 0xbb);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret) {
		log_debug("%s: failed to exit sleep mode: %d\n", __func__, ret);
		return log_ret(ret);
	}

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret) {
		log_debug("%s: failed to set display on: %d\n", __func__, ret);
		return log_ret(ret);
	}

	mdelay(120);

	return 0;
}

static int ek79007_set_backlight(struct udevice *dev, int percent)
{
	struct ek79007_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return log_ret(ret);

	return backlight_set_brightness(priv->backlight, percent);
}

static int ek79007_timings(struct udevice *dev, struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int ek79007_of_to_plat(struct udevice *dev)
{
	struct ek79007_priv *priv = dev_get_priv(dev);
	int ret;

	ret = uclass_get_device_by_phandle(UCLASS_PANEL_BACKLIGHT, dev,
					   "backlight", &priv->backlight);
	if (ret) {
		log_debug("%s: cannot get backlight: ret = %d\n",
			  __func__, ret);
		return log_ret(ret);
	}

	ret = device_get_supply_regulator(dev, "vcc-supply", &priv->vcc);
	if (ret) {
		log_debug("%s: cannot get vcc-supply: ret = %d\n",
			  __func__, ret);
		return log_ret(ret);
	}

	ret = device_get_supply_regulator(dev, "iovcc-supply", &priv->iovcc);
	if (ret) {
		log_debug("%s: cannot get iovcc-supply: ret = %d\n",
			  __func__, ret);
		return log_ret(ret);
	}

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: cannot decode reset-gpios (%d)\n",
			  __func__, ret);
		return log_ret(ret);
	}

	ret = gpio_request_by_name(dev, "enable-gpios", 0,
				   &priv->enable_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: cannot decode enable-gpios (%d)\n",
			  __func__, ret);
		return log_ret(ret);
	}

	return 0;
}

static int ek79007_hw_init(struct udevice *dev)
{
	struct ek79007_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return log_ret(ret);

	ret = dm_gpio_set_value(&priv->enable_gpio, 0);
	if (ret)
		return log_ret(ret);

	ret = regulator_set_enable_if_allowed(priv->vcc, true);
	if (ret)
		return log_ret(ret);

	ret = regulator_set_enable_if_allowed(priv->iovcc, true);
	if (ret)
		return log_ret(ret);

	mdelay(1);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret)
		return log_ret(ret);

	ret = dm_gpio_set_value(&priv->enable_gpio, 1);
	if (ret)
		return log_ret(ret);

	mdelay(10);

	ret = ek79007_set_backlight(dev, 100);
	if (ret)
		return log_ret(ret);

	return 0;
}

static int ek79007_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			   MIPI_DSI_MODE_LPM;

	if (plat->device) {
		plat->device->lanes = plat->lanes;
		plat->device->format = plat->format;
		plat->device->mode_flags = plat->mode_flags;

		ret = mipi_dsi_attach(plat->device);
		if (ret)
			return log_ret(ret);
	}

	return ek79007_hw_init(dev);
}

static const struct panel_ops ek79007_ops = {
	.enable_backlight	= ek79007_enable_backlight,
	.set_backlight		= ek79007_set_backlight,
	.get_display_timing	= ek79007_timings,
};

static const struct udevice_id ek79007_ids[] = {
	{ .compatible = "wsvgalnl,ek79007" },
	{ }
};

U_BOOT_DRIVER(ek79007) = {
	.name		= "ek79007",
	.id		= UCLASS_PANEL,
	.of_match	= ek79007_ids,
	.ops		= &ek79007_ops,
	.of_to_plat	= ek79007_of_to_plat,
	.probe		= ek79007_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct ek79007_priv),
};
