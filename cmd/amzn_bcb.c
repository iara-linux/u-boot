// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <bcblib.h>
#include <blk.h>
#include <command.h>
#include <env.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <vsprintf.h>
#include <linux/err.h>

#define AMZN_BCB_BUF_SZ 4096

static u8 buf[AMZN_BCB_BUF_SZ] __aligned(ARCH_DMA_MINALIGN);
static struct disk_partition partition_data;
static struct blk_desc *block;
static struct bcb *bcb;
static lbaint_t bcb_blk;
static lbaint_t bcb_cnt;

static int amzn_bcb_not_loaded(void)
{
	printf("FAIL: Please, load BCB first!\n");
	return CMD_RET_FAILURE;
}

static void amzn_bcb_reset(void)
{
	block = NULL;
	bcb = NULL;
	memset(buf, 0, sizeof(buf));
}

static int amzn_bcb_priority_slot(void)
{
	return bcblib_metadata_get_priority(&bcb->slot[1]) >
	       bcblib_metadata_get_priority(&bcb->slot[0]) ? 1 : 0;
}

static int amzn_bcb_check_preloader_slot(void)
{
	return bcblib_bcb_get_active_slot(bcb, false, false);
}

static void amzn_bcb_make_preferred(int idx)
{
	bcblib_metadata_set_priority(&bcb->slot[idx], 15);
	bcblib_metadata_set_priority(&bcb->slot[!idx], 14);
}

static int do_amzn_bcb_load(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct disk_partition *part = &partition_data;
	const char *iface = "mmc";
	unsigned long byte_off;
	int devnum, ret;
	char *endp;

	if (argc < 3)
		return CMD_RET_USAGE;

	if (argc == 4) {
		iface = argv[1];
		argc--;
		argv++;
	}

	devnum = simple_strtoul(argv[1], &endp, 0);
	if (*endp != '\0') {
		printf("FAIL: Device id '%s' not a number\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	amzn_bcb_reset();

	ret = blk_get_device_by_str(iface, argv[1], &block);
	if (ret < 0)
		return CMD_RET_FAILURE;

	ret = part_get_info_by_name(block, argv[2], part);
	if (ret < 0) {
		printf("FAIL: cannot find partition '%s'\n", argv[2]);
		block = NULL;
		return CMD_RET_FAILURE;
	}

	bcb_blk = BCB_OFFSET / part->blksz;
	byte_off = BCB_OFFSET % part->blksz;
	bcb_cnt = DIV_ROUND_UP(byte_off + sizeof(struct bcb), part->blksz);

	if (bcb_cnt * part->blksz > sizeof(buf)) {
		printf("FAIL: block size %lu too large\n", (ulong)part->blksz);
		block = NULL;
		return CMD_RET_FAILURE;
	}
	if (bcb_blk + bcb_cnt > part->size) {
		printf("FAIL: partition '%s' too small\n", argv[2]);
		block = NULL;
		return CMD_RET_FAILURE;
	}

	if (blk_dread(block, part->start + bcb_blk, bcb_cnt, buf) != bcb_cnt) {
		printf("FAIL: read failed\n");
		block = NULL;
		return CMD_RET_FAILURE;
	}

	bcb = (struct bcb *)(buf + byte_off);
	return CMD_RET_SUCCESS;
}

static int do_amzn_bcb_commit(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	if (!bcb)
		return amzn_bcb_not_loaded();

	if (blk_dwrite(block, partition_data.start + bcb_blk, bcb_cnt, buf) !=
	    bcb_cnt) {
		printf("FAIL: write failed\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static void amzn_bcb_print(void)
{
	int active = amzn_bcb_check_preloader_slot();

	printf("BCB [Magic: 0x%08x | Ver: %d]\n", bcb->magic, bcb->version);
	printf("Slot A: prio=%-2d tries=%-1d success=%d\n",
	       bcb->slot[0].priority, bcb->slot[0].tries, bcb->slot[0].success);
	printf("Slot B: prio=%-2d tries=%-1d success=%d\n",
	       bcb->slot[1].priority, bcb->slot[1].tries, bcb->slot[1].success);
	if (active >= 0)
		printf("Active slot: %c\n", 'a' + active);
	else
		printf("Active slot: NONE; WILL FAIL TO BOOT!\n");
}

static int do_amzn_bcb_dump(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	if (!bcb)
		return amzn_bcb_not_loaded();

	amzn_bcb_print();
	return CMD_RET_SUCCESS;
}

/*
 * NOTE: This function is written under the assumption that Preloader has
 * already decremented the try from the current slot if its not successful
 * but has higher priority.
 */
static int amzn_bcb_select(int argc, char *const argv[], bool fixup)
{
	char slot[2] = { 0, 0 };
	int active, next;

	if (!bcb)
		return amzn_bcb_not_loaded();

	if (!bcblib_bcb_magic_valid(bcb)) {
		printf("BAD: BCB header is corrupted, PLEASE RUN 'amzn_bcb repair'\n");
		return CMD_RET_FAILURE;
	}

	active = amzn_bcb_check_preloader_slot();
	if (active >= 0)
		goto out; // all good!

	/*
	 * Preloader has already decremented the preferred slots last try to
	 * get to this point. In this situation the slot we are currently running
	 * from is the preferred one that has a last-chance to boot successfully.
	 */
	active = amzn_bcb_priority_slot();

	if (!fixup) {
		printf("BAD: slot %c has no more tries, next boot would brick!\n",
		       'a' + active);
		return CMD_RET_FAILURE;
	}

	/*
	 * Re-run the slot selection but allow fallback to a lower priority
	 * slot that isn't successful but still has tries.
	 */
	next = bcblib_bcb_get_active_slot(bcb, false, true);
	if (next >= 0) {
		/* And then make that the preferred slot. */
		amzn_bcb_make_preferred(next);
		printf("WARN: slot %c has no more tries, make next boot be on slot %c\n",
		       'a' + active, 'a' + next);
	} else {
		/*
		 * Neither slot has a try left and neither is successful.
		 *
		 * If we do not correct this, then Preloader is going to die on the
		 * next boot, so we make the current slot successful with the rationale
		 * being that if this slot at least gets into U-boot, the user has a shot
		 * to recover, while they would be screwed if the device got stuck in Preloader
		 * and there is no way to get into BROM USBDL as a result of a bad BCB.
		 */
		bcblib_metadata_set_success(&bcb->slot[active], true);
		amzn_bcb_make_preferred(active);
		printf("WARN: no tries left anywhere, marking slot %c successful\n",
		       'a' + active);
	}

out:
	slot[0] = 'a' + active;
	if (argc >= 2)
		env_set(argv[1], slot);
	else
		printf("%s\n", slot);

	return CMD_RET_SUCCESS;
}

static int do_amzn_bcb_get_active(struct cmd_tbl *cmdtp, int flag, int argc,
				  char *const argv[])
{
	return amzn_bcb_select(argc, argv, false);
}

static int do_amzn_bcb_boot_slot(struct cmd_tbl *cmdtp, int flag, int argc,
				 char *const argv[])
{
	return amzn_bcb_select(argc, argv, true);
}

static int do_amzn_bcb_repair(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	int slot;

	if (!bcb)
		return amzn_bcb_not_loaded();

	// Sanity check BCB
	printf("CHECK: Sanity check BCB...\n");

	if (!bcblib_bcb_magic_valid(bcb)) {
		printf("CHECK: BCB magic is invalid, reinitialising\n");
		bcblib_bcb_init(bcb);

		// Make sure we mark a slot as successful, the defaults will
		// eventually brick the device.
		bcb->slot[0] = BCB_SLOT_METADATA_ACTIVE;
		bcb->slot[1] = BCB_SLOT_METADATA_EMPTY;
		amzn_bcb_print();
		goto bcb_repaired;
	}

	if (amzn_bcb_check_preloader_slot() >= 0) {
		printf("OK: Nothing to do\n");
		return CMD_RET_SUCCESS;
	}

	// Re-pick the current slot with fallback allowed.
	slot = bcblib_bcb_get_active_slot(bcb, false, true);
	if (slot < 0)
		slot = 0; // fallback to slot a

	bcblib_metadata_set_success(&bcb->slot[slot], true);
	amzn_bcb_make_preferred(slot);

bcb_repaired:
	// Commit the BCB now. Do not wait for the user to commit
	// it themselves.
	if (do_amzn_bcb_commit(cmdtp, flag, argc, argv) == CMD_RET_SUCCESS) {
		printf("OK: BCB Repairs were carried out\n");
		return CMD_RET_SUCCESS;
	} else {
		printf("FAIL: BCB Repairs were carried out but could not be written\n");
		return CMD_RET_FAILURE;
	}
}

static int do_amzn_bcb_check(struct cmd_tbl *cmdtp, int flag, int argc,
			     char *const argv[])
{
	if (!bcb)
		return amzn_bcb_not_loaded();

	if (!bcblib_bcb_magic_valid(bcb)) {
		printf("BAD: BCB header is corrupted, PLEASE RUN 'amzn_bcb repair'\n");
		return CMD_RET_FAILURE;
	}
	if (amzn_bcb_check_preloader_slot() < 0) {
		printf("BAD: no bootable slot, PLEASE RUN 'amzn_bcb repair'\n");
		return CMD_RET_FAILURE;
	}

	printf("OK: slot %c is active slot\n", 'a' + amzn_bcb_check_preloader_slot());
	return CMD_RET_SUCCESS;
}

static int amzn_bcb_slot_arg(const char *arg, bcb_slot_metadata_t **out)
{
	*out = bcblib_bcb_get_slot(bcb, arg[0]);
	if (!*out) {
		printf("FAIL: unknown slot '%s', expected 'a' or 'b'\n", arg);
		return -1;
	}
	return 0;
}

static int do_amzn_bcb_get(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	bcb_slot_metadata_t *s;

	if (argc < 3)
		return CMD_RET_USAGE;
	if (!bcb)
		return amzn_bcb_not_loaded();
	if (amzn_bcb_slot_arg(argv[1], &s))
		return CMD_RET_FAILURE;

	if (!strcmp(argv[2], "prio"))
		printf("%d\n", bcblib_metadata_get_priority(s));
	else if (!strcmp(argv[2], "tries"))
		printf("%d\n", bcblib_metadata_get_tries(s));
	else if (!strcmp(argv[2], "success"))
		printf("%d\n", bcblib_metadata_get_success(s));
	else if (!strcmp(argv[2], "all"))
		printf("prio: %d tries: %d success: %d\n",
		       bcblib_metadata_get_priority(s),
		       bcblib_metadata_get_tries(s),
		       bcblib_metadata_get_success(s));
	else
		return CMD_RET_USAGE;

	return CMD_RET_SUCCESS;
}

static int do_amzn_bcb_set(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	bcb_slot_metadata_t *s;
	ulong val;

	if (argc < 4)
		return CMD_RET_USAGE;
	if (!bcb)
		return amzn_bcb_not_loaded();
	if (amzn_bcb_slot_arg(argv[1], &s))
		return CMD_RET_FAILURE;

	val = simple_strtoul(argv[3], NULL, 0);

	if (!strcmp(argv[2], "prio"))
		bcblib_metadata_set_priority(s, val);
	else if (!strcmp(argv[2], "tries"))
		bcblib_metadata_set_tries(s, val);
	else if (!strcmp(argv[2], "success"))
		bcblib_metadata_set_success(s, !!val);
	else
		return CMD_RET_USAGE;

	return CMD_RET_SUCCESS;
}

static int amzn_bcb_mark_active(char slot)
{
	if (slot == 'a') {
		bcb->slot[0] = BCB_SLOT_METADATA_ACTIVE;
		bcb->slot[1] = BCB_SLOT_METADATA_EMPTY;
	} else if (slot == 'b') {
		bcb->slot[1] = BCB_SLOT_METADATA_ACTIVE;
		bcb->slot[0] = BCB_SLOT_METADATA_EMPTY;
	} else {
		printf("FAIL: unknown slot '%c', expected 'a' or 'b'\n", slot);
		return -1;
	}
	return 0;
}

static int do_amzn_bcb_set_active(struct cmd_tbl *cmdtp, int flag, int argc,
				  char *const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;
	if (!bcb)
		return amzn_bcb_not_loaded();
	if (amzn_bcb_mark_active(argv[1][0]))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static int do_amzn_bcb_init(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	if (!bcb)
		return amzn_bcb_not_loaded();

	bcblib_bcb_init(bcb);

	if (argc >= 2) {
		if (amzn_bcb_mark_active(argv[1][0]))
			return CMD_RET_FAILURE;
	} else {
		printf("Warning: no slot marked bootable. This may eventually BRICK your device.\n");
	}

	amzn_bcb_print();
	return CMD_RET_SUCCESS;
}

static char amzn_bcb_help_text[] =
	"load [<iface>] <dev> <part> - load slot metadata from <part>\n"
	"amzn_bcb dump - show the current state\n"
	"amzn_bcb get <a|b> <prio|tries|success|all> - read a field\n"
	"amzn_bcb set <a|b> <prio|tries|success> <val> - write a field\n"
	"amzn_bcb get_active [<var>] - active booted slot, follows preloader logic\n"
	"amzn_bcb boot_slot [<var>] - slot to boot now, decrements tries if slot !successful (needs commit afterwards)\n"
	"amzn_bcb set_active <a|b> - mark one slot bootable and the other unbootable\n"
	"amzn_bcb init [<a|b>] - reinitialise bcb metadata, use this if corrupted\n"
	"amzn_bcb check - verify preloader can still get a bootable slot\n"
	"amzn_bcb repair - repair metadata (to avoid possible brick)\n"
	"amzn_bcb commit - commit changes back to partition\n"
	"\n"
	"Changes stay in memory until 'commit'.\n";

U_BOOT_CMD_WITH_SUBCMDS(amzn_bcb,
	"Amazon BCB metadata", amzn_bcb_help_text,
	U_BOOT_SUBCMD_MKENT(load, 4, 1, do_amzn_bcb_load),
	U_BOOT_SUBCMD_MKENT(dump, 1, 1, do_amzn_bcb_dump),
	U_BOOT_SUBCMD_MKENT(get, 3, 1, do_amzn_bcb_get),
	U_BOOT_SUBCMD_MKENT(set, 4, 1, do_amzn_bcb_set),
	U_BOOT_SUBCMD_MKENT(get_active, 2, 1, do_amzn_bcb_get_active),
	U_BOOT_SUBCMD_MKENT(boot_slot, 2, 1, do_amzn_bcb_boot_slot),
	U_BOOT_SUBCMD_MKENT(set_active, 2, 1, do_amzn_bcb_set_active),
	U_BOOT_SUBCMD_MKENT(init, 2, 1, do_amzn_bcb_init),
	U_BOOT_SUBCMD_MKENT(check, 1, 1, do_amzn_bcb_check),
	U_BOOT_SUBCMD_MKENT(repair, 1, 1, do_amzn_bcb_repair),
	U_BOOT_SUBCMD_MKENT(commit, 1, 1, do_amzn_bcb_commit),
);
