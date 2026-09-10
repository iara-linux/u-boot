// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <bootm.h>
#include <config.h>
#include <dm.h>
#include <env.h>
#include <idme.h>
#include <init.h>
#include <asm/global_data.h>
#include <lmb.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/arch/misc.h>

#include <linux/libfdt.h>
#include <linux/sizes.h>

#include "preloader.h"

DECLARE_GLOBAL_DATA_PTR;

const struct boot_tag *preloader_param;

#ifdef CONFIG_BOARD_LATE_INIT
#define KERNEL_COMP_SIZE	SZ_64M
#ifdef CONFIG_FASTBOOT_BUF_SIZE
#define FASTBOOT_BUF_SIZE CONFIG_FASTBOOT_BUF_SIZE
#else
#define FASTBOOT_BUF_SIZE 0
#endif

#define lmb_alloc(size, addr) lmb_alloc_mem(LMB_MEM_ALLOC_ANY, SZ_2M, addr, size, LMB_NONE)

/*
 * Stolen from arch/arm/mach-snapdragon/board.c which
 * was originally stolen from arch/arm/mach-apple/board.c
 */
static int allocate_lmb_buffers(void)
{
	u32 status = 0, fdt_status = 0;
	phys_addr_t addr;
	struct fdt_header *fdt_blob = (struct fdt_header *)gd->fdt_blob;

	/* We need to be fairly conservative here as we support boards with just 1G of TOTAL RAM */
	status |= !lmb_alloc(SZ_128M, &addr) ?
		env_set_hex("loadaddr", addr) : 1;
	status |= env_set_hex("kernel_addr_r", addr);
	status |= !lmb_alloc(SZ_128M, &addr) ?
		env_set_hex("ramdisk_addr_r", addr) : 1;
	status |= !lmb_alloc(KERNEL_COMP_SIZE, &addr) ?
		env_set_hex("kernel_comp_addr_r", addr) : 1;
	status |= env_set_hex("kernel_comp_size", KERNEL_COMP_SIZE);
	status |= !lmb_alloc(SZ_4M, &addr) ?
		env_set_hex("scriptaddr", addr) : 1;

	if (IS_ENABLED(CONFIG_FASTBOOT)) {
		status |= !lmb_alloc(FASTBOOT_BUF_SIZE, &addr) ?
			env_set_hex("fastboot_addr_r", addr) : 1;
		/*
		 * Override loadaddr for memory rich soc since ${loadaddr} and
		 * ${kernel_addr_r} need to be different for the Android boot image
		 * flow. It's typically safe for ${loadaddr} to be the same address
		 * as the fastboot buffer.
		 */
		status |= env_set_hex("loadaddr", addr);
	}

	fdt_status |= !lmb_alloc(SZ_4M, &addr) ?
		env_set_hex("fdt_addr_r", addr) : 1;

	if (IS_ENABLED(CONFIG_OF_LIBFDT_OVERLAY)) {
		status |= !lmb_alloc(SZ_2M, &addr) ?
			env_set_hex("fdtoverlay_addr_r", addr) : 1;
	}

	if (status || fdt_status)
		log_warning("%s: Failed to set run time variables\n", __func__);

	/* By default copy U-Boots FDT, it will be used as a fallback */
	if (fdt_status)
		log_warning("%s: Failed to reserve memory for copying FDT\n",
			    __func__);
	else
		memcpy((void *)addr, (void *)gd->fdt_blob,
		       fdt32_to_cpu(fdt_blob->totalsize));

	return 0;
}

static void set_device_info(void)
{
	ofnode root = ofnode_root();
	const char *dev_compatible, *model;
	char *ptr;
	char buf[128];
	char manufacturer[64];
	int nr_compatibles;
	int ret;

	nr_compatibles = ofnode_read_string_count(root, "compatible");
	if (nr_compatibles < 2) {
		log_warning("%s: expected 2 or more compatible strings\n",
				__func__);
		return;
	}

	ret = ofnode_read_string_index(root, "compatible", 0,
					   &dev_compatible);
	if (ret) {
		log_warning("%s: failed to read device compatible\n",
				__func__);
		return;
	}

	ret = ofnode_read_string_index(root, "model", 0,
					   &model);
	if (ret) {
		log_warning("%s: failed to read device model\n",
				__func__);
		return;
	}
	env_set("model", model);

	/* <manufacturer>,<codename> => board = <manufacturer>-<codename> */
	strlcpy(manufacturer, dev_compatible, sizeof(manufacturer) - 1);
	ptr = strchr(manufacturer, ',');
	if (ptr)
		*ptr = '\0'; 

	strlcpy(buf, dev_compatible, sizeof(buf) - 1);
	ptr = strchr(buf, ',');
	if (ptr)
		*ptr = '-';
	env_set("board", buf);

	ptr = strchr(dev_compatible, ',');
	if (ptr)
		dev_compatible = ptr + 1;

	snprintf(buf, sizeof(buf), "mediatek/mt8516-%s-%s.dtb", 
			 manufacturer, dev_compatible);
	env_set("fdtfile", buf);
}
#endif

static const struct boot_tag_mem*
get_mem_boot_tag(const struct boot_tag *start)
{
	const struct boot_tag *cur = start;

	/* find memory boot tag */
	while (cur->hdr.size && cur->hdr.tag != BOOT_TAG_MEM)
		cur = boot_tag_next(cur);

	if (cur->hdr.tag != BOOT_TAG_MEM)
		return NULL;

	return &cur->tag.mem;
}

int dram_init(void)
{
	unsigned int i;
	const struct boot_tag_mem *mem_atag = NULL;

	if (((size_t)preloader_param >= CFG_SYS_SDRAM_BASE) &&
	    ((size_t)preloader_param % sizeof(size_t) == 0))
		mem_atag = get_mem_boot_tag(preloader_param);

	if (mem_atag &&
		mem_atag->dram_rank_num <= ARRAY_SIZE(mem_atag->dram_rank_size)) {
		gd->ram_size = 0;

		for (i = 0; i < mem_atag->dram_rank_num; i++)
			gd->ram_size += mem_atag->dram_rank_size[i];
		
		return 0;
	}

	/* SZ_512M is a safe maximum in this case. */
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE,
				    SZ_512M);
	return 0;
}

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

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	allocate_lmb_buffers();
	set_device_info();

	return 0;
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret;
	(void)ret;

#ifdef CONFIG_CMD_IDME
	ret = idme_fdt_fixup_mac_addrs(blob);
	if (ret)
		return ret;
#endif
	return 0;
}
#endif
