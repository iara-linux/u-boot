// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <asm/global_data.h>
#include <asm/system.h>
#include <dm/uclass.h>
#include <init.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

void reset_cpu(void)
{
	struct udevice *wdt;

	if (IS_ENABLED(CONFIG_PSCI_RESET)) {
		psci_system_reset();
	} else {
		uclass_first_device(UCLASS_WDT, &wdt);
		if (wdt)
			wdt_expire_now(wdt, 0);
	}
}

int print_cpuinfo(void) 
{
	printf("CPU:   MediaTek MT8163\n");
	return 0;
}
