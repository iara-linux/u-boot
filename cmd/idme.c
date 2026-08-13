// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 *
 * Based on "idmetool - tool for manipulating amazon idme data"
 * Copyright (c) 2026 Roger Ortiz <me@r0rt1z2.com>
 */

#include <blk.h>
#include <command.h>
#include <env.h>
#include <idmelib.h>
#include <malloc.h>
#include <mmc.h>
#include <vsprintf.h>
#include <string.h>
#include <asm/cache.h>
#include <linux/ctype.h>
#include <linux/stringify.h>
#include <linux/types.h>

/* parse a substring of length 'n' at offset 'off' as a hex integer. */
static unsigned int parse_hex_substr(const char *str, size_t off, size_t n)
{
	char tmp[8] = {0};
	size_t slen = strlen(str);

	if (off + n > slen || n >= sizeof(tmp))
		return 0;

	memcpy(tmp, str + off, n);
	return (unsigned int)simple_strtoul(tmp, NULL, 16);
}

static void idme_print_binary_item(const struct idme_item *item)
{
	u32 i;

	for (i = 0; i < item->desc.size; i++)
		printf("%02x", item->data[i]);
}

static void idme_print_string_item(const struct idme_item *item)
{
	printf("%.*s", item->desc.size, (const char *)item->data);
}

static void idme_print_bootmode_item(const struct idme_item *item)
{
	int mode = (int)simple_strtol(item->data, NULL, 10);
	printf("(%s)", idmelib_bootmode_to_str(mode));
}

static void idme_print_board_id_item(const struct idme_item *item)
{
	/* [0..3] board type, [7..8] board rev, [5] WAN flag */
	unsigned int board_type = parse_hex_substr(item->data, 0, 4);
	unsigned int board_rev = parse_hex_substr(item->data, 7, 2);
	bool has_wan = (item->data[0] == '0' && item->data[5] == '1');

	printf("(type=0x%04x rev=0x%02x wan=%s)",
		   board_type, board_rev,
		   has_wan ? "yes" : "no");
}

static void idme_print_flags_item(const struct idme_item *item)
{
	char flags_str[256];
	unsigned long long flags = simple_strtoull(item->data, NULL, 16);
	if (idmelib_flags_to_str(flags, flags_str,
				 sizeof(flags_str)) == 0)
		printf("(%s)", flags_str);
	else
		printf("(invalid)");
}

typedef void (*idme_print_value_fn)(const struct idme_item *item);
static struct {
	const char *name;
	idme_print_value_fn func;
} item_printers[] = {
	{"KB", idme_print_binary_item},
	{"DKB", idme_print_binary_item},
	{"bootmode", idme_print_bootmode_item},
	{"board_id", idme_print_board_id_item},
	{"fos_flags", idme_print_flags_item},
	{"dev_flags", idme_print_flags_item},
	{"usr_flags", idme_print_flags_item},
};

static void idme_print_value(const struct idme_item *item)
{
	u32 i;

	for (i = 0; i < ARRAY_SIZE(item_printers); i++) {
		if (strncmp(item->desc.name,
			item_printers[i].name, IDME_MAX_NAME_LEN) == 0) {
			item_printers[i].func(item);
			return;
		}
	}

	idme_print_string_item(item);
}

static void idme_hexdump(const struct idme_item *item)
{
	u32 off, i;

	for (off = 0; off < item->desc.size; off += 16) {
		u32 cols = min_t(u32, item->desc.size - off, 16);

		printf("  %04x  ", off);

		for (i = 0; i < 16; i++) {
			if (i == 8)
				printf(" ");
			if (i < cols)
				printf("%02x ", item->data[off + i]);
			else
				printf("   ");
		}

		printf(" |");
		for (i = 0; i < cols; i++) {
			u8 c = item->data[off + i];

			printf("%c", isprint(c) ? c : '.');
		}
		printf("|\n");
	}
}

static int idme_flag_type(const char *str, enum idme_flag_type *type)
{
	if (!strcmp(str, "fos"))
		*type = IDME_FLAGS_FOS;
	else if (!strcmp(str, "dev"))
		*type = IDME_FLAGS_DEV;
	else if (!strcmp(str, "usr"))
		*type = IDME_FLAGS_USR;
	else
		return -1;

	return 0;
}

static int idme_flag_value(const char *str, unsigned long long *out)
{
	char *end;

	*out = simple_strtoull(str, &end, 0);
	if (end != str && !*end)
		return 0;

	return idmelib_flags_parse_name(str, out);
}

static void idme_print_flags(enum idme_flag_type type, unsigned long long val)
{
	char sym[256];

	if (idmelib_flags_to_str(val, sym, sizeof(sym)))
		sym[0] = '\0';

	printf("%s = 0x%llx (%s)\n", idmelib_flags_var_name(type), val, sym);
}

static int idme_access(void *buf, bool write)
{
	struct blk_desc *desc;
	struct mmc *mmc;
	int part, ret = 0;
	ulong n;

	mmc = find_mmc_device(CONFIG_CMD_IDME_MMC_DEV);
	if (!mmc) {
		printf("idme: no mmc device %d\n", CONFIG_CMD_IDME_MMC_DEV);
		return -ENODEV;
	}

	if (mmc_init(mmc)) {
		printf("idme: cannot init mmc %d\n", CONFIG_CMD_IDME_MMC_DEV);
		return -EIO;
	}

	desc = mmc_get_blk_desc(mmc);
	if (!desc)
		return -ENODEV;

	part = desc->hwpart;

	ret = blk_dselect_hwpart(desc, CONFIG_CMD_IDME_MMC_HWPART);
	if (ret) {
		printf("idme: cannot select hwpart %d (%d)\n",
		       CONFIG_CMD_IDME_MMC_HWPART, ret);
		return ret;
	}

	if (write)
		n = blk_dwrite(desc, 0, IDME_NUM_BLOCKS, buf);
	else
		n = blk_dread(desc, 0, IDME_NUM_BLOCKS, buf);

	if (n != IDME_NUM_BLOCKS) {
		printf("idme: %s failed (%lu / %d blocks)\n",
		       write ? "write" : "read", n, IDME_NUM_BLOCKS);
		ret = -EIO;
	}

	if (blk_dselect_hwpart(desc, part))
		printf("idme: warning, could not restore hwpart %d\n", part);

	return ret;
}

static struct idme *idme_load(void)
{
	struct idme *hdr;

	hdr = memalign(ARCH_DMA_MINALIGN, IDME_SIZE);
	if (!hdr)
		return NULL;

	if (idme_access(hdr, false))
		goto fail;

	if (!idmelib_magic_valid(hdr)) {
		printf("idme: bad magic on mmc %d hwpart %d, expected %s\n",
		       CONFIG_CMD_IDME_MMC_DEV, CONFIG_CMD_IDME_MMC_HWPART,
		       IDME_MAGIC);
		goto fail;
	}

	return hdr;

fail:
	free(hdr);

	return NULL;
}

static void idme_list(struct idme *hdr)
{
	struct idme_item *item = idmelib_first_item(hdr);
	char version[IDME_VERSION_LEN + 1];
	u32 i;

	if (idmelib_get_version(hdr, version, sizeof(version)))
		version[0] = '\0';

	printf("IDME [Magic: %.8s | Ver: %.3s | Items: %u]\n",
	       hdr->magic, hdr->version, hdr->items_num);

	for (i = 0; i < hdr->items_num; i++) {
		char perm_str[10];

		idmelib_permission_to_str(item->desc.permission, perm_str);
		printf("  %-16s [size=%-4u exp=%u perm=%s] ",
		       item->desc.name, item->desc.size,
		       item->desc.exportable, perm_str);
		if (idmelib_item_has_data(item))
			idme_print_value(item);
		else
			printf("(empty)");
		printf("\n");

		item = idmelib_item_next(item);
	}
}

static int idme_cmd_flags(struct idme *hdr, int argc, char *const argv[])
{
	enum idme_flag_type type = IDME_FLAGS_FOS;
	unsigned long long cur, val, next;
	const char *sub;
	int off = 0;

	if (argc > 0 && !idme_flag_type(argv[0], &type))
		off = 1;

	if (argc <= off) {
		if (idmelib_flags_get(hdr, type, &cur)) {
			printf("idme: cannot read %s\n",
			       idmelib_flags_var_name(type));
			return CMD_RET_FAILURE;
		}

		idme_print_flags(type, cur);

		return CMD_RET_SUCCESS;
	}

	sub = argv[off];

	if (argc <= off + 1) {
		printf("idme: missing value for '%s'\n", sub);
		return CMD_RET_FAILURE;
	}

	if (idme_flag_value(argv[off + 1], &val)) {
		printf("idme: invalid flag value '%s'\n", argv[off + 1]);
		return CMD_RET_FAILURE;
	}

	if (idmelib_flags_get(hdr, type, &cur))
		cur = 0;

	if (!strcmp(sub, "set"))
		next = val;
	else if (!strcmp(sub, "add"))
		next = cur | val;
	else if (!strcmp(sub, "remove"))
		next = cur & ~val;
	else {
		printf("idme: unknown flags subcommand '%s'\n", sub);
		return CMD_RET_FAILURE;
	}

	if (idmelib_flags_set(hdr, type, next)) {
		printf("idme: cannot update %s\n",
		       idmelib_flags_var_name(type));
		return CMD_RET_FAILURE;
	}

	if (idme_access(hdr, true))
		return CMD_RET_FAILURE;

	idme_print_flags(type, next);

	return CMD_RET_SUCCESS;
}

static int do_idme(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct idme_item *item;
	struct idme *hdr;
	int ret = CMD_RET_SUCCESS;
	const char *op;

	if (argc < 2)
		return CMD_RET_USAGE;

	op = argv[1];

	hdr = idme_load();
	if (!hdr)
		return CMD_RET_FAILURE;

	if (!strcmp(op, "list") && argc == 2) {
		idme_list(hdr);
		goto out;
	}

	if (!strcmp(op, "read") && argc == 3) {
		item = idmelib_get_item(hdr, argv[2]);
		if (!item) {
			printf("idme: no entry '%s'\n", argv[2]);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		if (idmelib_item_has_data(item))
			idme_print_value(item);
		else
			printf("(empty)");
		printf("\n");
		goto out;
	}

	if (!strcmp(op, "get") && (argc == 3 || argc == 4)) {
		char value[256];

		if (idmelib_get_var(hdr, argv[2], value, sizeof(value))) {
			printf("idme: no entry '%s'\n", argv[2]);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		if (env_set(argc == 4 ? argv[3] : argv[2], value))
			ret = CMD_RET_FAILURE;
		goto out;
	}

	if (!strcmp(op, "write") && argc == 4) {
		size_t len = strlen(argv[3]);

		if (idmelib_set_var(hdr, argv[2], argv[3])) {
			printf("idme: no entry '%s'\n", argv[2]);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		item = idmelib_get_item(hdr, argv[2]);
		if (item && len > item->desc.size)
			printf("idme: '%s' size is %u bytes, %zu given, truncated\n",
			       argv[2], item->desc.size, len);

		if (idme_access(hdr, true)) {
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("idme: set '%s' to '%.*s'\n", argv[2],
		       item ? (int)item->desc.size : (int)len, argv[3]);
		goto out;
	}

	if (!strcmp(op, "hexdump") && argc == 3) {
		item = idmelib_get_item(hdr, argv[2]);
		if (!item) {
			printf("idme: no entry '%s'\n", argv[2]);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("%s (%u bytes):\n", argv[2], item->desc.size);
		idme_hexdump(item);
		goto out;
	}

	if (!strcmp(op, "flags")) {
		ret = idme_cmd_flags(hdr, argc - 2, argv + 2);
		goto out;
	}

	if (!strcmp(op, "board") && argc == 2) {
		struct idme_board_info info;

		if (idmelib_get_board_info(hdr, &info)) {
			printf("idme: cannot read board_id\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("board_id:   %s\n", info.raw);
		printf("board_type: 0x%04x\n", info.board_type);
		printf("board_rev:  0x%02x\n", info.board_rev);
		printf("wan:        %s\n", info.has_wan ? "yes" : "no");
		goto out;
	}

	if (!strcmp(op, "bootmode") && argc == 2) {
		int mode = idmelib_get_bootmode(hdr);

		if (mode < 0) {
			printf("idme: cannot read bootmode\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("%d (%s)\n", mode, idmelib_bootmode_to_str(mode));
		goto out;
	}

	if (!strcmp(op, "bootcount") && argc == 2) {
		unsigned int count;

		if (idmelib_get_bootcount(hdr, &count)) {
			printf("idme: cannot read bootcount\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("%u\n", count);
		goto out;
	}

	if (!strcmp(op, "bootcount") && argc == 3 &&
	    !strcmp(argv[2], "reset")) {
		if (idmelib_set_var(hdr, "bootcount", "0")) {
			printf("idme: cannot reset bootcount\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}

		if (idme_access(hdr, true)) {
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("idme: bootcount reset to 0\n");
		goto out;
	}

	if (!strcmp(op, "version") && argc == 2) {
		char ver[IDME_VERSION_LEN + 1];

		if (idmelib_get_version(hdr, ver, sizeof(ver))) {
			printf("idme: cannot read version\n");
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("%s\n", ver);
		goto out;
	}

	if (!strcmp(op, "version") && argc == 3) {
		if (idmelib_set_version(hdr, argv[2])) {
			printf("idme: invalid version '%s' (max %d chars)\n",
			       argv[2], IDME_VERSION_LEN);
			ret = CMD_RET_FAILURE;
			goto out;
		}

		if (idme_access(hdr, true)) {
			ret = CMD_RET_FAILURE;
			goto out;
		}

		printf("idme: version set to '%s'\n", argv[2]);
		goto out;
	}

	ret = CMD_RET_USAGE;
out:
	free(hdr);

	return ret;
}

U_BOOT_CMD(idme, 5, 0, do_idme,
	   "read and write Amazon idme data",
	   "list\n"
	   "    List all IDME entries: size, permissions and value.\n"
	   "idme read <name>\n"
	   "    Print one entry.\n"
	   "idme get <name> [var]\n"
	   "    Read an entry into the environment, into <name> by default.\n"
	   "idme write <name> <value>\n"
	   "    Update the contents of an entry. Entries cannot be created\n"
	   "    or resized.\n"
	   "idme hexdump <name>\n"
	   "    Dump an entry's raw bytes.\n"
	   "idme flags [fos|dev|usr]\n"
	   "    Show value of IDME fos/dev/usr flags.\n"
	   "idme flags [fos|dev|usr] set|add|remove <value>\n"
	   "    Update value of IDME fos/dev/usr flags.\n"
	   "    <value> is a number or one flag name.\n"
	   "idme board\n"
	   "    Show board_id split into its fields.\n"
	   "idme bootmode\n"
	   "    Show the boot mode.\n"
	   "idme bootcount [reset]\n"
	   "    Show the boot count, or set it back to zero.\n"
	   "idme version [<new>]\n"
	   "    Show the idme version, or set it.\n");
