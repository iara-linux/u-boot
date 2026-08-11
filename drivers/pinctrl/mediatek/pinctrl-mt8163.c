// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 MediaTek Inc.
 * Author: Hongzhou.Yang <hongzhou.yang@mediatek.com>
 *         Yingjoe Chen <yingjoe.chen@mediatek.com>
 * Copyright (c) 2025-2026 Ben Grisdale <bengris32@protonmail.ch>
 */

#include <dm.h>

#include "pinctrl-mtk-common.h"

#define PIN_FIELD(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit, _x_bits)	\
	PIN_FIELD_CALC(_s_pin, _e_pin, _s_addr, _x_addrs, _s_bit,	\
			   _x_bits, 16, false)

static const struct mtk_pin_field_calc mt8163_pin_mode_range[] = {
	PIN_FIELD_CALC(0, 154, 0x600, 0x10, 0, 3, 15, false),
};

static const struct mtk_pin_field_calc mt8163_pin_dir_range[] = {
	PIN_FIELD(0, 154, 0x0, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_di_range[] = {
	PIN_FIELD(0, 154, 0x500, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_do_range[] = {
	PIN_FIELD(0, 154, 0x400, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_pullen_range[] = {
	PIN_FIELD(0, 154, 0x100, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_pullsel_range[] = {
	PIN_FIELD(0, 154, 0x200, 0x10, 0, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_ies_range[] = {
	PIN_FIELD(0, 9, 0x900, 0x10, 0, 1),
	PIN_FIELD(10, 13, 0x900, 0x10, 1, 1),
	PIN_FIELD(14, 28, 0x900, 0x10, 2, 1),
	PIN_FIELD(29, 32, 0x900, 0x10, 3, 1),
	PIN_FIELD(33, 33, 0x910, 0x10, 11, 1),
	PIN_FIELD(34, 38, 0x900, 0x10, 10, 1),
	PIN_FIELD(39, 42, 0x900, 0x10, 11, 1),
	PIN_FIELD(43, 45, 0x900, 0x10, 12, 1),
	PIN_FIELD(46, 49, 0x900, 0x10, 13, 1),
	PIN_FIELD(50, 52, 0x910, 0x10, 10, 1),
	PIN_FIELD(53, 56, 0x900, 0x10, 14, 1),
	PIN_FIELD(57, 58, 0x910, 0x10, 0, 1),
	PIN_FIELD(59, 65, 0x910, 0x10, 2, 1),
	PIN_FIELD(66, 71, 0x910, 0x10, 3, 1),
	PIN_FIELD(72, 74, 0x910, 0x10, 4, 1),
	PIN_FIELD(75, 76, 0x900, 0x10, 15, 1),
	PIN_FIELD(77, 78, 0x910, 0x10, 1, 1),
	PIN_FIELD(79, 82, 0x910, 0x10, 5, 1),
	PIN_FIELD(83, 84, 0x910, 0x10, 6, 1),
	PIN_FIELD(87, 87, 0xcf0, 0x10, 4, 1),
	PIN_FIELD(117, 120, 0x910, 0x10, 7, 1),
	PIN_FIELD(121, 121, 0xc80, 0x10, 4, 1),
	PIN_FIELD(122, 122, 0xc70, 0x10, 4, 1),
	PIN_FIELD(123, 126, 0xc90, 0x10, 4, 1),
	PIN_FIELD(127, 131, 0xc20, 0x10, 4, 1),
	PIN_FIELD(132, 132, 0xc10, 0x10, 4, 1),
	PIN_FIELD(133, 133, 0xc00, 0x10, 4, 1),
	PIN_FIELD(134, 137, 0xc20, 0x10, 4, 1),
	PIN_FIELD(138, 141, 0x910, 0x10, 9, 1),
	PIN_FIELD(142, 142, 0x900, 0x10, 13, 1),
	PIN_FIELD(143, 147, 0xf20, 0x10, 4, 1),
	PIN_FIELD(148, 148, 0xf10, 0x10, 4, 1),
	PIN_FIELD(149, 149, 0xf00, 0x10, 4, 1),
	PIN_FIELD(150, 153, 0xf20, 0x10, 4, 1),
	PIN_FIELD(154, 154, 0xf20, 0x10, 7, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_smt_range[] = {
	PIN_FIELD(0, 9, 0x920, 0x10, 0, 1),
	PIN_FIELD(10, 13, 0x920, 0x10, 1, 1),
	PIN_FIELD(14, 28, 0x920, 0x10, 2, 1),
	PIN_FIELD(29, 32, 0x920, 0x10, 3, 1),
	PIN_FIELD(33, 33, 0x930, 0x10, 11, 1),
	PIN_FIELD(34, 38, 0x920, 0x10, 10, 1),
	PIN_FIELD(39, 42, 0x920, 0x10, 11, 1),
	PIN_FIELD(43, 45, 0x920, 0x10, 12, 1),
	PIN_FIELD(46, 49, 0x920, 0x10, 13, 1),
	PIN_FIELD(50, 52, 0x930, 0x10, 10, 1),
	PIN_FIELD(53, 56, 0x920, 0x10, 14, 1),
	PIN_FIELD(57, 58, 0x930, 0x10, 0, 1),
	PIN_FIELD(59, 65, 0x930, 0x10, 2, 1),
	PIN_FIELD(66, 71, 0x930, 0x10, 3, 1),
	PIN_FIELD(72, 74, 0x930, 0x10, 4, 1),
	PIN_FIELD(75, 76, 0x920, 0x10, 15, 1),
	PIN_FIELD(77, 78, 0x930, 0x10, 1, 1),
	PIN_FIELD(79, 82, 0x930, 0x10, 5, 1),
	PIN_FIELD(83, 84, 0x930, 0x10, 6, 1),
	PIN_FIELD(85, 85, 0xce0, 0x10, 11, 1),
	PIN_FIELD(86, 86, 0xcd0, 0x10, 11, 1),
	PIN_FIELD(87, 87, 0xd00, 0x10, 3, 1),
	PIN_FIELD(88, 88, 0xd00, 0x10, 7, 1),
	PIN_FIELD(89, 89, 0xd00, 0x10, 11, 1),
	PIN_FIELD(90, 90, 0xd00, 0x10, 15, 1),
	PIN_FIELD(117, 120, 0x930, 0x10, 7, 1),
	PIN_FIELD(121, 121, 0xc80, 0x10, 11, 1),
	PIN_FIELD(122, 122, 0xc70, 0x10, 11, 1),
	PIN_FIELD(123, 123, 0xca0, 0x10, 3, 1),
	PIN_FIELD(124, 124, 0xca0, 0x10, 7, 1),
	PIN_FIELD(125, 125, 0xca0, 0x10, 11, 1),
	PIN_FIELD(126, 126, 0xca0, 0x10, 15, 1),
	PIN_FIELD(127, 127, 0xc40, 0x10, 15, 1),
	PIN_FIELD(128, 128, 0xc40, 0x10, 11, 1),
	PIN_FIELD(129, 129, 0xc40, 0x10, 7, 1),
	PIN_FIELD(130, 130, 0xc40, 0x10, 3, 1),
	PIN_FIELD(131, 131, 0xc50, 0x10, 3, 1),
	PIN_FIELD(132, 132, 0xc10, 0x10, 11, 1),
	PIN_FIELD(133, 133, 0xc00, 0x10, 11, 1),
	PIN_FIELD(134, 134, 0xc30, 0x10, 15, 1),
	PIN_FIELD(135, 135, 0xc30, 0x10, 11, 1),
	PIN_FIELD(136, 136, 0xc30, 0x10, 7, 1),
	PIN_FIELD(137, 137, 0xc30, 0x10, 3, 1),
	PIN_FIELD(138, 141, 0x930, 0x10, 9, 1),
	PIN_FIELD(142, 142, 0x920, 0x10, 13, 1),
	PIN_FIELD(143, 143, 0xf40, 0x10, 15, 1),
	PIN_FIELD(144, 144, 0xf40, 0x10, 11, 1),
	PIN_FIELD(145, 145, 0xf40, 0x10, 7, 1),
	PIN_FIELD(146, 146, 0xf40, 0x10, 3, 1),
	PIN_FIELD(147, 147, 0xf50, 0x10, 3, 1),
	PIN_FIELD(148, 148, 0xf10, 0x10, 11, 1),
	PIN_FIELD(149, 149, 0xf00, 0x10, 11, 1),
	PIN_FIELD(150, 150, 0xf30, 0x10, 15, 1),
	PIN_FIELD(151, 151, 0xf30, 0x10, 11, 1),
	PIN_FIELD(152, 152, 0xf30, 0x10, 7, 1),
	PIN_FIELD(153, 153, 0xf30, 0x10, 3, 1),
	PIN_FIELD(154, 154, 0xf50, 0x10, 3, 1),
};

static const struct mtk_pin_field_calc mt8163_pin_drv_range[] = {
	PIN_FIELD(0, 6, 0xb00, 0x10, 0, 4),
	PIN_FIELD(7, 9, 0xb00, 0x10, 12, 4),
	PIN_FIELD(10, 13, 0xb00, 0x10, 8, 4),
	PIN_FIELD(14, 17, 0xb10, 0x10, 4, 4),
	PIN_FIELD(18, 21, 0xb10, 0x10, 8, 4),
	PIN_FIELD(22, 26, 0xb20, 0x10, 0, 4),
	PIN_FIELD(27, 32, 0xb20, 0x10, 4, 4),
	PIN_FIELD(33, 33, 0xb30, 0x10, 4, 4),
	PIN_FIELD(34, 38, 0xb30, 0x10, 8, 4),
	PIN_FIELD(39, 42, 0xb30, 0x10, 12, 4),
	PIN_FIELD(43, 45, 0xb40, 0x10, 12, 4),
	PIN_FIELD(46, 49, 0xb50, 0x10, 0, 4),
	PIN_FIELD(50, 52, 0xb70, 0x10, 0, 4),
	PIN_FIELD(53, 56, 0xb50, 0x10, 12, 4),
	PIN_FIELD(59, 59, 0xb40, 0x10, 4, 4),
	PIN_FIELD(60, 62, 0xb40, 0x10, 0, 4),
	PIN_FIELD(63, 65, 0xb40, 0x10, 4, 4),
	PIN_FIELD(66, 71, 0xb40, 0x10, 8, 4),
	PIN_FIELD(72, 74, 0xb50, 0x10, 4, 4),
	PIN_FIELD(79, 84, 0xb50, 0x10, 8, 4),
	PIN_FIELD(85, 85, 0xce0, 0x10, 0, 4),
	PIN_FIELD(86, 86, 0xcd0, 0x10, 0, 4),
	PIN_FIELD(87, 90, 0xcf0, 0x10, 0, 4),
	PIN_FIELD(117, 120, 0xb60, 0x10, 12, 4),
	PIN_FIELD(121, 121, 0xc80, 0x10, 0, 4),
	PIN_FIELD(122, 122, 0xc70, 0x10, 0, 4),
	PIN_FIELD(123, 126, 0xc90, 0x10, 0, 4),
	PIN_FIELD(127, 131, 0xc20, 0x10, 0, 4),
	PIN_FIELD(132, 132, 0xc10, 0x10, 0, 4),
	PIN_FIELD(133, 133, 0xc00, 0x10, 0, 4),
	PIN_FIELD(134, 137, 0xc20, 0x10, 0, 4),
	PIN_FIELD(142, 142, 0xb50, 0x10, 0, 4),
	PIN_FIELD(143, 147, 0xf20, 0x10, 0, 4),
	PIN_FIELD(148, 148, 0xf10, 0x10, 0, 4),
	PIN_FIELD(149, 149, 0xf00, 0x10, 0, 4),
	PIN_FIELD(150, 154, 0xf20, 0x10, 0, 4),
};

static const struct mtk_pin_field_calc mt8163_pin_pupd_range[] = {
	PIN_FIELD(33, 35, 0xd90, 0x10, 2, 4),	/* KPROW0 */
	PIN_FIELD(36, 38, 0xda0, 0x10, 2, 4),	/* KPCOL0 */
	PIN_FIELD(46, 49, 0xdb0, 0x10, 2, 4),	/* EINT14 */
	PIN_FIELD(85, 85, 0xce0, 0x10, 8, 1),	/* MSDC2_CMD */
	PIN_FIELD(86, 86, 0xcd0, 0x10, 8, 1),	/* MSDC2_CLK */
	PIN_FIELD(87, 90, 0xd00, 0x10, 0, 4),	/* MSDC2_DAT0 */
	PIN_FIELD(121, 121, 0xc80, 0x10, 8, 1),	/* MSDC1_CMD */
	PIN_FIELD(122, 122, 0xc70, 0x10, 8, 1),	/* MSDC1_CLK */
	PIN_FIELD(123, 126, 0xca0, 0x10, 0, 4),	/* MSDC1_DAT0 */
	PIN_FIELD(127, 127, 0xc40, 0x10, 12, 1),	/* MSDC0_DAT7 */
	PIN_FIELD(128, 128, 0xc40, 0x10, 8, 1),	/* MSDC0_DAT6 */
	PIN_FIELD(129, 129, 0xc40, 0x10, 4, 1),	/* MSDC0_DAT5 */
	PIN_FIELD(130, 130, 0xc40, 0x10, 0, 1),	/* MSDC0_DAT4 */
	PIN_FIELD(131, 131, 0xc50, 0x10, 0, 1),	/* MSDC0_RSTB */
	PIN_FIELD(132, 132, 0xc10, 0x10, 8, 1),	/* MSDC0_CMD */
	PIN_FIELD(133, 133, 0xc00, 0x10, 8, 1),	/* MSDC0_CLK */
	PIN_FIELD(134, 134, 0xc30, 0x10, 12, 1),	/* MSDC0_DAT3 */
	PIN_FIELD(135, 135, 0xc30, 0x10, 8, 1),	/* MSDC0_DAT2 */
	PIN_FIELD(136, 136, 0xc30, 0x10, 4, 1),	/* MSDC0_DAT1 */
	PIN_FIELD(137, 137, 0xc30, 0x10, 0, 1),	/* MSDC0_DAT0 */
	PIN_FIELD(142, 142, 0xdc0, 0x10, 2, 1),	/* EINT21 */
	PIN_FIELD(143, 143, 0xf40, 0x10, 12, 1),	/* MSDC3_DAT7 */
	PIN_FIELD(144, 144, 0xf40, 0x10, 8, 1),	/* MSDC3_DAT6 */
	PIN_FIELD(145, 145, 0xf40, 0x10, 4, 1),	/* MSDC3_DAT5 */
	PIN_FIELD(146, 146, 0xf40, 0x10, 0, 1),	/* MSDC3_DAT4 */
	PIN_FIELD(147, 147, 0xf50, 0x10, 0, 1),	/* MSDC3_RSTB */
	PIN_FIELD(148, 148, 0xf10, 0x10, 8, 1),	/* MSDC3_CMD */
	PIN_FIELD(149, 149, 0xf00, 0x10, 8, 1),	/* MSDC3_CLK */
	PIN_FIELD(150, 150, 0xf30, 0x10, 12, 1),	/* MSDC3_DAT3 */
	PIN_FIELD(151, 151, 0xf30, 0x10, 8, 1),	/* MSDC3_DAT2 */
	PIN_FIELD(152, 152, 0xf30, 0x10, 4, 1),	/* MSDC3_DAT1 */
	PIN_FIELD(153, 153, 0xf30, 0x10, 0, 1),	/* MSDC3_DAT0 */
	PIN_FIELD(154, 154, 0xf50, 0x10, 4, 1),	/* MSDC3_DSL */
};

static const struct mtk_pin_field_calc mt8163_pin_r0_range[] = {
	PIN_FIELD(33, 35, 0xd90, 0x10, 1, 4),	/* KPROW0 */
	PIN_FIELD(36, 38, 0xda0, 0x10, 1, 4),	/* KPCOL0 */
	PIN_FIELD(46, 49, 0xdb0, 0x10, 1, 4),	/* EINT14 */
	PIN_FIELD(85, 85, 0xce0, 0x10, 9, 1),	/* MSDC2_CMD */
	PIN_FIELD(86, 86, 0xcd0, 0x10, 9, 1),	/* MSDC2_CLK */
	PIN_FIELD(87, 90, 0xd00, 0x10, 1, 4),	/* MSDC2_DAT0 */
	PIN_FIELD(121, 121, 0xc80, 0x10, 9, 1),	/* MSDC1_CMD */
	PIN_FIELD(122, 122, 0xc70, 0x10, 9, 1),	/* MSDC1_CLK */
	PIN_FIELD(123, 126, 0xca0, 0x10, 1, 4),	/* MSDC1_DAT0 */
	PIN_FIELD(127, 127, 0xc40, 0x10, 13, 1),	/* MSDC0_DAT7 */
	PIN_FIELD(128, 128, 0xc40, 0x10, 9, 1),	/* MSDC0_DAT6 */
	PIN_FIELD(129, 129, 0xc40, 0x10, 5, 1),	/* MSDC0_DAT5 */
	PIN_FIELD(130, 130, 0xc40, 0x10, 1, 1),	/* MSDC0_DAT4 */
	PIN_FIELD(131, 131, 0xc50, 0x10, 1, 1),	/* MSDC0_RSTB */
	PIN_FIELD(132, 132, 0xc10, 0x10, 9, 1),	/* MSDC0_CMD */
	PIN_FIELD(133, 133, 0xc00, 0x10, 9, 1),	/* MSDC0_CLK */
	PIN_FIELD(134, 134, 0xc30, 0x10, 13, 1),	/* MSDC0_DAT3 */
	PIN_FIELD(135, 135, 0xc30, 0x10, 9, 1),	/* MSDC0_DAT2 */
	PIN_FIELD(136, 136, 0xc30, 0x10, 5, 1),	/* MSDC0_DAT1 */
	PIN_FIELD(137, 137, 0xc30, 0x10, 1, 1),	/* MSDC0_DAT0 */
	PIN_FIELD(142, 142, 0xdc0, 0x10, 1, 1),	/* EINT21 */
	PIN_FIELD(143, 143, 0xf40, 0x10, 13, 1),	/* MSDC3_DAT7 */
	PIN_FIELD(144, 144, 0xf40, 0x10, 9, 1),	/* MSDC3_DAT6 */
	PIN_FIELD(145, 145, 0xf40, 0x10, 5, 1),	/* MSDC3_DAT5 */
	PIN_FIELD(146, 146, 0xf40, 0x10, 1, 1),	/* MSDC3_DAT4 */
	PIN_FIELD(147, 147, 0xf50, 0x10, 1, 1),	/* MSDC3_RSTB */
	PIN_FIELD(148, 148, 0xf10, 0x10, 9, 1),	/* MSDC3_CMD */
	PIN_FIELD(149, 149, 0xf00, 0x10, 9, 1),	/* MSDC3_CLK */
	PIN_FIELD(150, 150, 0xf30, 0x10, 13, 1),	/* MSDC3_DAT3 */
	PIN_FIELD(151, 151, 0xf30, 0x10, 9, 1),	/* MSDC3_DAT2 */
	PIN_FIELD(152, 152, 0xf30, 0x10, 5, 1),	/* MSDC3_DAT1 */
	PIN_FIELD(153, 153, 0xf30, 0x10, 1, 1),	/* MSDC3_DAT0 */
	PIN_FIELD(154, 154, 0xf50, 0x10, 5, 1),	/* MSDC3_DSL */
};

static const struct mtk_pin_field_calc mt8163_pin_r1_range[] = {
	PIN_FIELD(33, 35, 0xd90, 0x10, 0, 4),	/* KPROW0 */
	PIN_FIELD(36, 38, 0xda0, 0x10, 0, 4),	/* KPCOL0 */
	PIN_FIELD(46, 49, 0xdb0, 0x10, 0, 4),	/* EINT14 */
	PIN_FIELD(85, 85, 0xce0, 0x10, 10, 1),	/* MSDC2_CMD */
	PIN_FIELD(86, 86, 0xcd0, 0x10, 10, 1),	/* MSDC2_CLK */
	PIN_FIELD(87, 90, 0xd00, 0x10, 2, 4),	/* MSDC2_DAT0 */
	PIN_FIELD(121, 121, 0xc80, 0x10, 10, 1),	/* MSDC1_CMD */
	PIN_FIELD(122, 122, 0xc70, 0x10, 10, 1),	/* MSDC1_CLK */
	PIN_FIELD(123, 126, 0xca0, 0x10, 2, 4),	/* MSDC1_DAT0 */
	PIN_FIELD(127, 127, 0xc40, 0x10, 14, 1),	/* MSDC0_DAT7 */
	PIN_FIELD(128, 128, 0xc40, 0x10, 10, 1),	/* MSDC0_DAT6 */
	PIN_FIELD(129, 129, 0xc40, 0x10, 6, 1),	/* MSDC0_DAT5 */
	PIN_FIELD(130, 130, 0xc40, 0x10, 2, 1),	/* MSDC0_DAT4 */
	PIN_FIELD(131, 131, 0xc50, 0x10, 2, 1),	/* MSDC0_RSTB */
	PIN_FIELD(132, 132, 0xc10, 0x10, 10, 1),	/* MSDC0_CMD */
	PIN_FIELD(133, 133, 0xc00, 0x10, 10, 1),	/* MSDC0_CLK */
	PIN_FIELD(134, 134, 0xc30, 0x10, 14, 1),	/* MSDC0_DAT3 */
	PIN_FIELD(135, 135, 0xc30, 0x10, 10, 1),	/* MSDC0_DAT2 */
	PIN_FIELD(136, 136, 0xc30, 0x10, 6, 1),	/* MSDC0_DAT1 */
	PIN_FIELD(137, 137, 0xc30, 0x10, 2, 1),	/* MSDC0_DAT0 */
	PIN_FIELD(142, 142, 0xdc0, 0x10, 0, 1),	/* EINT21 */
	PIN_FIELD(143, 143, 0xf40, 0x10, 10, 1),	/* MSDC3_DAT7 */
	PIN_FIELD(144, 144, 0xf40, 0x10, 10, 1),	/* MSDC3_DAT6 */
	PIN_FIELD(145, 145, 0xf40, 0x10, 6, 1),	/* MSDC3_DAT5 */
	PIN_FIELD(146, 146, 0xf40, 0x10, 2, 1),	/* MSDC3_DAT4 */
	PIN_FIELD(147, 147, 0xf50, 0x10, 2, 1),	/* MSDC3_RSTB */
	PIN_FIELD(148, 148, 0xf10, 0x10, 10, 1),	/* MSDC3_CMD */
	PIN_FIELD(149, 149, 0xf00, 0x10, 10, 1),	/* MSDC3_CLK */
	PIN_FIELD(150, 150, 0xf30, 0x10, 14, 1),	/* MSDC3_DAT3 */
	PIN_FIELD(151, 151, 0xf30, 0x10, 10, 1),	/* MSDC3_DAT2 */
	PIN_FIELD(152, 152, 0xf30, 0x10, 6, 1),	/* MSDC3_DAT1 */
	PIN_FIELD(153, 153, 0xf30, 0x10, 2, 1),	/* MSDC3_DAT0 */
	PIN_FIELD(154, 154, 0xf50, 0x10, 6, 1),	/* MSDC3_DSL */
};

static const struct mtk_pin_reg_calc mt8163_reg_cals[PINCTRL_PIN_REG_MAX] = {
	[PINCTRL_PIN_REG_MODE] = MTK_RANGE(mt8163_pin_mode_range),
	[PINCTRL_PIN_REG_DIR] = MTK_RANGE(mt8163_pin_dir_range),
	[PINCTRL_PIN_REG_DI] = MTK_RANGE(mt8163_pin_di_range),
	[PINCTRL_PIN_REG_DO] = MTK_RANGE(mt8163_pin_do_range),
	[PINCTRL_PIN_REG_IES] = MTK_RANGE(mt8163_pin_ies_range),
	[PINCTRL_PIN_REG_SMT] = MTK_RANGE(mt8163_pin_smt_range),
	[PINCTRL_PIN_REG_PULLSEL] = MTK_RANGE(mt8163_pin_pullsel_range),
	[PINCTRL_PIN_REG_PULLEN] = MTK_RANGE(mt8163_pin_pullen_range),
	[PINCTRL_PIN_REG_DRV] = MTK_RANGE(mt8163_pin_drv_range),
	[PINCTRL_PIN_REG_PUPD] = MTK_RANGE(mt8163_pin_pupd_range),
	[PINCTRL_PIN_REG_R0] = MTK_RANGE(mt8163_pin_r0_range),
	[PINCTRL_PIN_REG_R1] = MTK_RANGE(mt8163_pin_r1_range),
};

static const struct mtk_pin_desc mt8163_pins[] = {
	MTK_PIN(0, "PWRAP_SPI0_MI", DRV_GRP2),
	MTK_PIN(1, "PWRAP_SPI0_MO", DRV_GRP2),
	MTK_PIN(2, "PWRAP_INT", DRV_GRP2),
	MTK_PIN(3, "PWRAP_SPI0_CK", DRV_GRP2),
	MTK_PIN(4, "PWRAP_SPI0_CSN", DRV_GRP2),
	MTK_PIN(5, "PWRAP_SPI0_CK2", DRV_GRP2),
	MTK_PIN(6, "PWRAP_SPI0_CSN2", DRV_GRP2),
	MTK_PIN(7, "AUD_CLK_MOSI", DRV_GRP2),
	MTK_PIN(8, "AUD_DAT_MISO", DRV_GRP2),
	MTK_PIN(9, "AUD_DAT_MOSI", DRV_GRP2),
	MTK_PIN(10, "RTC32K_CK", DRV_GRP2),
	MTK_PIN(11, "WATCHDOG", DRV_GRP2),
	MTK_PIN(12, "SRCLKENA", DRV_GRP2),
	MTK_PIN(13, "SRCLKENAI", DRV_GRP2),
	MTK_PIN(14, "URXD2", DRV_GRP0),
	MTK_PIN(15, "UTXD2", DRV_GRP0),
	MTK_PIN(16, "URXD3", DRV_GRP0),
	MTK_PIN(17, "UTXD3", DRV_GRP0),
	MTK_PIN(18, "PCM_CLK", DRV_GRP0),
	MTK_PIN(19, "PCM_SYNC", DRV_GRP0),
	MTK_PIN(20, "PCM_RX", DRV_GRP0),
	MTK_PIN(21, "PCM_TX", DRV_GRP0),
	MTK_PIN(22, "EINT0", DRV_GRP0),
	MTK_PIN(23, "EINT1", DRV_GRP0),
	MTK_PIN(24, "EINT2", DRV_GRP0),
	MTK_PIN(25, "EINT3", DRV_GRP0),
	MTK_PIN(26, "EINT4", DRV_GRP0),
	MTK_PIN(27, "EINT5", DRV_GRP0),
	MTK_PIN(28, "EINT6", DRV_GRP0),
	MTK_PIN(29, "EINT7", DRV_GRP0),
	MTK_PIN(30, "EINT8", DRV_GRP0),
	MTK_PIN(31, "EINT9", DRV_GRP0),
	MTK_PIN(32, "EINT10", DRV_GRP0),
	MTK_TYPED_PIN(33, "KPROW0", DRV_GRP2, IO_TYPE_GRP1),
	MTK_TYPED_PIN(34, "KPROW1", DRV_GRP2, IO_TYPE_GRP1),
	MTK_TYPED_PIN(35, "KPROW2", DRV_GRP2, IO_TYPE_GRP1),
	MTK_TYPED_PIN(36, "KPCOL0", DRV_GRP2, IO_TYPE_GRP1),
	MTK_TYPED_PIN(37, "KPCOL1", DRV_GRP2, IO_TYPE_GRP1),
	MTK_TYPED_PIN(38, "KPCOL2", DRV_GRP2, IO_TYPE_GRP1),
	MTK_PIN(39, "JTMS", DRV_GRP2),
	MTK_PIN(40, "JTCK", DRV_GRP2),
	MTK_PIN(41, "JTDI", DRV_GRP2),
	MTK_PIN(42, "JTDO", DRV_GRP2),
	MTK_PIN(43, "EINT11", DRV_GRP0),
	MTK_PIN(44, "EINT12", DRV_GRP0),
	MTK_PIN(45, "EINT13", DRV_GRP0),
	MTK_TYPED_PIN(46, "EINT14", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(47, "EINT15", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(48, "EINT16", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(49, "EINT17", DRV_GRP4, IO_TYPE_GRP1),
	MTK_PIN(50, "EINT18", DRV_GRP2),
	MTK_PIN(51, "EINT19", DRV_GRP2),
	MTK_PIN(52, "EINT20", DRV_GRP2),
	MTK_PIN(53, "SPI_CS", DRV_GRP2),
	MTK_PIN(54, "SPI_CK", DRV_GRP2),
	MTK_PIN(55, "SPI_MI", DRV_GRP2),
	MTK_PIN(56, "SPI_MO", DRV_GRP2),
	MTK_PIN(57, "SDA1", DRV_GRP0),
	MTK_PIN(58, "SCL1", DRV_GRP0),
	MTK_PIN(59, "DISP_PWM", DRV_GRP2),
	MTK_PIN(60, "WB_RSTB", DRV_GRP2),
	MTK_PIN(61, "F2W_DATA", DRV_GRP2),
	MTK_PIN(62, "F2W_CLK", DRV_GRP2),
	MTK_PIN(63, "WB_SCLK", DRV_GRP2),
	MTK_PIN(64, "WB_SDATA", DRV_GRP2),
	MTK_PIN(65, "WB_SEN", DRV_GRP2),
	MTK_PIN(66, "WB_CRTL0", DRV_GRP2),
	MTK_PIN(67, "WB_CRTL1", DRV_GRP2),
	MTK_PIN(68, "WB_CRTL2", DRV_GRP2),
	MTK_PIN(69, "WB_CRTL3", DRV_GRP2),
	MTK_PIN(70, "WB_CRTL4", DRV_GRP2),
	MTK_PIN(71, "WB_CRTL5", DRV_GRP2),
	MTK_PIN(72, "I2S_DATA_IN", DRV_GRP2),
	MTK_PIN(73, "I2S_LRCK", DRV_GRP2),
	MTK_PIN(74, "I2S_BCK", DRV_GRP2),
	MTK_PIN(75, "SDA0", DRV_GRP0),
	MTK_PIN(76, "SCL0", DRV_GRP0),
	MTK_PIN(77, "SDA2", DRV_GRP0),
	MTK_PIN(78, "SCL2", DRV_GRP0),
	MTK_PIN(79, "URXD0", DRV_GRP2),
	MTK_PIN(80, "UTXD0", DRV_GRP2),
	MTK_PIN(81, "URXD1", DRV_GRP2),
	MTK_PIN(82, "UTXD1", DRV_GRP2),
	MTK_PIN(83, "LCM_RST", DRV_GRP2),
	MTK_PIN(84, "DSI_TE", DRV_GRP2),
	MTK_TYPED_PIN(85, "MSDC2_CMD", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(86, "MSDC2_CLK", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(87, "MSDC2_DAT0", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(88, "MSDC2_DAT1", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(89, "MSDC2_DAT2", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(90, "MSDC2_DAT3", DRV_GRP4, IO_TYPE_GRP1),
	MTK_PIN(91, "TDN3", DRV_GRP0),
	MTK_PIN(92, "TDP3", DRV_GRP0),
	MTK_PIN(93, "TDN2", DRV_GRP0),
	MTK_PIN(94, "TDP2", DRV_GRP0),
	MTK_PIN(95, "TCN", DRV_GRP0),
	MTK_PIN(96, "TCP", DRV_GRP0),
	MTK_PIN(97, "TDN1", DRV_GRP0),
	MTK_PIN(98, "TDP1", DRV_GRP0),
	MTK_PIN(99, "TDN0", DRV_GRP0),
	MTK_PIN(100, "TDP0", DRV_GRP0),
	MTK_PIN(101, "RDN0", DRV_GRP0),
	MTK_PIN(102, "RDP0", DRV_GRP0),
	MTK_PIN(103, "RDN1", DRV_GRP0),
	MTK_PIN(104, "RDP1", DRV_GRP0),
	MTK_PIN(105, "RCN", DRV_GRP0),
	MTK_PIN(106, "RCP", DRV_GRP0),
	MTK_PIN(107, "RDN2", DRV_GRP0),
	MTK_PIN(108, "RDP2", DRV_GRP0),
	MTK_PIN(109, "RDN3", DRV_GRP0),
	MTK_PIN(110, "RDP3", DRV_GRP0),
	MTK_PIN(111, "RCN_A", DRV_GRP0),
	MTK_PIN(112, "RCP_A", DRV_GRP0),
	MTK_PIN(113, "RDN1_A", DRV_GRP0),
	MTK_PIN(114, "RDP1_A", DRV_GRP0),
	MTK_PIN(115, "RDN0_A", DRV_GRP0),
	MTK_PIN(116, "RDP0_A", DRV_GRP0),
	MTK_PIN(117, "CMDAT0", DRV_GRP2),
	MTK_PIN(118, "CMDAT1", DRV_GRP2),
	MTK_PIN(119, "CMMCLK", DRV_GRP2),
	MTK_PIN(120, "CMPCLK", DRV_GRP2),
	MTK_TYPED_PIN(121, "MSDC1_CMD", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(122, "MSDC1_CLK", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(123, "MSDC1_DAT0", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(124, "MSDC1_DAT1", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(125, "MSDC1_DAT2", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(126, "MSDC1_DAT3", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(127, "MSDC0_DAT7", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(128, "MSDC0_DAT6", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(129, "MSDC0_DAT5", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(130, "MSDC0_DAT4", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(131, "MSDC0_RSTB", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(132, "MSDC0_CMD", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(133, "MSDC0_CLK", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(134, "MSDC0_DAT3", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(135, "MSDC0_DAT2", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(136, "MSDC0_DAT1", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(137, "MSDC0_DAT0", DRV_GRP4, IO_TYPE_GRP1),
	MTK_PIN(138, "CEC", DRV_GRP0),
	MTK_PIN(139, "HTPLG", DRV_GRP0),
	MTK_PIN(140, "HDMISCK", DRV_GRP0),
	MTK_PIN(141, "HDMISD", DRV_GRP0),
	MTK_TYPED_PIN(142, "EINT21", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(143, "MSDC3_DAT7", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(144, "MSDC3_DAT6", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(145, "MSDC3_DAT5", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(146, "MSDC3_DAT4", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(147, "MSDC3_RSTB", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(148, "MSDC3_CMD", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(149, "MSDC3_CLK", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(150, "MSDC3_DAT3", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(151, "MSDC3_DAT2", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(152, "MSDC3_DAT1", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(153, "MSDC3_DAT0", DRV_GRP4, IO_TYPE_GRP1),
	MTK_TYPED_PIN(154, "MSDC3_DSL", DRV_GRP4, IO_TYPE_GRP1),
};

/* List all groups consisting of these pins dedicated to the enablement of
 * certain hardware block and the corresponding mode for all of the pins.
 * The hardware probably has multiple combinations of these pinouts.
 */

/* MSDC */
static const int mt8163_msdc0_pins[] = { 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, };
static const int mt8163_msdc0_funcs[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, };

static const char *const mt8163_msdc_groups[] = { "msdc0" };

/* I2C */
static const int mt8163_i2c0_sda_scl_pins[]			= { 75, 76, };
static const int mt8163_i2c0_sda_scl_funcs[]		= {  1,  1, };
static const int mt8163_i2c2_sda_scl_pins[]			= { 77, 78, };
static const int mt8163_i2c2_sda_scl_funcs[]		= {  1,  1, };

static const char *const mt8163_i2c_groups[] = { "i2c0_sda_scl_pins", "i2c2_sda_scl_pins" };

/* UART */
static const int mt8163_uart0_0_rxd_txd_pins[]		= { 79, 80, };
static const int mt8163_uart0_0_rxd_txd_funcs[]		= {  1,  1, };

static const char *const mt8163_uart_groups[] = { "uart0_0_rxd_txd" };

/* Display */
static const int mt8163_disp_pwm_pins[]			= { 59, };
static const int mt8163_disp_pwm_funcs[]		= {  1, };
static const int mt8163_dsi_te_pins[]			= { 84, };
static const int mt8163_dsi_te_funcs[]			= {  1, };

static const char *const mt8163_disp_groups[] = {
	"disp_pwm", "dsi_te"
};

/* Groups */
static const struct mtk_group_desc mt8163_groups[] = {
	PINCTRL_PIN_GROUP("uart0_0_rxd_txd", mt8163_uart0_0_rxd_txd),
	PINCTRL_PIN_GROUP("i2c0_sda_scl", mt8163_i2c0_sda_scl),
	PINCTRL_PIN_GROUP("i2c2_sda_scl", mt8163_i2c2_sda_scl),
	PINCTRL_PIN_GROUP("msdc0", mt8163_msdc0),
	PINCTRL_PIN_GROUP("disp_pwm", mt8163_disp_pwm),
	PINCTRL_PIN_GROUP("dsi_te", mt8163_dsi_te),
};

static const struct mtk_function_desc mt8163_functions[] = {
	{"uart", mt8163_uart_groups, ARRAY_SIZE(mt8163_uart_groups)},
	{"msdc", mt8163_msdc_groups, ARRAY_SIZE(mt8163_msdc_groups)},
	{"i2c", mt8163_i2c_groups, ARRAY_SIZE(mt8163_i2c_groups)},
	{"disp", mt8163_disp_groups, ARRAY_SIZE(mt8163_disp_groups)},
};

static const struct mtk_io_type_desc mt8163_io_type_desc[] = {
	[IO_TYPE_DEFAULT] = {
		.name = "default",
		.bias_set = mtk_pinconf_bias_set_pullen_pullsel,
		.drive_set = mtk_pinconf_drive_set_v1,
		.input_enable = mtk_pinconf_input_enable_v1,
	},
	[IO_TYPE_GRP1] = {
		.name = "pupd",
		.bias_set = mtk_pinconf_bias_set_pupd_r1_r0,
		.drive_set = mtk_pinconf_drive_set_v1,
		.input_enable = mtk_pinconf_input_enable_v1,
	},
};

static struct mtk_pinctrl_soc mt8163_data = {
	.name = "mt8163_pinctrl",
	.reg_cal = mt8163_reg_cals,
	.pins = mt8163_pins,
	.npins = ARRAY_SIZE(mt8163_pins),
	.grps = mt8163_groups,
	.ngrps = ARRAY_SIZE(mt8163_groups),
	.funcs = mt8163_functions,
	.nfuncs = ARRAY_SIZE(mt8163_functions),
	.io_type = mt8163_io_type_desc,
	.ntype = ARRAY_SIZE(mt8163_io_type_desc),
	.gpio_mode = 0,
	.rev = MTK_PINCTRL_V1,
};

static int mtk_pinctrl_mt8163_probe(struct udevice *dev)
{
	return mtk_pinctrl_common_probe(dev, &mt8163_data);
}

static const struct udevice_id mt8163_pctrl_match[] = {
	{ .compatible = "mediatek,mt8163-pinctrl" },
	{ },
};

U_BOOT_DRIVER(mt8163_pinctrl) = {
	.name = "mt8163_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = mt8163_pctrl_match,
	.ops = &mtk_pinctrl_ops,
	.bind = mtk_pinctrl_common_bind,
	.probe = mtk_pinctrl_mt8163_probe,
	.priv_auto = sizeof(struct mtk_pinctrl_priv),
};
