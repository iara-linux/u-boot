// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>

int board_init(void)
{
	struct udevice *dev;
	int ret;

	if (CONFIG_IS_ENABLED(USB_GADGET)) {
		ret = uclass_get_device(UCLASS_USB_GADGET_GENERIC, 0, &dev);
		if (ret) {
			pr_err("%s: Cannot find USB device\n", __func__);
			return ret;
		}
	}

	return 0;
}
