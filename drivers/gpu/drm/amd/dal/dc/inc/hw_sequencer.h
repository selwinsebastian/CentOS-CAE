/*
 * Copyright 2015 Advanced Micro Devices, Inc.
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

#ifndef __DC_HW_SEQUENCER_H__
#define __DC_HW_SEQUENCER_H__
#include "core_types.h"
#include "timing_generator.h"

struct gamma_parameters;

enum pipe_gating_control {
	PIPE_GATING_CONTROL_DISABLE = 0,
	PIPE_GATING_CONTROL_ENABLE,
	PIPE_GATING_CONTROL_INIT
};

enum pipe_lock_control {
	PIPE_LOCK_CONTROL_GRAPHICS = 1 << 0,
	PIPE_LOCK_CONTROL_BLENDER = 1 << 1,
	PIPE_LOCK_CONTROL_SCL = 1 << 2,
	PIPE_LOCK_CONTROL_SURFACE = 1 << 3,
	PIPE_LOCK_CONTROL_MODE = 1 << 4
};

struct hw_sequencer_funcs {

	void (*init_hw)(struct core_dc *dc);

	enum dc_status (*apply_ctx_to_hw)(
			struct core_dc *dc, struct validate_context *context);

	void (*prepare_pipe_for_context)(
			struct core_dc *dc,
			struct pipe_ctx *pipe_ctx,
			struct validate_context *context);

	void (*apply_ctx_for_surface)(
			struct core_dc *dc,
			struct core_surface *surface,
			struct validate_context *context);

	void (*set_plane_config)(
			const struct core_dc *dc,
			struct pipe_ctx *pipe_ctx,
			struct resource_context *res_ctx);

	void (*update_plane_addr)(
		const struct core_dc *dc,
		struct pipe_ctx *pipe_ctx);

	void (*update_pending_status)(
			struct pipe_ctx *pipe_ctx);

	bool (*set_gamma_correction)(
				struct input_pixel_processor *ipp,
				struct output_pixel_processor *opp,
				const struct core_gamma *ramp,
				const struct core_surface *surface);

	void (*power_down)(struct core_dc *dc);

	void (*enable_accelerated_mode)(struct core_dc *dc);

	void (*enable_timing_synchronization)(
			struct core_dc *dc,
			int group_index,
			int group_size,
			struct pipe_ctx *grouped_pipes[]);

	/* backlight control */
	void (*encoder_set_lcd_backlight_level)(
		struct link_encoder *enc, uint32_t level);

	void (*crtc_switch_to_clk_src)(struct clock_source *, uint8_t);

	/* power management */
	void (*clock_gating_power_up)(struct dc_context *ctx, bool enable);

	void (*enable_display_pipe_clock_gating)(
					struct dc_context *ctx,
					bool clock_gating);

	bool (*enable_display_power_gating)(
					struct core_dc *dc,
					uint8_t controller_id,
					struct dc_bios *dcb,
					enum pipe_gating_control power_gating);

	void (*power_down_front_end)(struct core_dc *dc, struct pipe_ctx *pipe);
	void (*update_info_frame)(struct pipe_ctx *pipe_ctx);

	void (*enable_stream)(struct pipe_ctx *pipe_ctx);

	void (*disable_stream)(struct pipe_ctx *pipe_ctx);

	void (*enable_fe_clock)(
		struct dc_context *ctx, uint8_t controller_id, bool enable);

	bool (*pipe_control_lock)(
				struct dc_context *ctx,
				uint8_t controller_idx,
				uint32_t control_mask,
				bool lock);

	void (*set_blender_mode)(
				struct core_dc *dc,
				uint8_t controller_id,
				uint32_t mode);

	void (*set_displaymarks)(
				const struct core_dc *dc,
				struct validate_context *context);

	void (*increase_watermarks_for_pipe)(struct core_dc *dc,
			struct pipe_ctx *pipe_ctx,
			struct validate_context *context);

	void (*set_display_clock)(struct validate_context *context);

	void (*set_bandwidth)(struct core_dc *dc);

	void (*set_drr)(struct pipe_ctx **pipe_ctx, int num_pipes,
			int vmin, int vmax);

	void (*set_static_screen_control)(struct pipe_ctx **pipe_ctx,
			int num_pipes, int value);
};

void color_space_to_black_color(
		enum dc_color_space colorspace,
	struct tg_color *black_color);

#endif /* __DC_HW_SEQUENCER_H__ */
