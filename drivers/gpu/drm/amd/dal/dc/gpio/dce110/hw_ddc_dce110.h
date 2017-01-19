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

#ifndef __DAL_HW_DDC_DCE110_H__
#define __DAL_HW_DDC_DCE110_H__

struct hw_ddc_dce110_addr {
	uint32_t dc_i2c_ddc_setup;
};

struct hw_ddc_dce110 {
	struct hw_ddc base;
	struct hw_ddc_dce110_addr addr;
};

#define DDC_DCE110_FROM_BASE(ddc_base) \
	container_of((HW_DDC_FROM_BASE(ddc_base)), struct hw_ddc_dce110, base)

struct hw_gpio_pin *dal_hw_ddc_dce110_create(
	struct dc_context *ctx,
	enum gpio_id id,
	uint32_t en);

#endif
