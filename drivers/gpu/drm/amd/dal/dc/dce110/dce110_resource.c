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

#include "dm_services.h"

#include "link_encoder.h"
#include "stream_encoder.h"

#include "resource.h"
#include "dce110/dce110_resource.h"
#include "include/irq_service_interface.h"
#include "../virtual/virtual_stream_encoder.h"
#include "dce110/audio_dce110.h"
#include "dce110/dce110_timing_generator.h"
#include "dce110/dce110_timing_generator_v.h"
#include "dce110/dce110_link_encoder.h"
#include "dce110/dce110_mem_input.h"
#include "dce110/dce110_mem_input_v.h"
#include "dce110/dce110_ipp.h"
#include "dce110/dce110_transform.h"
#include "dce110/dce110_transform_v.h"
#include "dce110/dce110_stream_encoder.h"
#include "dce110/dce110_opp.h"
#include "dce110/dce110_opp_v.h"
#include "dce110/dce110_clock_source.h"
#include "dce110/dce110_hw_sequencer.h"
#include "dce/dce_11_0_d.h"
#include "adapter_service_interface.h"

#ifndef mmDP_DPHY_INTERNAL_CTRL
	#define mmDP_DPHY_INTERNAL_CTRL 0x4aa7
	#define mmDP0_DP_DPHY_INTERNAL_CTRL 0x4aa7
	#define mmDP1_DP_DPHY_INTERNAL_CTRL 0x4ba7
	#define mmDP2_DP_DPHY_INTERNAL_CTRL 0x4ca7
	#define mmDP3_DP_DPHY_INTERNAL_CTRL 0x4da7
	#define mmDP4_DP_DPHY_INTERNAL_CTRL 0x4ea7
	#define mmDP5_DP_DPHY_INTERNAL_CTRL 0x4fa7
	#define mmDP6_DP_DPHY_INTERNAL_CTRL 0x54a7
	#define mmDP7_DP_DPHY_INTERNAL_CTRL 0x56a7
	#define mmDP8_DP_DPHY_INTERNAL_CTRL 0x57a7
#endif

#ifndef mmBIOS_SCRATCH_2
	#define mmBIOS_SCRATCH_2 0x05CB
#endif

#ifndef mmDP_DPHY_BS_SR_SWAP_CNTL
	#define mmDP_DPHY_BS_SR_SWAP_CNTL                       0x4ADC
	#define mmDP0_DP_DPHY_BS_SR_SWAP_CNTL                   0x4ADC
	#define mmDP1_DP_DPHY_BS_SR_SWAP_CNTL                   0x4BDC
	#define mmDP2_DP_DPHY_BS_SR_SWAP_CNTL                   0x4CDC
	#define mmDP3_DP_DPHY_BS_SR_SWAP_CNTL                   0x4DDC
	#define mmDP4_DP_DPHY_BS_SR_SWAP_CNTL                   0x4EDC
	#define mmDP5_DP_DPHY_BS_SR_SWAP_CNTL                   0x4FDC
	#define mmDP6_DP_DPHY_BS_SR_SWAP_CNTL                   0x54DC
#endif

#ifndef mmDP_DPHY_FAST_TRAINING
	#define mmDP_DPHY_FAST_TRAINING                         0x4ABC
	#define mmDP0_DP_DPHY_FAST_TRAINING                     0x4ABC
	#define mmDP1_DP_DPHY_FAST_TRAINING                     0x4BBC
	#define mmDP2_DP_DPHY_FAST_TRAINING                     0x4CBC
	#define mmDP3_DP_DPHY_FAST_TRAINING                     0x4DBC
	#define mmDP4_DP_DPHY_FAST_TRAINING                     0x4EBC
	#define mmDP5_DP_DPHY_FAST_TRAINING                     0x4FBC
	#define mmDP6_DP_DPHY_FAST_TRAINING                     0x54BC
#endif

#ifndef DPHY_RX_FAST_TRAINING_CAPABLE
	#define DPHY_RX_FAST_TRAINING_CAPABLE 0x1
#endif

static const struct dce110_timing_generator_offsets dce110_tg_offsets[] = {
	{
		.crtc = (mmCRTC0_CRTC_CONTROL - mmCRTC_CONTROL),
		.dcp =  (mmDCP0_GRPH_CONTROL - mmGRPH_CONTROL),
	},
	{
		.crtc = (mmCRTC1_CRTC_CONTROL - mmCRTC_CONTROL),
		.dcp = (mmDCP1_GRPH_CONTROL - mmGRPH_CONTROL),
	},
	{
		.crtc = (mmCRTC2_CRTC_CONTROL - mmCRTC_CONTROL),
		.dcp = (mmDCP2_GRPH_CONTROL - mmGRPH_CONTROL),
	},
	{
		.crtc = (mmCRTC3_CRTC_CONTROL - mmCRTC_CONTROL),
		.dcp =  (mmDCP3_GRPH_CONTROL - mmGRPH_CONTROL),
	},
	{
		.crtc = (mmCRTC4_CRTC_CONTROL - mmCRTC_CONTROL),
		.dcp = (mmDCP4_GRPH_CONTROL - mmGRPH_CONTROL),
	},
	{
		.crtc = (mmCRTC5_CRTC_CONTROL - mmCRTC_CONTROL),
		.dcp = (mmDCP5_GRPH_CONTROL - mmGRPH_CONTROL),
	}
};

static const struct dce110_mem_input_reg_offsets dce110_mi_reg_offsets[] = {
	{
		.dcp = (mmDCP0_GRPH_CONTROL - mmGRPH_CONTROL),
		.dmif = (mmDMIF_PG0_DPG_WATERMARK_MASK_CONTROL
				- mmDPG_WATERMARK_MASK_CONTROL),
		.pipe = (mmPIPE0_DMIF_BUFFER_CONTROL
				- mmPIPE0_DMIF_BUFFER_CONTROL),
	},
	{
		.dcp = (mmDCP1_GRPH_CONTROL - mmGRPH_CONTROL),
		.dmif = (mmDMIF_PG1_DPG_WATERMARK_MASK_CONTROL
				- mmDPG_WATERMARK_MASK_CONTROL),
		.pipe = (mmPIPE1_DMIF_BUFFER_CONTROL
				- mmPIPE0_DMIF_BUFFER_CONTROL),
	},
	{
		.dcp = (mmDCP2_GRPH_CONTROL - mmGRPH_CONTROL),
		.dmif = (mmDMIF_PG2_DPG_WATERMARK_MASK_CONTROL
				- mmDPG_WATERMARK_MASK_CONTROL),
		.pipe = (mmPIPE2_DMIF_BUFFER_CONTROL
				- mmPIPE0_DMIF_BUFFER_CONTROL),
	}
};

static const struct dce110_transform_reg_offsets dce110_xfm_offsets[] = {
{
	.scl_offset = (mmSCL0_SCL_CONTROL - mmSCL_CONTROL),
	.dcfe_offset = (mmDCFE0_DCFE_MEM_PWR_CTRL - mmDCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP0_GRPH_CONTROL - mmGRPH_CONTROL),
	.lb_offset = (mmLB0_LB_DATA_FORMAT - mmLB_DATA_FORMAT),
},
{	.scl_offset = (mmSCL1_SCL_CONTROL - mmSCL_CONTROL),
	.dcfe_offset = (mmDCFE1_DCFE_MEM_PWR_CTRL - mmDCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP1_GRPH_CONTROL - mmGRPH_CONTROL),
	.lb_offset = (mmLB1_LB_DATA_FORMAT - mmLB_DATA_FORMAT),
},
{	.scl_offset = (mmSCL2_SCL_CONTROL - mmSCL_CONTROL),
	.dcfe_offset = (mmDCFE2_DCFE_MEM_PWR_CTRL - mmDCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP2_GRPH_CONTROL - mmGRPH_CONTROL),
	.lb_offset = (mmLB2_LB_DATA_FORMAT - mmLB_DATA_FORMAT),
}
};

static const struct dce110_ipp_reg_offsets dce110_ipp_reg_offsets[] = {
{
	.dcp_offset = (mmDCP0_CUR_CONTROL - mmCUR_CONTROL),
},
{
	.dcp_offset = (mmDCP1_CUR_CONTROL - mmCUR_CONTROL),
},
{
	.dcp_offset = (mmDCP2_CUR_CONTROL - mmCUR_CONTROL),
},
{
	.dcp_offset = (mmDCP3_CUR_CONTROL - mmCUR_CONTROL),
},
{
	.dcp_offset = (mmDCP4_CUR_CONTROL - mmCUR_CONTROL),
},
{
	.dcp_offset = (mmDCP5_CUR_CONTROL - mmCUR_CONTROL),
}
};




/* set register offset */
#define SR(reg_name)\
	.reg_name = mm ## reg_name

/* set register offset with instance */
#define SRI(reg_name, block, id)\
	.reg_name = mm ## block ## id ## _ ## reg_name



#define aux_regs(id)\
[id] = {\
	AUX_REG_LIST(id)\
}

static const struct dce110_link_enc_aux_registers link_enc_aux_regs[] = {
		aux_regs(0),
		aux_regs(1),
		aux_regs(2),
		aux_regs(3),
		aux_regs(4),
		aux_regs(5)
};

#define link_regs(id)\
[id] = {\
	LE_DCE110_REG_LIST(id)\
}

static const struct dce110_link_enc_registers link_enc_regs[] = {
	link_regs(0),
	link_regs(1),
	link_regs(2),
	link_regs(3),
	link_regs(4),
	link_regs(5),
	link_regs(6),
};

#define stream_enc_regs(id)\
[id] = {\
	SE_COMMON_REG_LIST(id)\
}

static const struct dce110_stream_enc_registers stream_enc_regs[] = {
	stream_enc_regs(0),
	stream_enc_regs(1),
	stream_enc_regs(2),
	stream_enc_regs(3),
	stream_enc_regs(4),
	stream_enc_regs(5),
	stream_enc_regs(6),
};

#define audio_regs(id)\
[id] = {\
	AUD_COMMON_REG_LIST(id)\
}

static const struct dce110_audio_registers audio_regs[] = {
	audio_regs(0),
	audio_regs(1),
	audio_regs(2),
	audio_regs(3),
	audio_regs(4),
	audio_regs(5),
	audio_regs(6),
};

/* AG TBD Needs to be reduced back to 3 pipes once dce10 hw sequencer implemented. */
static const struct dce110_opp_reg_offsets dce110_opp_reg_offsets[] = {
{
	.fmt_offset = (mmFMT0_FMT_CONTROL - mmFMT0_FMT_CONTROL),
	.dcfe_offset = (mmDCFE0_DCFE_MEM_PWR_CTRL - mmDCFE0_DCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP0_GRPH_CONTROL - mmDCP0_GRPH_CONTROL),
},
{	.fmt_offset = (mmFMT1_FMT_CONTROL - mmFMT0_FMT_CONTROL),
	.dcfe_offset = (mmDCFE1_DCFE_MEM_PWR_CTRL - mmDCFE0_DCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP1_GRPH_CONTROL - mmDCP0_GRPH_CONTROL),
},
{	.fmt_offset = (mmFMT2_FMT_CONTROL - mmFMT0_FMT_CONTROL),
	.dcfe_offset = (mmDCFE2_DCFE_MEM_PWR_CTRL - mmDCFE0_DCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP2_GRPH_CONTROL - mmDCP0_GRPH_CONTROL),
},
{
	.fmt_offset = (mmFMT3_FMT_CONTROL - mmFMT0_FMT_CONTROL),
	.dcfe_offset = (mmDCFE3_DCFE_MEM_PWR_CTRL - mmDCFE0_DCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP3_GRPH_CONTROL - mmDCP0_GRPH_CONTROL),
},
{	.fmt_offset = (mmFMT4_FMT_CONTROL - mmFMT0_FMT_CONTROL),
	.dcfe_offset = (mmDCFE4_DCFE_MEM_PWR_CTRL - mmDCFE0_DCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP4_GRPH_CONTROL - mmDCP0_GRPH_CONTROL),
},
{	.fmt_offset = (mmFMT5_FMT_CONTROL - mmFMT0_FMT_CONTROL),
	.dcfe_offset = (mmDCFE5_DCFE_MEM_PWR_CTRL - mmDCFE0_DCFE_MEM_PWR_CTRL),
	.dcp_offset = (mmDCP5_GRPH_CONTROL - mmDCP0_GRPH_CONTROL),
}
};

static const struct dce110_clk_src_reg_offsets dce110_clk_src_reg_offsets[] = {
	{
		.pll_cntl = mmBPHYC_PLL0_PLL_CNTL,
		.pixclk_resync_cntl  = mmPIXCLK0_RESYNC_CNTL
	},
	{
		.pll_cntl = mmBPHYC_PLL1_PLL_CNTL,
		.pixclk_resync_cntl  = mmPIXCLK1_RESYNC_CNTL
	},
	{
		.pll_cntl = mmBPHYC_PLL2_PLL_CNTL,
		.pixclk_resync_cntl  = mmPIXCLK2_RESYNC_CNTL
	}
};

static struct timing_generator *dce110_timing_generator_create(
		struct adapter_service *as,
		struct dc_context *ctx,
		uint32_t instance,
		const struct dce110_timing_generator_offsets *offsets)
{
	struct dce110_timing_generator *tg110 =
		dm_alloc(sizeof(struct dce110_timing_generator));

	if (!tg110)
		return NULL;

	if (dce110_timing_generator_construct(tg110, as, ctx, instance, offsets))
		return &tg110->base;

	BREAK_TO_DEBUGGER();
	dm_free(tg110);
	return NULL;
}

static struct stream_encoder *dce110_stream_encoder_create(
	enum engine_id eng_id,
	struct dc_context *ctx,
	struct dc_bios *bp,
	const struct dce110_stream_enc_registers *regs)
{
	struct dce110_stream_encoder *enc110 =
		dm_alloc(sizeof(struct dce110_stream_encoder));

	if (!enc110)
		return NULL;

	if (dce110_stream_encoder_construct(enc110, ctx, bp, eng_id, regs))
		return &enc110->base;

	BREAK_TO_DEBUGGER();
	dm_free(enc110);
	return NULL;
}

static struct mem_input *dce110_mem_input_create(
	struct dc_context *ctx,
	struct adapter_service *as,
	uint32_t inst,
	const struct dce110_mem_input_reg_offsets *offset)
{
	struct dce110_mem_input *mem_input110 =
		dm_alloc(sizeof(struct dce110_mem_input));

	if (!mem_input110)
		return NULL;

	if (dce110_mem_input_construct(mem_input110,
				       ctx, as, inst, offset))
		return &mem_input110->base;

	BREAK_TO_DEBUGGER();
	dm_free(mem_input110);
	return NULL;
}

static void dce110_transform_destroy(struct transform **xfm)
{
	dm_free(TO_DCE110_TRANSFORM(*xfm));
	*xfm = NULL;
}

static struct transform *dce110_transform_create(
	struct dc_context *ctx,
	uint32_t inst,
	const struct dce110_transform_reg_offsets *offsets)
{
	struct dce110_transform *transform =
		dm_alloc(sizeof(struct dce110_transform));

	if (!transform)
		return NULL;

	if (dce110_transform_construct(transform, ctx, inst, offsets))
		return &transform->base;

	BREAK_TO_DEBUGGER();
	dm_free(transform);
	return NULL;
}

static struct input_pixel_processor *dce110_ipp_create(
	struct dc_context *ctx,
	uint32_t inst,
	const struct dce110_ipp_reg_offsets *offsets)
{
	struct dce110_ipp *ipp =
		dm_alloc(sizeof(struct dce110_ipp));

	if (!ipp)
		return NULL;

	if (dce110_ipp_construct(ipp, ctx, inst, offsets))
		return &ipp->base;

	BREAK_TO_DEBUGGER();
	dm_free(ipp);
	return NULL;
}

struct link_encoder *dce110_link_encoder_create(
	const struct encoder_init_data *enc_init_data)
{
	struct dce110_link_encoder *enc110 =
		dm_alloc(sizeof(struct dce110_link_encoder));

	if (!enc110)
		return NULL;

	if (dce110_link_encoder_construct(
			enc110,
			enc_init_data,
			&link_enc_regs[enc_init_data->transmitter],
			&link_enc_aux_regs[enc_init_data->channel - 1]))
		return &enc110->base;

	BREAK_TO_DEBUGGER();
	dm_free(enc110);
	return NULL;
}

static struct output_pixel_processor *dce110_opp_create(
	struct dc_context *ctx,
	uint32_t inst,
	const struct dce110_opp_reg_offsets *offsets)
{
	struct dce110_opp *opp =
		dm_alloc(sizeof(struct dce110_opp));

	if (!opp)
		return NULL;

	if (dce110_opp_construct(opp,
			ctx, inst, offsets))
		return &opp->base;

	BREAK_TO_DEBUGGER();
	dm_free(opp);
	return NULL;
}

struct clock_source *dce110_clock_source_create(
	struct dc_context *ctx,
	struct dc_bios *bios,
	enum clock_source_id id,
	const struct dce110_clk_src_reg_offsets *offsets)
{
	struct dce110_clk_src *clk_src =
		dm_alloc(sizeof(struct dce110_clk_src));

	if (!clk_src)
		return NULL;

	if (dce110_clk_src_construct(clk_src, ctx, bios, id, offsets))
		return &clk_src->base;

	BREAK_TO_DEBUGGER();
	return NULL;
}

void dce110_clock_source_destroy(struct clock_source **clk_src)
{
	struct dce110_clk_src *dce110_clk_src;

	if (!clk_src)
		return;

	dce110_clk_src = TO_DCE110_CLK_SRC(*clk_src);

	if (dce110_clk_src->dp_ss_params)
		dm_free(dce110_clk_src->dp_ss_params);

	if (dce110_clk_src->hdmi_ss_params)
		dm_free(dce110_clk_src->hdmi_ss_params);

	if (dce110_clk_src->dvi_ss_params)
		dm_free(dce110_clk_src->dvi_ss_params);

	dm_free(dce110_clk_src);
	*clk_src = NULL;
}

static void destruct(struct dce110_resource_pool *pool)
{
	unsigned int i;

	for (i = 0; i < pool->base.pipe_count; i++) {
		if (pool->base.opps[i] != NULL)
			dce110_opp_destroy(&pool->base.opps[i]);

		if (pool->base.transforms[i] != NULL)
			dce110_transform_destroy(&pool->base.transforms[i]);

		if (pool->base.ipps[i] != NULL)
			dce110_ipp_destroy(&pool->base.ipps[i]);

		if (pool->base.mis[i] != NULL) {
			dm_free(TO_DCE110_MEM_INPUT(pool->base.mis[i]));
			pool->base.mis[i] = NULL;
		}

		if (pool->base.timing_generators[i] != NULL)	{
			dm_free(DCE110TG_FROM_TG(pool->base.timing_generators[i]));
			pool->base.timing_generators[i] = NULL;
		}
	}

	for (i = 0; i < pool->base.stream_enc_count; i++) {
		if (pool->base.stream_enc[i] != NULL)
			dm_free(DCE110STRENC_FROM_STRENC(pool->base.stream_enc[i]));
	}

	for (i = 0; i < pool->base.clk_src_count; i++) {
		if (pool->base.clock_sources[i] != NULL) {
			dce110_clock_source_destroy(&pool->base.clock_sources[i]);
		}
	}

	if (pool->base.dp_clock_source != NULL)
		dce110_clock_source_destroy(&pool->base.dp_clock_source);

	for (i = 0; i < pool->base.audio_count; i++)	{
		if (pool->base.audios[i] != NULL) {
			dce110_aud_destroy(&pool->base.audios[i]);
		}
	}

	if (pool->base.display_clock != NULL) {
		dal_display_clock_destroy(&pool->base.display_clock);
	}

	if (pool->base.scaler_filter != NULL) {
		dal_scaler_filter_destroy(&pool->base.scaler_filter);
	}
	if (pool->base.irqs != NULL) {
		dal_irq_service_destroy(&pool->base.irqs);
	}

	if (pool->base.adapter_srv != NULL) {
		dal_adapter_service_destroy(&pool->base.adapter_srv);
	}
}


static void get_pixel_clock_parameters(
	const struct pipe_ctx *pipe_ctx,
	struct pixel_clk_params *pixel_clk_params)
{
	const struct core_stream *stream = pipe_ctx->stream;

	/*TODO: is this halved for YCbCr 420? in that case we might want to move
	 * the pixel clock normalization for hdmi up to here instead of doing it
	 * in pll_adjust_pix_clk
	 */
	pixel_clk_params->requested_pix_clk = stream->public.timing.pix_clk_khz;
	pixel_clk_params->encoder_object_id = stream->sink->link->link_enc->id;
	pixel_clk_params->signal_type = pipe_ctx->stream->signal;
	pixel_clk_params->controller_id = pipe_ctx->pipe_idx + 1;
	/* TODO: un-hardcode*/
	pixel_clk_params->requested_sym_clk = LINK_RATE_LOW *
						LINK_RATE_REF_FREQ_IN_KHZ;
	pixel_clk_params->flags.ENABLE_SS = 0;
	pixel_clk_params->color_depth =
		stream->public.timing.display_color_depth;
	pixel_clk_params->flags.DISPLAY_BLANKED = 1;
	pixel_clk_params->flags.SUPPORT_YCBCR420 = (stream->public.timing.pixel_encoding ==
			PIXEL_ENCODING_YCBCR420);
}

void dce110_resource_build_bit_depth_reduction_params(
		const struct core_stream *stream,
		struct bit_depth_reduction_params *fmt_bit_depth)
{
	memset(fmt_bit_depth, 0, sizeof(*fmt_bit_depth));

	/*TODO: Need to un-hardcode, refer to function with same name
	 * in dal2 hw_sequencer*/

	fmt_bit_depth->flags.TRUNCATE_ENABLED = 0;
	fmt_bit_depth->flags.SPATIAL_DITHER_ENABLED = 0;
	fmt_bit_depth->flags.FRAME_MODULATION_ENABLED = 0;

	/* Diagnostics need consistent CRC of the image, that means
	 * dithering should not be enabled for Diagnostics. */
	if (IS_DIAG_DC(stream->ctx->dce_environment) == false) {
		switch (stream->public.timing.display_color_depth) {
		case COLOR_DEPTH_666:
			fmt_bit_depth->flags.SPATIAL_DITHER_ENABLED = 1;
			fmt_bit_depth->flags.SPATIAL_DITHER_DEPTH = 0;
		break;
		case COLOR_DEPTH_888:
			fmt_bit_depth->flags.SPATIAL_DITHER_ENABLED = 1;
			fmt_bit_depth->flags.SPATIAL_DITHER_DEPTH = 1;
		break;
		case COLOR_DEPTH_101010:
			fmt_bit_depth->flags.SPATIAL_DITHER_ENABLED = 1;
			fmt_bit_depth->flags.SPATIAL_DITHER_DEPTH = 2;
		break;
		default:
		break;
		}
		fmt_bit_depth->flags.RGB_RANDOM = 1;
		fmt_bit_depth->flags.HIGHPASS_RANDOM = 1;
		fmt_bit_depth->flags.TRUNCATE_ENABLED = 1;
		fmt_bit_depth->flags.TRUNCATE_DEPTH = 2;
	}

	return;
}

enum dc_status dce110_resource_build_pipe_hw_param(struct pipe_ctx *pipe_ctx)
{
	get_pixel_clock_parameters(pipe_ctx, &pipe_ctx->pix_clk_params);
	pipe_ctx->clock_source->funcs->get_pix_clk_dividers(
		pipe_ctx->clock_source,
		&pipe_ctx->pix_clk_params,
		&pipe_ctx->pll_settings);
	dce110_resource_build_bit_depth_reduction_params(pipe_ctx->stream,
			&pipe_ctx->stream->bit_depth_params);
	pipe_ctx->stream->clamping.pixel_encoding = pipe_ctx->stream->public.timing.pixel_encoding;

	return DC_OK;
}

static bool is_surface_pixel_format_supported(struct pipe_ctx *pipe_ctx, unsigned int underlay_idx)
{
	if (pipe_ctx->pipe_idx != underlay_idx)
		return true;
	if (!pipe_ctx->surface)
		return false;
	if (pipe_ctx->surface->public.format < SURFACE_PIXEL_FORMAT_VIDEO_BEGIN)
		return false;
	return true;
}

static enum dc_status validate_mapped_resource(
		const struct core_dc *dc,
		struct validate_context *context)
{
	enum dc_status status = DC_OK;
	uint8_t i, j, k;

	for (i = 0; i < context->target_count; i++) {
		struct core_target *target = context->targets[i];

		for (j = 0; j < target->public.stream_count; j++) {
			struct core_stream *stream =
				DC_STREAM_TO_CORE(target->public.streams[j]);
			struct core_link *link = stream->sink->link;

			if (resource_is_stream_unchanged(dc->current_context, stream))
				continue;

			for (k = 0; k < MAX_PIPES; k++) {
				struct pipe_ctx *pipe_ctx =
					&context->res_ctx.pipe_ctx[k];

				if (context->res_ctx.pipe_ctx[k].stream != stream)
					continue;

				if (!is_surface_pixel_format_supported(pipe_ctx,
						context->res_ctx.pool->underlay_pipe_index))
					return DC_SURFACE_PIXEL_FORMAT_UNSUPPORTED;

				if (!pipe_ctx->tg->funcs->validate_timing(
					pipe_ctx->tg, &stream->public.timing))
					return DC_FAIL_CONTROLLER_VALIDATE;

				status = dce110_resource_build_pipe_hw_param(pipe_ctx);

				if (status != DC_OK)
					return status;

				if (!link->link_enc->funcs->validate_output_with_stream(
						link->link_enc,
						pipe_ctx))
					return DC_FAIL_ENC_VALIDATE;

				/* TODO: validate audio ASIC caps, encoder */

				status = dc_link_validate_mode_timing(stream,
						link,
						&stream->public.timing);

				if (status != DC_OK)
					return status;

				resource_build_info_frame(pipe_ctx);

				/* do not need to validate non root pipes */
				break;
			}
		}
	}

	return DC_OK;
}

enum dc_status dce110_validate_bandwidth(
	const struct core_dc *dc,
	struct validate_context *context)
{
	enum dc_status result = DC_ERROR_UNEXPECTED;

	dal_logger_write(
		dc->ctx->logger,
		LOG_MAJOR_BWM,
		LOG_MINOR_BWM_REQUIRED_BANDWIDTH_CALCS,
		"%s: start",
		__func__);

	if (!bw_calcs(
			dc->ctx,
			&dc->bw_dceip,
			&dc->bw_vbios,
			context->res_ctx.pipe_ctx,
			context->res_ctx.pool->pipe_count,
			&context->bw_results))
		result =  DC_FAIL_BANDWIDTH_VALIDATE;
	else
		result =  DC_OK;

	if (result == DC_FAIL_BANDWIDTH_VALIDATE)
		dal_logger_write(dc->ctx->logger,
			LOG_MAJOR_BWM,
			LOG_MINOR_BWM_MODE_VALIDATION,
			"%s: %dx%d@%d Bandwidth validation failed!\n",
			__func__,
			context->targets[0]->public.streams[0]->timing.h_addressable,
			context->targets[0]->public.streams[0]->timing.v_addressable,
			context->targets[0]->public.streams[0]->timing.pix_clk_khz);

	if (memcmp(&dc->current_context->bw_results,
			&context->bw_results, sizeof(context->bw_results))) {
		struct log_entry log_entry;
		dal_logger_open(
			dc->ctx->logger,
			&log_entry,
			LOG_MAJOR_BWM,
			LOG_MINOR_BWM_REQUIRED_BANDWIDTH_CALCS);
		dal_logger_append(&log_entry, "%s: finish,\n"
			"nbpMark_b: %d nbpMark_a: %d urgentMark_b: %d urgentMark_a: %d\n"
			"stutMark_b: %d stutMark_a: %d\n",
			__func__,
			context->bw_results.nbp_state_change_wm_ns[0].b_mark,
			context->bw_results.nbp_state_change_wm_ns[0].a_mark,
			context->bw_results.urgent_wm_ns[0].b_mark,
			context->bw_results.urgent_wm_ns[0].a_mark,
			context->bw_results.stutter_exit_wm_ns[0].b_mark,
			context->bw_results.stutter_exit_wm_ns[0].a_mark);
		dal_logger_append(&log_entry,
			"nbpMark_b: %d nbpMark_a: %d urgentMark_b: %d urgentMark_a: %d\n"
			"stutMark_b: %d stutMark_a: %d\n",
			context->bw_results.nbp_state_change_wm_ns[1].b_mark,
			context->bw_results.nbp_state_change_wm_ns[1].a_mark,
			context->bw_results.urgent_wm_ns[1].b_mark,
			context->bw_results.urgent_wm_ns[1].a_mark,
			context->bw_results.stutter_exit_wm_ns[1].b_mark,
			context->bw_results.stutter_exit_wm_ns[1].a_mark);
		dal_logger_append(&log_entry,
			"nbpMark_b: %d nbpMark_a: %d urgentMark_b: %d urgentMark_a: %d\n"
			"stutMark_b: %d stutMark_a: %d stutter_mode_enable: %d\n",
			context->bw_results.nbp_state_change_wm_ns[2].b_mark,
			context->bw_results.nbp_state_change_wm_ns[2].a_mark,
			context->bw_results.urgent_wm_ns[2].b_mark,
			context->bw_results.urgent_wm_ns[2].a_mark,
			context->bw_results.stutter_exit_wm_ns[2].b_mark,
			context->bw_results.stutter_exit_wm_ns[2].a_mark,
			context->bw_results.stutter_mode_enable);
		dal_logger_append(&log_entry,
			"cstate: %d pstate: %d nbpstate: %d sync: %d dispclk: %d\n"
			"sclk: %d sclk_sleep: %d yclk: %d blackout_recovery_time_us: %d\n",
			context->bw_results.cpuc_state_change_enable,
			context->bw_results.cpup_state_change_enable,
			context->bw_results.nbp_state_change_enable,
			context->bw_results.all_displays_in_sync,
			context->bw_results.dispclk_khz,
			context->bw_results.required_sclk,
			context->bw_results.required_sclk_deep_sleep,
			context->bw_results.required_yclk,
			context->bw_results.blackout_recovery_time_us);
		dal_logger_close(&log_entry);
	}
	return result;
}

static bool dce110_validate_surface_sets(
		const struct dc_validation_set set[],
		int set_count)
{
	int i;

	for (i = 0; i < set_count; i++) {
		if (set[i].surface_count == 0)
			continue;

		if (set[i].surface_count > 2)
			return false;

		if (set[i].surfaces[0]->src_rect.width
				!= set[i].target->streams[0]->src.width
				|| set[i].surfaces[0]->src_rect.height
				!= set[i].target->streams[0]->src.height)
			return false;
		if (set[i].surfaces[0]->format
				>= SURFACE_PIXEL_FORMAT_VIDEO_BEGIN)
			return false;

		if (set[i].surface_count == 2) {
			if (set[i].surfaces[1]->format
					< SURFACE_PIXEL_FORMAT_VIDEO_BEGIN)
				return false;
			if (set[i].surfaces[1]->src_rect.width > 1920
					|| set[i].surfaces[1]->src_rect.height > 1080)
				return false;

			if (set[i].target->streams[0]->timing.pixel_encoding != PIXEL_ENCODING_RGB)
				return false;
		}
	}

	return true;
}

enum dc_status dce110_validate_with_context(
		const struct core_dc *dc,
		const struct dc_validation_set set[],
		int set_count,
		struct validate_context *context)
{
	struct dc_context *dc_ctx = dc->ctx;
	enum dc_status result = DC_ERROR_UNEXPECTED;
	int i;

	if (!dce110_validate_surface_sets(set, set_count))
		return DC_FAIL_SURFACE_VALIDATE;

	context->res_ctx.pool = dc->res_pool;

	for (i = 0; i < set_count; i++) {
		context->targets[i] = DC_TARGET_TO_CORE(set[i].target);
		dc_target_retain(&context->targets[i]->public);
		context->target_count++;
	}

	result = resource_map_pool_resources(dc, context);

	if (result == DC_OK)
		result = resource_map_clock_resources(dc, context);

	if (!resource_validate_attach_surfaces(
			set, set_count, dc->current_context, context)) {
		DC_ERROR("Failed to attach surface to target!\n");
		return DC_FAIL_ATTACH_SURFACES;
	}

	if (result == DC_OK)
		result = validate_mapped_resource(dc, context);

	if (result == DC_OK)
		result = resource_build_scaling_params_for_context(dc, context);

	if (result == DC_OK)
		result = dce110_validate_bandwidth(dc, context);

	return result;
}

enum dc_status dce110_validate_guaranteed(
		const struct core_dc *dc,
		const struct dc_target *dc_target,
		struct validate_context *context)
{
	enum dc_status result = DC_ERROR_UNEXPECTED;

	context->res_ctx.pool = dc->res_pool;

	context->targets[0] = DC_TARGET_TO_CORE(dc_target);
	dc_target_retain(&context->targets[0]->public);
	context->target_count++;

	result = resource_map_pool_resources(dc, context);

	if (result == DC_OK)
		result = resource_map_clock_resources(dc, context);

	if (result == DC_OK)
		result = validate_mapped_resource(dc, context);

	if (result == DC_OK) {
		validate_guaranteed_copy_target(
				context, dc->public.caps.max_targets);
		result = resource_build_scaling_params_for_context(dc, context);
	}

	if (result == DC_OK)
		result = dce110_validate_bandwidth(dc, context);

	return result;
}

static struct pipe_ctx *dce110_acquire_idle_pipe_for_layer(
		struct resource_context *res_ctx,
		struct core_stream *stream)
{
	unsigned int underlay_idx = res_ctx->pool->underlay_pipe_index;
	struct pipe_ctx *pipe_ctx = &res_ctx->pipe_ctx[underlay_idx];

	if (res_ctx->pipe_ctx[underlay_idx].stream) {
		return NULL;
	}

	pipe_ctx->tg = res_ctx->pool->timing_generators[underlay_idx];
	pipe_ctx->mi = res_ctx->pool->mis[underlay_idx];
	/*pipe_ctx->ipp = res_ctx->pool->ipps[underlay_idx];*/
	pipe_ctx->xfm = res_ctx->pool->transforms[underlay_idx];
	pipe_ctx->opp = res_ctx->pool->opps[underlay_idx];
	pipe_ctx->dis_clk = res_ctx->pool->display_clock;
	pipe_ctx->pipe_idx = underlay_idx;

	pipe_ctx->stream = stream;

	return pipe_ctx;

}

static void dce110_destroy_resource_pool(struct resource_pool **pool)
{
	struct dce110_resource_pool *dce110_pool = TO_DCE110_RES_POOL(*pool);

	destruct(dce110_pool);
	dm_free(dce110_pool);
	*pool = NULL;
}


static const struct resource_funcs dce110_res_pool_funcs = {
	.destroy = dce110_destroy_resource_pool,
	.link_enc_create = dce110_link_encoder_create,
	.validate_with_context = dce110_validate_with_context,
	.validate_guaranteed = dce110_validate_guaranteed,
	.validate_bandwidth = dce110_validate_bandwidth,
	.acquire_idle_pipe_for_layer = dce110_acquire_idle_pipe_for_layer
};

static void underlay_create(struct dc_context *ctx, struct resource_pool *pool)
{
	struct dce110_timing_generator *dce110_tgv = dm_alloc(sizeof (*dce110_tgv));
	struct dce110_transform *dce110_xfmv = dm_alloc(sizeof (*dce110_xfmv));
	struct dce110_mem_input *dce110_miv = dm_alloc(sizeof (*dce110_miv));
	struct dce110_opp *dce110_oppv = dm_alloc(sizeof (*dce110_oppv));

	dce110_opp_v_construct(dce110_oppv, ctx);
	dce110_timing_generator_v_construct(dce110_tgv, pool->adapter_srv, ctx);
	dce110_mem_input_v_construct(dce110_miv, ctx, pool->adapter_srv);
	dce110_transform_v_construct(dce110_xfmv, ctx);

	pool->opps[pool->pipe_count] = &dce110_oppv->base;
	pool->timing_generators[pool->pipe_count] = &dce110_tgv->base;
	pool->mis[pool->pipe_count] = &dce110_miv->base;
	pool->transforms[pool->pipe_count] = &dce110_xfmv->base;

	pool->transforms[pool->pipe_count]->funcs->transform_set_scaler_filter(
			pool->transforms[pool->pipe_count],
			pool->scaler_filter);
	pool->pipe_count++;

	/* update the public caps to indicate an underlay is available */
	ctx->dc->caps.max_slave_planes = 1;
	ctx->dc->caps.max_slave_planes = 1;
}

static void bw_calcs_data_update_from_pplib(struct core_dc *dc)
{
	struct dm_pp_clock_levels clks = {0};

	/*do system clock*/
	dm_pp_get_clock_levels_by_type(
			dc->ctx,
			DM_PP_CLOCK_TYPE_ENGINE_CLK,
			&clks);
	/* convert all the clock fro kHz to fix point mHz */
	dc->bw_vbios.high_sclk = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels-1], 1000);
	dc->bw_vbios.mid1_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels/8], 1000);
	dc->bw_vbios.mid2_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels*2/8], 1000);
	dc->bw_vbios.mid3_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels*3/8], 1000);
	dc->bw_vbios.mid4_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels*4/8], 1000);
	dc->bw_vbios.mid5_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels*5/8], 1000);
	dc->bw_vbios.mid6_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels*6/8], 1000);
	dc->bw_vbios.low_sclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[0], 1000);
	dc->sclk_lvls = clks;

	/*do display clock*/
	dm_pp_get_clock_levels_by_type(
			dc->ctx,
			DM_PP_CLOCK_TYPE_DISPLAY_CLK,
			&clks);
	dc->bw_vbios.high_voltage_max_dispclk = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels-1], 1000);
	dc->bw_vbios.mid_voltage_max_dispclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[clks.num_levels>>1], 1000);
	dc->bw_vbios.low_voltage_max_dispclk  = bw_frc_to_fixed(
			clks.clocks_in_khz[0], 1000);

	/*do memory clock*/
	dm_pp_get_clock_levels_by_type(
			dc->ctx,
			DM_PP_CLOCK_TYPE_MEMORY_CLK,
			&clks);

	dc->bw_vbios.low_yclk = bw_frc_to_fixed(
		clks.clocks_in_khz[0] * MEMORY_TYPE_MULTIPLIER, 1000);
	dc->bw_vbios.mid_yclk = bw_frc_to_fixed(
		clks.clocks_in_khz[clks.num_levels>>1] * MEMORY_TYPE_MULTIPLIER,
		1000);
	dc->bw_vbios.high_yclk = bw_frc_to_fixed(
		clks.clocks_in_khz[clks.num_levels-1] * MEMORY_TYPE_MULTIPLIER,
		1000);
}

static bool construct(
	struct adapter_service *as,
	uint8_t num_virtual_links,
	struct core_dc *dc,
	struct dce110_resource_pool *pool,
	struct hw_asic_id asic_id)
{
	unsigned int i;
	struct dc_context *ctx = dc->ctx;
	struct firmware_info info;
	struct dc_bios *bp;

	pool->base.adapter_srv = as;
	pool->base.funcs = &dce110_res_pool_funcs;

	/*************************************************
	 *  Resource + asic cap harcoding                *
	 *************************************************/

	pool->base.pipe_count = 3;
	pool->base.stream_enc_count = 3;
	pool->base.underlay_pipe_index = 3;

	if (ASIC_REV_IS_STONEY(asic_id.hw_internal_rev)) {
		pool->base.pipe_count = 2;
		pool->base.stream_enc_count = 2;
		pool->base.underlay_pipe_index = 2;
	}

	dc->public.caps.max_downscale_ratio = 150;
	dc->public.caps.i2c_speed_in_khz = 100;

	/*************************************************
	 *  Create resources                             *
	 *************************************************/


	pool->base.stream_engines.engine.ENGINE_ID_DIGA = 1;
	pool->base.stream_engines.engine.ENGINE_ID_DIGB = 1;
	pool->base.stream_engines.engine.ENGINE_ID_DIGC = 1;
	pool->base.stream_engines.engine.ENGINE_ID_DIGD = 1;
	pool->base.stream_engines.engine.ENGINE_ID_DIGE = 1;
	pool->base.stream_engines.engine.ENGINE_ID_DIGF = 1;

	bp = dal_adapter_service_get_bios_parser(as);

	if (dal_adapter_service_get_firmware_info(as, &info) &&
		info.external_clock_source_frequency_for_dp != 0) {
		pool->base.dp_clock_source =
				dce110_clock_source_create(ctx, bp, CLOCK_SOURCE_ID_EXTERNAL, NULL);

		pool->base.clock_sources[0] =
				dce110_clock_source_create(ctx, bp, CLOCK_SOURCE_ID_PLL0, &dce110_clk_src_reg_offsets[0]);
		pool->base.clock_sources[1] =
				dce110_clock_source_create(ctx, bp, CLOCK_SOURCE_ID_PLL1, &dce110_clk_src_reg_offsets[1]);

		pool->base.clk_src_count = 2;

		/* TODO: find out if CZ support 3 PLLs */
	}

	if (pool->base.dp_clock_source == NULL) {
		dm_error("DC: failed to create dp clock source!\n");
		BREAK_TO_DEBUGGER();
		goto clk_src_create_fail;
	}

	for (i = 0; i < pool->base.clk_src_count; i++) {
		if (pool->base.clock_sources[i] == NULL) {
			dm_error("DC: failed to create clock sources!\n");
			BREAK_TO_DEBUGGER();
			goto clk_src_create_fail;
		}
	}

	pool->base.display_clock = dal_display_clock_dce110_create(ctx, as);
	if (pool->base.display_clock == NULL) {
		dm_error("DC: failed to create display clock!\n");
		BREAK_TO_DEBUGGER();
		goto disp_clk_create_fail;
	}

	{
		struct irq_service_init_data init_data;
		init_data.ctx = dc->ctx;
		pool->base.irqs = dal_irq_service_create(
				dal_adapter_service_get_dce_version(
					pool->base.adapter_srv),
				&init_data);
		if (!pool->base.irqs)
			goto irqs_create_fail;

	}

	pool->base.scaler_filter = dal_scaler_filter_create(ctx);
	if (pool->base.scaler_filter == NULL) {
		BREAK_TO_DEBUGGER();
		dm_error("DC: failed to create filter!\n");
		goto filter_create_fail;
	}

	for (i = 0; i < pool->base.pipe_count; i++) {
		pool->base.timing_generators[i] = dce110_timing_generator_create(
				as, ctx, i, &dce110_tg_offsets[i]);
		if (pool->base.timing_generators[i] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error("DC: failed to create tg!\n");
			goto controller_create_fail;
		}

		pool->base.mis[i] = dce110_mem_input_create(ctx, as, i,
				&dce110_mi_reg_offsets[i]);
		if (pool->base.mis[i] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error(
				"DC: failed to create memory input!\n");
			goto controller_create_fail;
		}

		pool->base.ipps[i] = dce110_ipp_create(ctx, i, &dce110_ipp_reg_offsets[i]);
		if (pool->base.ipps[i] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error(
				"DC: failed to create input pixel processor!\n");
			goto controller_create_fail;
		}

		pool->base.transforms[i] = dce110_transform_create(
					ctx, i, &dce110_xfm_offsets[i]);
		if (pool->base.transforms[i] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error(
				"DC: failed to create transform!\n");
			goto controller_create_fail;
		}
		pool->base.transforms[i]->funcs->transform_set_scaler_filter(
				pool->base.transforms[i],
				pool->base.scaler_filter);

		pool->base.opps[i] = dce110_opp_create(ctx, i, &dce110_opp_reg_offsets[i]);
		if (pool->base.opps[i] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error(
				"DC: failed to create output pixel processor!\n");
			goto controller_create_fail;
		}
	}

	pool->base.audio_count = 0;
	for (i = 0; i < pool->base.pipe_count; i++) {
		struct graphics_object_id obj_id;

		obj_id = dal_adapter_service_enum_audio_object(as, i);
		if (false == dal_graphics_object_id_is_valid(obj_id)) {
			/* no more valid audio objects */
			break;
		}

		pool->base.audios[i] = dce110_audio_create(
				ctx, i, &audio_regs[i]);

		if (pool->base.audios[i] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error("DC: failed to create audio!\n");
			goto audio_create_fail;
		}
		pool->base.audio_count++;
	}

	/* TODO: failure? */
	underlay_create(ctx, &pool->base);

	for (i = 0; i < pool->base.stream_enc_count; i++) {
		/* TODO: rework fragile code*/
		if (pool->base.stream_engines.u_all & 1 << i) {
			pool->base.stream_enc[i] = dce110_stream_encoder_create(
				i, dc->ctx,
				dal_adapter_service_get_bios_parser(as),
				&stream_enc_regs[i]);
			if (pool->base.stream_enc[i] == NULL) {
				BREAK_TO_DEBUGGER();
				dm_error("DC: failed to create stream_encoder!\n");
				goto stream_enc_create_fail;
			}
		}
	}

	for (i = 0; i < num_virtual_links; i++) {
		pool->base.stream_enc[pool->base.stream_enc_count] =
			virtual_stream_encoder_create(
				dc->ctx,
				dal_adapter_service_get_bios_parser(as));

		if (pool->base.stream_enc[pool->base.stream_enc_count] == NULL) {
			BREAK_TO_DEBUGGER();
			dm_error("DC: failed to create stream_encoder!\n");
			goto stream_enc_create_fail;
		}
		pool->base.stream_enc_count++;
	}


	/* Create hardware sequencer */
	if (!dce110_hw_sequencer_construct(dc))
		goto stream_enc_create_fail;

	bw_calcs_init(&dc->bw_dceip, &dc->bw_vbios, BW_CALCS_VERSION_CARRIZO);

	bw_calcs_data_update_from_pplib(dc);

	return true;

stream_enc_create_fail:
	for (i = 0; i < pool->base.stream_enc_count; i++) {
		if (pool->base.stream_enc[i] != NULL)
			dm_free(DCE110STRENC_FROM_STRENC(pool->base.stream_enc[i]));
	}

audio_create_fail:
	for (i = 0; i < pool->base.pipe_count; i++) {
		if (pool->base.audios[i] != NULL)
			dce110_aud_destroy(&pool->base.audios[i]);
	}

controller_create_fail:
	for (i = 0; i < pool->base.pipe_count; i++) {
		if (pool->base.opps[i] != NULL)
			pool->base.opps[i]->funcs->opp_destroy(&pool->base.opps[i]);

		if (pool->base.transforms[i] != NULL)
			dce110_transform_destroy(&pool->base.transforms[i]);

		if (pool->base.ipps[i] != NULL)
			dce110_ipp_destroy(&pool->base.ipps[i]);

		if (pool->base.mis[i] != NULL) {
			dm_free(TO_DCE110_MEM_INPUT(pool->base.mis[i]));
			pool->base.mis[i] = NULL;
		}

		if (pool->base.timing_generators[i] != NULL)	{
			dm_free(DCE110TG_FROM_TG(pool->base.timing_generators[i]));
			pool->base.timing_generators[i] = NULL;
		}
	}

filter_create_fail:
	dal_irq_service_destroy(&pool->base.irqs);

irqs_create_fail:
	dal_display_clock_destroy(&pool->base.display_clock);

disp_clk_create_fail:
clk_src_create_fail:
	for (i = 0; i < pool->base.clk_src_count; i++) {
		if (pool->base.clock_sources[i] != NULL)
			dce110_clock_source_destroy(&pool->base.clock_sources[i]);
	}

	return false;
}

struct resource_pool *dce110_create_resource_pool(
	struct adapter_service *as,
	uint8_t num_virtual_links,
	struct core_dc *dc,
	struct hw_asic_id asic_id)
{
	struct dce110_resource_pool *pool =
		dm_alloc(sizeof(struct dce110_resource_pool));

	if (!pool)
		return NULL;

	if (construct(as, num_virtual_links, dc, pool, asic_id))
		return &pool->base;

	BREAK_TO_DEBUGGER();
	return NULL;
}
