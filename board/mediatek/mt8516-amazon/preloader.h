/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#ifndef __PRELOADER_H_
#define __PRELOADER_H_

#define MBLOCK_RESERVED_NAME_SIZE	128
#define MBLOCK_RESERVED_NUM_MAX		128
#define MBLOCK_NUM_MAX			128
#define MBLOCK_MAGIC			0x99999999
#define MBLOCK_VERSION			0x2

struct mem_desc {
	uint64_t start;
	uint64_t size;
};

struct mblock {
	uint64_t start;
	uint64_t size;
	uint32_t rank;
};

struct reserved {
	uint64_t start;
	uint64_t size;
	uint32_t mapping;
	char name[MBLOCK_RESERVED_NAME_SIZE];
};

struct mblock_info {
	uint32_t mblock_num;
	struct mblock mblock[MBLOCK_NUM_MAX];
	uint32_t mblock_magic;
	uint32_t mblock_version;
	uint32_t reserved_num;
	struct reserved reserved[MBLOCK_RESERVED_NAME_SIZE];
};

struct dram_info {
	uint32_t rank_num;
	struct mem_desc rank_info[4];
};

#define BOOT_TAG_MEM	0x88610005
struct boot_tag_mem {
	uint32_t dram_rank_num;
	uint32_t dram_rank_size[4];
	struct mblock_info mblock_info;
	struct dram_info orig_dram_info;
	struct mem_desc lca_reserved_mem;
	struct mem_desc tee_reserved_mem;
};

struct boot_tag_header {
	uint32_t size;
	uint32_t tag;
};

struct boot_tag {
	struct boot_tag_header hdr;
	union {
		struct boot_tag_mem mem;
	} tag;
};

#define boot_tag_next(t)	((struct boot_tag *)((uint32_t *)(t) + (t)->hdr.size))
#define boot_tag_size(type)	((sizeof(struct boot_tag_header) + sizeof(struct type)) >> 2)

#endif /* __PRELOADER_H_ */
