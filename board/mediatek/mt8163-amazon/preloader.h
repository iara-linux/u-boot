/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#ifndef __PRELOADER_H_
#define __PRELOADER_H_

#define BOOT_ARGUMENT_MAGIC 0x504c504c
#define SEC_LIMIT_MAGIC  0x4c4c4c4c

typedef enum {
	NORMAL_BOOT = 0,
	META_BOOT = 1,
	RECOVERY_BOOT = 2,
	SW_REBOOT = 3,
	FACTORY_BOOT = 4,
	ADVMETA_BOOT = 5,
	ATE_FACTORY_BOOT = 6,
	ALARM_BOOT = 7,
	KERNEL_POWER_OFF_CHARGING_BOOT = 8,
	LOW_POWER_OFF_CHARGING_BOOT = 9,
	FASTBOOT = 99,
	DOWNLOAD_BOOT = 100,
	UNKNOWN_BOOT
} boot_mode_t;

typedef enum {
	BR_POWER_KEY = 0,
	BR_USB,
	BR_RTC,
	BR_WDT,
	BR_WDT_BY_PASS_PWK,
	BR_TOOL_BY_PASS_PWK,
	BR_2SEC_REBOOT,
	BR_UNKNOWN,
	BR_KERNEL_PANIC,
	BR_WDT_SW,
	BR_WDT_HW
} boot_reason_t;

typedef enum {
	META_UNKNOWN_COM = 0,
	META_UART_COM,
	META_USB_COM
} meta_com_type_t;

typedef enum {
	F_FACTORY_MODE = 0x0001,
} forbidden_mode_t;

typedef struct {
	u32 addr;
	u32 arg1;
	u32 arg2;
	u32 len;
	u32 sig_len;
} da_info_t;

typedef struct {
	u32 pll_setting_num;
	u32 freq_setting_num;
	u32 low_freq_pll_setting_addr;
	u32 low_freq_cha_setting_addr;
	u32 low_freq_chb_setting_addr;
	u32 high_freq_pll_setting_addr;
	u32 high_freq_cha_setting_addr;
	u32 high_freq_chb_setting_addr;
} vcore_dvfs_info_t;

typedef struct {
	u64 start;
	u64 size;
} mem_desc_t;

typedef struct {
	u64 start;
	u64 size;
	u32 rank;
} mblock_t;

typedef struct {
	u32 mblock_num;
	mblock_t mblock[4];
} mblock_info_t;

typedef struct {
	u32 rank_num;
	mem_desc_t rank_info[4];
} dram_info_t;

typedef struct {
	unsigned int magic_num;
	forbidden_mode_t forbid_mode;
} sec_limit_t;

typedef union {
	struct {
		u32 magic;
		u32 size;
		char name[32];
		u32 loadaddr;
	};

	u8 data[512];
} part_hdr_t;

typedef struct {
	u32 magic;
	boot_mode_t boot_mode;
	u32 e_flag;
	u32 log_port;
	u32 log_baudrate;
	u8 log_enable;
	u8 part_num;
	u8 reserved[2];
	u32 dram_rank_num;
	u32 dram_rank_size[4];
	mblock_info_t mblock_info;
	dram_info_t orig_dram_info;
	mem_desc_t lca_reserved_mem;
	mem_desc_t tee_reserved_mem;
	u32 boot_reason;
	meta_com_type_t meta_com_type;
	u32 meta_com_id;
	u32 boot_time;
	da_info_t da_info;
	sec_limit_t sec_limit;
	part_hdr_t *part_info;
	u8 md_type[4];
	u32 ddr_reserve_enable;
	u32 ddr_reserve_success;
	u32 dram_buf_size;
	u32 meta_uart_port;
	u32 smc_boot_opt;
	u32 lk_boot_opt;
	u32 kernel_boot_opt;
	u32 non_secure_sram_addr;
	u32 non_secure_sram_size;
	char pl_version[8];
} boot_args_t;

#endif /* __PRELOADER_H_ */
