/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#ifndef __TPS65132_H_
#define __TPS65132_H_

#define TPS65132_REG_VPOS		0x00
#define TPS65132_REG_VNEG		0x01
#define TPS65132_REG_APPS_DISP_DISN	0x03

#define TPS65132_VOUT_MASK		0x1f
#define TPS65132_VOUT_VMIN		4000000
#define TPS65132_VOUT_VMAX		6000000
#define TPS65132_VOUT_STEP		100000

#define TPS65132_APPS_DIS_VNEG		BIT(0)
#define TPS65132_APPS_DIS_VPOS		BIT(1)
#define TPS65132_APPS_APPS		BIT(6)

/* Time for the chip to come out of shutdown once an output is enabled */
#define TPS65132_ENABLE_TIME_US		500

#define TPS65132_OUT_DRIVER		"tps65132_regulator"

#endif /* __TPS65132_H_ */
