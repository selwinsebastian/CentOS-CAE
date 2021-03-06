/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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


#ifndef MOD_COLOR_H_
#define MOD_COLOR_H_

#include "dm_services.h"

struct mod_color {
	int dummy;
};

struct color_space_coordinates {
	unsigned int redX;
	unsigned int redY;
	unsigned int greenX;
	unsigned int greenY;
	unsigned int blueX;
	unsigned int blueY;
	unsigned int whiteX;
	unsigned int whiteY;
};

struct gamut_space_coordinates {
	unsigned int redX;
	unsigned int redY;
	unsigned int greenX;
	unsigned int greenY;
	unsigned int blueX;
	unsigned int blueY;
};

struct gamut_space_entry {
	unsigned int index;
	unsigned int redX;
	unsigned int redY;
	unsigned int greenX;
	unsigned int greenY;
	unsigned int blueX;
	unsigned int blueY;

	int a0;
	int a1;
	int a2;
	int a3;
	int gamma;
};

struct white_point_coodinates {
	unsigned int whiteX;
	unsigned int whiteY;
};

struct white_point_coodinates_entry {
	unsigned int index;
	unsigned int whiteX;
	unsigned int whiteY;
};

struct color_range {
	int current;
	int min;
	int max;
};

struct mod_color *mod_color_create(struct dc *dc);

void mod_color_destroy(struct mod_color *mod_color);

bool mod_color_add_sink(struct mod_color *mod_color,
		const struct dc_sink *sink);

bool mod_color_remove_sink(struct mod_color *mod_color,
		const struct dc_sink *sink);

bool mod_color_update_gamut_to_stream(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams);

bool mod_color_set_white_point(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		struct white_point_coodinates *white_point);

bool mod_color_adjust_source_gamut(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		struct gamut_space_coordinates *input_gamut_coordinates,
		struct white_point_coodinates *input_white_point_coordinates);

bool mod_color_adjust_destination_gamut(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		struct gamut_space_coordinates *input_gamut_coordinates,
		struct white_point_coodinates *input_white_point_coordinates);

bool mod_color_get_user_enable(struct mod_color *mod_color,
		const struct dc_sink *sink,
		bool *user_enable);

bool mod_color_set_user_enable(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		bool user_enable);

bool mod_color_get_custom_color_temperature(struct mod_color *mod_color,
		const struct dc_sink *sink,
		int *color_temperature);

bool mod_color_set_custom_color_temperature(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		int color_temperature);

bool mod_color_get_color_saturation(struct mod_color *mod_color,
		const struct dc_sink *sink,
		struct color_range *color_saturation);

bool mod_color_get_color_contrast(struct mod_color *mod_color,
		const struct dc_sink *sink,
		struct color_range *color_contrast);

bool mod_color_get_color_brightness(struct mod_color *mod_color,
		const struct dc_sink *sink,
		struct color_range *color_brightness);

bool mod_color_get_color_hue(struct mod_color *mod_color,
		const struct dc_sink *sink,
		struct color_range *color_hue);

bool mod_color_get_source_gamut(struct mod_color *mod_color,
		const struct dc_sink *sink,
		struct color_space_coordinates *source_gamut);

bool mod_color_notify_mode_change(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams);

bool mod_color_set_brightness(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		int brightness_value);

bool mod_color_set_contrast(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		int contrast_value);

bool mod_color_set_hue(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		int hue_value);

bool mod_color_set_saturation(struct mod_color *mod_color,
		const struct dc_stream **streams, int num_streams,
		int saturation_value);

bool mod_color_set_preferred_quantization_range(struct mod_color *mod_color,
		const struct dc_sink *sink,
		enum dc_quantization_range quantization_range);

bool mod_color_get_preferred_quantization_range(struct mod_color *mod_color,
		const struct dc_sink *sink,
		enum dc_quantization_range *quantization_range);

#endif /* MOD_COLOR_H_ */
