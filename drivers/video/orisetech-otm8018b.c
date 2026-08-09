// SPDX-License-Identifier: GPL-2.0+

#include <backlight.h>
#include <dm.h>
#include <panel.h>
#include <log.h>
#include <mipi_dsi.h>
#include <linux/delay.h>
#include <power/regulator.h>
#include <asm/gpio.h>

struct otm8018b_priv {
	struct udevice *vcc;

	struct udevice *backlight;

	struct gpio_desc reset_gpio;
};

static struct display_timing default_timing = {
	.pixelclock.typ		= (480 + 33 + 6 + 33) * (800 + 28 + 84 + 28) * 60,
	.hactive.typ		= 480,
	.hfront_porch.typ	= 33,
	.hback_porch.typ	= 6,
	.hsync_len.typ		= 33,
	.vactive.typ		= 800,
	.vfront_porch.typ	= 28,
	.vback_porch.typ	= 84,
	.vsync_len.typ		= 28,
};

#define dsi_generic_write_seq(dsi, cmd, seq...) do {			\
		static const u8 b[] = { cmd, seq };			\
		int ret;						\
		ret = mipi_dsi_dcs_write_buffer(dsi, b, ARRAY_SIZE(b));	\
		if (ret < 0)						\
			return log_ret(ret);				\
	} while (0)

static int otm8018b_enable_backlight(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	struct mipi_dsi_device *dsi = plat->device;
	int ret;

	dsi_generic_write_seq(dsi, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0xFF, 0x80, 0x09, 0x01);
	dsi_generic_write_seq(dsi, 0x00, 0x80);
	dsi_generic_write_seq(dsi, 0xFF, 0x80, 0x09);
	dsi_generic_write_seq(dsi, 0x00, 0x80);

	dsi_generic_write_seq(dsi, 0xF5, 0x01, 0x18, 0x02, 0x18, 0x10,
					 0x18, 0x02, 0x18, 0x0e, 0x18, 0x0f, 0x20);

	dsi_generic_write_seq(dsi, 0x00, 0x90);
	dsi_generic_write_seq(dsi, 0xF5, 0x02, 0x18, 0x08, 0x18, 0x06,
					 0x18, 0x0d, 0x18, 0x0b, 0x18);

	dsi_generic_write_seq(dsi, 0x00, 0xA0);
	dsi_generic_write_seq(dsi, 0xF5, 0x10, 0x18, 0x01, 0x18, 0x14,
					 0x18, 0x14, 0x18);

	dsi_generic_write_seq(dsi, 0x00, 0xB0);
	dsi_generic_write_seq(dsi, 0xF5, 0x14, 0x18, 0x12, 0x18, 0x13,
					 0x18, 0x11, 0x18, 0x13, 0x18, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0x8B);
	dsi_generic_write_seq(dsi, 0xB0, 0x40);

	dsi_generic_write_seq(dsi, 0x00, 0xC0);
	dsi_generic_write_seq(dsi, 0xC5, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xD8, 0x43, 0x43);

	dsi_generic_write_seq(dsi, 0x00, 0xB1);
	dsi_generic_write_seq(dsi, 0xC5, 0xA9);

	dsi_generic_write_seq(dsi, 0x00, 0x90);
	dsi_generic_write_seq(dsi, 0xC5, 0x96, 0xA7, 0x01);

	dsi_generic_write_seq(dsi, 0x00, 0x82);
	dsi_generic_write_seq(dsi, 0xC5, 0xA3);

	dsi_generic_write_seq(dsi, 0x00, 0x81);
	dsi_generic_write_seq(dsi, 0xC1, 0x66);

	dsi_generic_write_seq(dsi, 0x00, 0xA0);
	dsi_generic_write_seq(dsi, 0xC1, 0xEA);

	dsi_generic_write_seq(dsi, 0x00, 0xA1);
	dsi_generic_write_seq(dsi, 0xC1, 0x08);

	dsi_generic_write_seq(dsi, 0x00, 0xA2);
	dsi_generic_write_seq(dsi, 0xC0, 0x02, 0x1B);

	dsi_generic_write_seq(dsi, 0x00, 0x80);
	dsi_generic_write_seq(dsi, 0xC4, 0x30);

	dsi_generic_write_seq(dsi, 0x00, 0x81);
	dsi_generic_write_seq(dsi, 0xC4, 0x83);

	dsi_generic_write_seq(dsi, 0x00, 0x88);
	dsi_generic_write_seq(dsi, 0xC4, 0x80);

	dsi_generic_write_seq(dsi, 0x00, 0xA1);
	dsi_generic_write_seq(dsi, 0xB3, 0x10);

	dsi_generic_write_seq(dsi, 0x00, 0xB4);
	dsi_generic_write_seq(dsi, 0xC0, 0x50);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0x36, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0x90);
	dsi_generic_write_seq(dsi, 0xC0, 0x00, 0x44, 0x00, 0x00, 0x00, 0x03);

	dsi_generic_write_seq(dsi, 0x00, 0xA6);
	dsi_generic_write_seq(dsi, 0xC1, 0x01, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0x80);
	dsi_generic_write_seq(dsi, 0xCE, 0x87, 0x03, 0x14, 0x86, 0x03, 0x14);

	dsi_generic_write_seq(dsi, 0x00, 0x90);
	dsi_generic_write_seq(dsi, 0xCE, 0x33, 0x1E, 0x14, 0x33, 0x1F, 0x14);

	dsi_generic_write_seq(dsi, 0x00, 0xA0);
	dsi_generic_write_seq(dsi, 0xCE, 0x38, 0x03, 0x03, 0x1C, 0x00,
					 0x14, 0x00, 0x38, 0x02, 0x03, 0x1D, 0x00,
					 0x14, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0xB0);
	dsi_generic_write_seq(dsi, 0xCE, 0x38, 0x01, 0x03, 0x1E, 0x00,
					 0x14, 0x00, 0x38, 0x00, 0x03, 0x1F, 0x00,
					 0x14, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0xC0);
	dsi_generic_write_seq(dsi, 0xCE, 0x30, 0x00, 0x03, 0x20, 0x00,
					 0x14, 0x00, 0x30, 0x01, 0x03, 0x21, 0x00,
					 0x14, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0xD0);
	dsi_generic_write_seq(dsi, 0xCE, 0x30, 0x02, 0x03, 0x22, 0x00,
					 0x14, 0x00, 0x30, 0x03, 0x03, 0x23, 0x00,
					 0x14, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0xC6);
	dsi_generic_write_seq(dsi, 0xCF, 0x01, 0x80);

	dsi_generic_write_seq(dsi, 0x00, 0xC9);
	dsi_generic_write_seq(dsi, 0xCF, 0x10);

	dsi_generic_write_seq(dsi, 0x00, 0xC0);
	dsi_generic_write_seq(dsi, 0xCB, 0x00, 0x54, 0x54, 0x54, 0x54,
					 0x00, 0x00, 0x54, 0x54, 0x54, 0x54, 0x00,
					 0x00, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0xD0);
	dsi_generic_write_seq(dsi, 0xCB, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x54, 0x54, 0x54, 0x54, 0x00, 0x00,
					 0x54, 0x54, 0x54);

	dsi_generic_write_seq(dsi, 0x00, 0xE0);
	dsi_generic_write_seq(dsi, 0xCB, 0x54, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0x80);
	dsi_generic_write_seq(dsi, 0xCC, 0x00, 0x26, 0x25, 0x02, 0x06,
					 0x00, 0x00, 0x0A, 0x0E, 0x0C);

	dsi_generic_write_seq(dsi, 0x00, 0x90);
	dsi_generic_write_seq(dsi, 0xCC, 0x10, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26,
					 0x25, 0x01, 0x05);

	dsi_generic_write_seq(dsi, 0x00, 0xA0);
	dsi_generic_write_seq(dsi, 0xCC, 0x00, 0x00, 0x09, 0x0D, 0x0B,
					 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0xB0);
	dsi_generic_write_seq(dsi, 0xCC, 0x00, 0x25, 0x26, 0x05, 0x01,
					 0x00, 0x00, 0x0F, 0x0B, 0x0D);

	dsi_generic_write_seq(dsi, 0x00, 0xC0);
	dsi_generic_write_seq(dsi, 0xCC, 0x09, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25,
					 0x26, 0x06, 0x02);

	dsi_generic_write_seq(dsi, 0x00, 0xD0);
	dsi_generic_write_seq(dsi, 0xCC, 0x00, 0x00, 0x10, 0x0C, 0x0E,
					 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					 0x00, 0x00, 0x00);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xD9, 0x31);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xE1, 0x06, 0x07, 0x0E, 0x0D, 0x07,
					 0x16, 0x0C, 0x0C, 0x02, 0x06, 0x05, 0x07,
					 0x0F, 0x2B, 0x27, 0x0D);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xE2, 0x06, 0x07, 0x0E, 0x0D, 0x07,
					 0x16, 0x0C, 0x0C, 0x02, 0x06, 0x05, 0x07,
					 0x0F, 0x2B, 0x27, 0x0D);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0xFF, 0xFF, 0xFF, 0xFF);

	dsi_generic_write_seq(dsi, 0x00, 0x00);
	dsi_generic_write_seq(dsi, 0x3A, 0x77);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret) {
		log_debug("%s: failed to exit sleep mode: %d\n", __func__, ret);
		return log_ret(ret);
	}

	mdelay(150);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret) {
		log_debug("%s: failed to set display on: %d\n", __func__, ret);
		return log_ret(ret);
	}

	mdelay(50);

	return 0;
}

static int otm8018b_set_backlight(struct udevice *dev, int percent)
{
	struct otm8018b_priv *priv = dev_get_priv(dev);
	int ret;

	ret = backlight_enable(priv->backlight);
	if (ret)
		return log_ret(ret);

	return backlight_set_brightness(priv->backlight, percent);
}

static int otm8018b_timings(struct udevice *dev, struct display_timing *timing)
{
	memcpy(timing, &default_timing, sizeof(*timing));
	return 0;
}

static int otm8018b_of_to_plat(struct udevice *dev)
{
	struct otm8018b_priv *priv = dev_get_priv(dev);
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

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->reset_gpio, GPIOD_IS_OUT);
	if (ret) {
		log_debug("%s: cannot decode reset-gpios (%d)\n",
			  __func__, ret);
		return log_ret(ret);
	}

	return 0;
}

static int otm8018b_hw_init(struct udevice *dev)
{
	struct otm8018b_priv *priv = dev_get_priv(dev);
	int ret;

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return log_ret(ret);

	ret = regulator_set_enable_if_allowed(priv->vcc, true);
	if (ret)
		return log_ret(ret);
	mdelay(20);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret)
		return log_ret(ret);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return log_ret(ret);

	mdelay(10);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret)
		return log_ret(ret);

	mdelay(50);

	ret = otm8018b_set_backlight(dev, 100);
	if (ret)
		return log_ret(ret);

	return 0;
}

static int otm8018b_probe(struct udevice *dev)
{
	struct mipi_dsi_panel_plat *plat = dev_get_plat(dev);
	int ret;

	/* fill characteristics of DSI data link */
	plat->lanes = 2;
	plat->format = MIPI_DSI_FMT_RGB888;
	plat->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_SYNC_PULSE |
			   MIPI_DSI_MODE_LPM | MIPI_DSI_CLOCK_NON_CONTINUOUS;

	if (plat->device) {
		plat->device->lanes = plat->lanes;
		plat->device->format = plat->format;
		plat->device->mode_flags = plat->mode_flags;

		ret = mipi_dsi_attach(plat->device);
		if (ret)
			return log_ret(ret);
	}

	return otm8018b_hw_init(dev);
}

static const struct panel_ops otm8018b_ops = {
	.enable_backlight	= otm8018b_enable_backlight,
	.set_backlight		= otm8018b_set_backlight,
	.get_display_timing	= otm8018b_timings,
};

static const struct udevice_id otm8018b_ids[] = {
	{ .compatible = "djn,otm8018b" },
	{ }
};

U_BOOT_DRIVER(otm8018b) = {
	.name		= "otm8018b",
	.id		= UCLASS_PANEL,
	.of_match	= otm8018b_ids,
	.ops		= &otm8018b_ops,
	.of_to_plat	= otm8018b_of_to_plat,
	.probe		= otm8018b_probe,
	.plat_auto	= sizeof(struct mipi_dsi_panel_plat),
	.priv_auto	= sizeof(struct otm8018b_priv),
};
