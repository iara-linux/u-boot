// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <i2c.h>
#include <led.h>
#include <asm/gpio.h>
#include <linux/delay.h>

/* Register map */
#define IS31FL3236_REG_SHUTDOWN		0x00
#define IS31FL3236_REG_PWM_BASE		0x01
#define IS31FL3236_REG_PWM_UPDATE	0x25
#define IS31FL3236_REG_CTRL_BASE	0x26
#define IS31FL3236_REG_GLOBAL_CTRL	0x4a
#define IS31FL3236_REG_RESET		0x4f

#define IS31FL3236_CHANNELS		36

/* 0x00 Shutdown: SSD bit, 0 = software shutdown, 1 = normal operation */
#define IS31FL3236_SSD_SHUTDOWN		0x00
#define IS31FL3236_SSD_NORMAL		BIT(0)

/* 0x26..0x49 LED control: OUT enable in bit 0 */
#define IS31FL3236_CTRL_OUT_ENABLE	BIT(0)

/* 0x4a Global control: 0 = channels operate normally, 1 = all shut down */
#define IS31FL3236_GLOBAL_CTRL_NORMAL	0x00

/* Full brightness when a channel is switched on */
#define IS31FL3236_DEFAULT_PWM		0xff

struct is31fl3236_priv {
	u8 channel;
	u8 pwm;
};

struct is31fl3236_wrap_priv {
	struct gpio_desc enable;
};

static int is31fl3236_write(struct udevice *dev, u8 reg, u8 value)
{
	return dm_i2c_reg_write(dev, reg, value);
}

static int is31fl3236_set_pwm(struct udevice *dev, u8 channel, u8 value)
{
	struct udevice *chip = dev_get_parent(dev);
	int ret;

	/* Channel numbering is 1-based; PWM registers start at 0x01. */
	ret = is31fl3236_write(chip, IS31FL3236_REG_PWM_BASE + channel - 1,
			       value);
	if (ret)
		return ret;

	/* Commit shadowed values */
	return is31fl3236_write(chip, IS31FL3236_REG_PWM_UPDATE, 0);
}

static int is31fl3236_led_set_state(struct udevice *dev, enum led_state_t state)
{
	struct is31fl3236_priv *priv = dev_get_priv(dev);
	u8 pwm;
	int ret;

	switch (state) {
	case LEDST_OFF:
		pwm = 0;
		break;
	case LEDST_ON:
		pwm = IS31FL3236_DEFAULT_PWM;
		break;
	case LEDST_TOGGLE:
		pwm = priv->pwm ? 0 : IS31FL3236_DEFAULT_PWM;
		break;
	default:
		return -ENOSYS;
	}

	ret = is31fl3236_set_pwm(dev, priv->channel, pwm);
	if (ret)
		return ret;

	priv->pwm = pwm;

	return 0;
}

static enum led_state_t is31fl3236_led_get_state(struct udevice *dev)
{
	struct is31fl3236_priv *priv = dev_get_priv(dev);

	/*
	 * The PWM registers are write-only, so report the last value
	 * we wrote.
	 */
	return priv->pwm ? LEDST_ON : LEDST_OFF;
}

static int is31fl3236_led_probe(struct udevice *dev)
{
	struct is31fl3236_priv *priv = dev_get_priv(dev);
	u32 reg;
	int ret;

	ret = dev_read_u32(dev, "reg", &reg);
	if (ret) {
		dev_err(dev, "missing 'reg' property\n");
		return ret;
	}

	/* Channels are numbered 1..36 in the binding. */
	if (reg < 1 || reg > IS31FL3236_CHANNELS) {
		dev_err(dev, "channel %u out of range (1..%d)\n",
			reg, IS31FL3236_CHANNELS);
		return -EINVAL;
	}

	priv->channel = (u8)reg;
	priv->pwm = 0;

	return is31fl3236_set_pwm(dev, priv->channel, 0);
}

static const struct led_ops is31fl3236_led_ops = {
	.set_state = is31fl3236_led_set_state,
	.get_state = is31fl3236_led_get_state,
};

U_BOOT_DRIVER(is31fl3236_led) = {
	.name = "is31fl3236-led",
	.id = UCLASS_LED,
	.probe = is31fl3236_led_probe,
	.priv_auto = sizeof(struct is31fl3236_priv),
	.ops = &is31fl3236_led_ops,
};

static int is31fl3236_led_wrap_probe(struct udevice *dev)
{
	struct is31fl3236_wrap_priv *priv = dev_get_priv(dev);
	int ret, i;

	/* Get optional enable GPIO */
	ret = gpio_request_by_name(dev, "enable-gpios", 0, &priv->enable,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret && ret != -ENOENT)
		return ret;

	/* Reset registers to defaults. */
	ret = is31fl3236_write(dev, IS31FL3236_REG_RESET, 0x0);
	if (ret)
		return ret;

	/* Leave software shutdown before touching the channels. */
	ret = is31fl3236_write(dev, IS31FL3236_REG_SHUTDOWN,
			       IS31FL3236_SSD_NORMAL);
	if (ret)
		return ret;

	/*
	 * Enable the output of every channel. On/off is controlled
	 * via the PWM registers rather than the real output control.
	 */
	for (i = 0; i < IS31FL3236_CHANNELS; i++) {
		ret = is31fl3236_write(dev, IS31FL3236_REG_CTRL_BASE + i,
				       IS31FL3236_CTRL_OUT_ENABLE);
		if (ret)
			return ret;
	}

	/*
	 * The control registers are shadowed just like the PWM ones, so
	 * none of the above takes effect until the update register is
	 * written.
	 */
	return is31fl3236_write(dev, IS31FL3236_REG_PWM_UPDATE, 0);
}

static int is31fl3236_led_wrap_bind(struct udevice *dev)
{
	return led_bind_generic(dev, "is31fl3236-led");
}

static const struct udevice_id is31fl3236_led_ids[] = {
	{ .compatible = "issi,is31fl3236" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(is31fl3236_led_wrap) = {
	.name = "is31fl3236-led-wrap",
	.id = UCLASS_NOP,
	.of_match = is31fl3236_led_ids,
	.bind = is31fl3236_led_wrap_bind,
	.probe = is31fl3236_led_wrap_probe,
	.priv_auto = sizeof(struct is31fl3236_wrap_priv),
};
