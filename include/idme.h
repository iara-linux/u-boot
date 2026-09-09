/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __IDME_H
#define __IDME_H

#include <idmelib.h>

/* idme variables holding MAC addresses */
#define IDME_VAR_MAC_ADDR	"mac_addr"
#define IDME_VAR_BT_MAC_ADDR	"bt_mac_addr"

/* read idme data from storage */
struct idme *idme_load(void);

/* copy idme mac addresses into device tree */
int idme_fdt_fixup_mac_addrs(void *blob);

#endif /* __IDME_H */
