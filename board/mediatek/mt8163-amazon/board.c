// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <bootm.h>
#include <config.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <lmb.h>
#include <video.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include <asm/arch/misc.h>

#include <linux/libfdt.h>
#include <linux/sizes.h>
#include <linux/types.h>

#include "preloader.h"

DECLARE_GLOBAL_DATA_PTR;

boot_args_t *preloader_param;

#if CONFIG_IS_ENABLED(VIDEO)
#define MT8163_DISP_OVL0_BASE	0x14007000UL
#define DISP_REG_OVL_EN		0x000c
#define DISP_REG_OVL_RST	0x0014
#define DISP_REG_OVL_SRC_CON	0x002c
#endif

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

	snprintf(buf, sizeof(buf), "mediatek/mt8163-%s-%s.dtb", 
			 manufacturer, dev_compatible);
	env_set("fdtfile", buf);
}
#endif

int dram_init(void)
{
	unsigned int i;

	if (((size_t)preloader_param >= CFG_SYS_SDRAM_BASE) &&
	    ((size_t)preloader_param % sizeof(size_t) == 0) &&
	    preloader_param->magic == BOOT_ARGUMENT_MAGIC &&
	    preloader_param->dram_rank_num <=
	    ARRAY_SIZE(preloader_param->dram_rank_size)) {
		gd->ram_size = 0;

		for (i = 0; i < preloader_param->dram_rank_num; i++)
			gd->ram_size += preloader_param->dram_rank_size[i];
	} else {
		/* SZ_256M is a safe maximum in this case. */
		gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE,
					    SZ_256M);
	}

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

#if CONFIG_IS_ENABLED(VIDEO)
static inline void ovl_writel(u32 val, unsigned long off)
{
	writel(val, (void __iomem *)(MT8163_DISP_OVL0_BASE + off));
}

static void ovl_disable(void)
{
	ovl_writel(0, DISP_REG_OVL_SRC_CON);
	ovl_writel(0, DISP_REG_OVL_EN);

	ovl_writel(1, DISP_REG_OVL_RST);
	udelay(10);
	ovl_writel(0, DISP_REG_OVL_RST);
}
#endif

void board_preboot_os(void)
{
#if CONFIG_IS_ENABLED(VIDEO)
	ovl_disable();
#endif
}

#if CONFIG_IS_ENABLED(VIDEO)

#ifdef CONFIG_OF_BOARD_SETUP
static const char *panel_compatibles[] = {
#ifdef CONFIG_VIDEO_LCD_AMAZON_JD936X
	"amazon,crown-jd9365",
	"amazon,crown-ili9881",
#endif
#ifdef CONFIG_VIDEO_LCD_AMAZON_ST7701S
	"amazon,checkers-st7701s",
	"amazon,cronos-st7701s",
	"amazon,cronos-gc9503np",
#endif
};

static int fdt_fixup_panel_node(void *blob)
{
	const char *new_panel_compat, *match_generic;
	int offset, ret;
	u32 i;

	new_panel_compat = env_get("panel");
	if (!new_panel_compat)
		return -1;

	for (i = 0; i < ARRAY_SIZE(panel_compatibles); i++) {
		offset = fdt_node_offset_by_compatible(blob, -1, panel_compatibles[i]);
		if (offset > 0) {
			match_generic = panel_compatibles[i];
			break;
		}
	}

	if (offset < 0) {
		printf("failed to find panel node by compatible, skipping\n");
		return 0;
	}

	ret = fdt_setprop_string(blob, offset, "compatible", new_panel_compat);
	if (ret < 0) {
		printf("failed to update panel compatible: %d\n", ret);
		return -1;
	}

	ret = fdt_appendprop_string(blob, offset, "compatible", match_generic);
	if (ret < 0) {
		printf("failed to append panel compatible: %d\n", ret);
		return -1;
	}

	/* Now we can set status = "okay" on it. */
	ret = fdt_status_okay(blob, offset);
	if (ret < 0) {
		printf("Failed to set status okay on panel node: %d.\n", ret);
		return -1;
	}

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */
#endif /* CONFIG_IS_ENABLED(VIDEO) */

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret;
	(void)ret;

#if CONFIG_IS_ENABLED(VIDEO)
	ret = fdt_fixup_panel_node(blob);
	if (ret)
		return ret;
#endif
	return 0;
}
#endif
