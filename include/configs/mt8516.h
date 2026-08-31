/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration for Pumpkin board
 *
 * Copyright (C) 2019 BayLibre, SAS
 * Author: Fabien Parent <fparent@baylibre.com
 */

#ifndef __MT8516_H
#define __MT8516_H

#if 0
#include <linux/sizes.h>

#define CFG_SYS_NS16550_COM1		0x11005000
#define CFG_SYS_NS16550_CLK		26000000

/* Environment settings */
#include <config_distro_bootcmd.h>

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0)

#define CFG_EXTRA_ENV_SETTINGS \
	"scriptaddr=0x40000000\0" \
	BOOTENV
#else
#define CFG_SYS_SDRAM_BASE 0x40000000
#endif

#endif
