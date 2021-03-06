/*
 * Copyright 2012-15 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: AMD
 *
 */

#ifndef __DAL_BIOS_PARSER_HELPER_H__
#define __DAL_BIOS_PARSER_HELPER_H__

#if defined(CONFIG_DRM_AMD_DAL_DCE8_0)
#include "dce80/bios_parser_helper_dce80.h"
#endif

#if defined(CONFIG_DRM_AMD_DAL_DCE11_0) || defined(CONFIG_DRM_AMD_DAL_DCE10_0)
#include "dce110/bios_parser_helper_dce110.h"
#endif

#if defined(CONFIG_DRM_AMD_DAL_DCE11_2)
#include "dce112/bios_parser_helper_dce112.h"
#endif

struct bios_parser;

struct bios_parser_helper {
	bool (*is_accelerated_mode)(
		struct dc_context *ctx);
};

bool dal_bios_parser_init_bios_helper(
	struct bios_parser *bp,
	enum dce_version ver);


uint8_t *get_image(struct dc_bios *bp, uint32_t offset,
	uint32_t size);

#define GET_IMAGE(type, offset) ((type *) get_image(&bp->base, offset, sizeof(type)))





#endif
