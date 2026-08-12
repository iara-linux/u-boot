// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Ondrej Jirman <megi@xff.cz>
 */

#include <errno.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <touchpanel.h>

int touchpanel_start(struct udevice *dev)
{
	const struct touchpanel_ops *ops = touchpanel_get_ops(dev);

	if (!ops || !ops->start)
		return -ENOSYS;

	return ops->start(dev);
}

int touchpanel_stop(struct udevice *dev)
{
	const struct touchpanel_ops *ops = touchpanel_get_ops(dev);

	if (!ops || !ops->stop)
		return -ENOSYS;

	return ops->stop(dev);
}

static void touchpanel_orient(struct touchpanel_priv *uc_priv,
			      struct touchpanel_touch *touch)
{
	int tmp;

	if (uc_priv->inverted_x && uc_priv->max_x > 0)
		touch->x = uc_priv->max_x - 1 - touch->x;

	if (uc_priv->inverted_y && uc_priv->max_y > 0)
		touch->y = uc_priv->max_y - 1 - touch->y;

	if (uc_priv->swapped_x_y) {
		tmp = touch->x;
		touch->x = touch->y;
		touch->y = tmp;
	}
}

int touchpanel_get_touches(struct udevice *dev,
			   struct touchpanel_touch* touches, int max_touches)
{
	const struct touchpanel_ops *ops = touchpanel_get_ops(dev);
	struct touchpanel_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret, i;

	if (!ops || !ops->get_touches)
		return -ENOSYS;

	ret = ops->get_touches(dev, touches, max_touches);
	if (ret <= 0 || !touches)
		return ret;

	for (i = 0; i < ret && i < max_touches; i++)
		touchpanel_orient(uc_priv, &touches[i]);

	return ret;
}

static int touchpanel_pre_probe(struct udevice *dev)
{
	struct touchpanel_priv *uc_priv;

	uc_priv = dev_get_uclass_priv(dev);
	if (!uc_priv)
		return -ENXIO;

	uc_priv->max_x = dev_read_u32_default(dev, "touchscreen-size-x",
					      -ENODATA);
	uc_priv->max_y = dev_read_u32_default(dev, "touchscreen-size-y",
					      -ENODATA);

	if (uc_priv->max_x == -ENODATA || uc_priv->max_y == -ENODATA)
		uc_priv->max_x = uc_priv->max_y = -ENODATA;

	uc_priv->inverted_x = dev_read_bool(dev, "touchscreen-inverted-x");
	uc_priv->inverted_y = dev_read_bool(dev, "touchscreen-inverted-y");
	uc_priv->swapped_x_y = dev_read_bool(dev, "touchscreen-swapped-x-y");

	if (uc_priv->swapped_x_y) {
		uc_priv->size_x = uc_priv->max_y;
		uc_priv->size_y = uc_priv->max_x;
	} else {
		uc_priv->size_x = uc_priv->max_x;
		uc_priv->size_y = uc_priv->max_y;
	}

	return 0;
}

UCLASS_DRIVER(touchpanel) = {
	.id		= UCLASS_TOUCHPANEL,
	.name		= "touchpanel",
	.pre_probe	= touchpanel_pre_probe,
	.per_device_auto = sizeof(struct touchpanel_priv),
};
