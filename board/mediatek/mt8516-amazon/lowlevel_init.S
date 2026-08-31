/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <linux/linkage.h>

.extern	preloader_param

ENTRY(save_boot_params)
	adrp	x6, preloader_param
	add	x6, x6, :lo12:preloader_param
	str	x4, [x6]
	b	save_boot_params_ret
ENDPROC(save_boot_params)
