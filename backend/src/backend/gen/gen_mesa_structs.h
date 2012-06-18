/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

 /*
  * Authors:
  *   Keith Whitwell <keith@tungstengraphics.com>
  */
        

#ifndef BRW_STRUCTS_H
#define BRW_STRUCTS_H


/** Number of general purpose registers (VS, WM, etc) */
#define BRW_MAX_GRF 128

/**
 * First GRF used for the MRF hack.
 *
 * On gen7, MRFs are no longer used, and contiguous GRFs are used instead.  We
 * haven't converted our compiler to be aware of this, so it asks for MRFs and
 * brw_eu_emit.c quietly converts them to be accesses of the top GRFs.  The
 * register allocators have to be careful of this to avoid corrupting the "MRF"s
 * with actual GRF allocations.
 */
#define GEN7_MRF_HACK_START 112.

/** Number of message register file registers */
#define BRW_MAX_MRF 16

/* These seem to be passed around as function args, so it works out
 * better to keep them as #defines:
 */
#define BRW_FLUSH_READ_CACHE           0x1
#define BRW_FLUSH_STATE_CACHE          0x2
#define BRW_INHIBIT_FLUSH_RENDER_CACHE 0x4
#define BRW_FLUSH_SNAPSHOT_COUNTERS    0x8

struct brw_urb_fence
{
   struct
   {
      uint32_t length:8;   
      uint32_t vs_realloc:1;   
      uint32_t gs_realloc:1;   
      uint32_t clp_realloc:1;   
      uint32_t sf_realloc:1;   
      uint32_t vfe_realloc:1;   
      uint32_t cs_realloc:1;   
      uint32_t pad:2;
      uint32_t opcode:16;   
   } header;

   struct
   {
      uint32_t vs_fence:10;  
      uint32_t gs_fence:10;  
      uint32_t clp_fence:10;  
      uint32_t pad:2;
   } bits0;

   struct
   {
      uint32_t sf_fence:10;  
      uint32_t vf_fence:10;  
      uint32_t cs_fence:11;  
      uint32_t pad:1;
   } bits1;
};

/* State structs for the various fixed function units:
 */


struct thread0
{
   uint32_t pad0:1;
   uint32_t grf_reg_count:3; 
   uint32_t pad1:2;
   uint32_t kernel_start_pointer:26; /* Offset from GENERAL_STATE_BASE */
};

struct thread1
{
   uint32_t ext_halt_exception_enable:1; 
   uint32_t sw_exception_enable:1; 
   uint32_t mask_stack_exception_enable:1; 
   uint32_t timeout_exception_enable:1; 
   uint32_t illegal_op_exception_enable:1; 
   uint32_t pad0:3;
   uint32_t depth_coef_urb_read_offset:6;	/* WM only */
   uint32_t pad1:2;
   uint32_t floating_point_mode:1; 
   uint32_t thread_priority:1; 
   uint32_t binding_table_entry_count:8; 
   uint32_t pad3:5;
   uint32_t single_program_flow:1; 
};

struct thread2
{
   uint32_t per_thread_scratch_space:4; 
   uint32_t pad0:6;
   uint32_t scratch_space_base_pointer:22; 
};

   
struct thread3
{
   uint32_t dispatch_grf_start_reg:4; 
   uint32_t urb_entry_read_offset:6; 
   uint32_t pad0:1;
   uint32_t urb_entry_read_length:6; 
   uint32_t pad1:1;
   uint32_t const_urb_entry_read_offset:6; 
   uint32_t pad2:1;
   uint32_t const_urb_entry_read_length:6; 
   uint32_t pad3:1;
};



struct brw_clip_unit_state
{
   struct thread0 thread0;
   struct
   {
      uint32_t pad0:7;
      uint32_t sw_exception_enable:1;
      uint32_t pad1:3;
      uint32_t mask_stack_exception_enable:1;
      uint32_t pad2:1;
      uint32_t illegal_op_exception_enable:1;
      uint32_t pad3:2;
      uint32_t floating_point_mode:1;
      uint32_t thread_priority:1;
      uint32_t binding_table_entry_count:8;
      uint32_t pad4:5;
      uint32_t single_program_flow:1;
   } thread1;

   struct thread2 thread2;
   struct thread3 thread3;

   struct
   {
      uint32_t pad0:9;
      uint32_t gs_output_stats:1; /* not always */
      uint32_t stats_enable:1; 
      uint32_t nr_urb_entries:7; 
      uint32_t pad1:1;
      uint32_t urb_entry_allocation_size:5; 
      uint32_t pad2:1;
      uint32_t max_threads:5; 	/* may be less */
      uint32_t pad3:2;
   } thread4;   
      
   struct
   {
      uint32_t pad0:13;
      uint32_t clip_mode:3; 
      uint32_t userclip_enable_flags:8; 
      uint32_t userclip_must_clip:1; 
      uint32_t negative_w_clip_test:1;
      uint32_t guard_band_enable:1; 
      uint32_t viewport_z_clip_enable:1; 
      uint32_t viewport_xy_clip_enable:1; 
      uint32_t vertex_position_space:1; 
      uint32_t api_mode:1; 
      uint32_t pad2:1;
   } clip5;
   
   struct
   {
      uint32_t pad0:5;
      uint32_t clipper_viewport_state_ptr:27; 
   } clip6;

   
   float viewport_xmin;  
   float viewport_xmax;  
   float viewport_ymin;  
   float viewport_ymax;  
};

struct gen6_blend_state
{
   struct {
      uint32_t dest_blend_factor:5;
      uint32_t source_blend_factor:5;
      uint32_t pad3:1;
      uint32_t blend_func:3;
      uint32_t pad2:1;
      uint32_t ia_dest_blend_factor:5;
      uint32_t ia_source_blend_factor:5;
      uint32_t pad1:1;
      uint32_t ia_blend_func:3;
      uint32_t pad0:1;
      uint32_t ia_blend_enable:1;
      uint32_t blend_enable:1;
   } blend0;

   struct {
      uint32_t post_blend_clamp_enable:1;
      uint32_t pre_blend_clamp_enable:1;
      uint32_t clamp_range:2;
      uint32_t pad0:4;
      uint32_t x_dither_offset:2;
      uint32_t y_dither_offset:2;
      uint32_t dither_enable:1;
      uint32_t alpha_test_func:3;
      uint32_t alpha_test_enable:1;
      uint32_t pad1:1;
      uint32_t logic_op_func:4;
      uint32_t logic_op_enable:1;
      uint32_t pad2:1;
      uint32_t write_disable_b:1;
      uint32_t write_disable_g:1;
      uint32_t write_disable_r:1;
      uint32_t write_disable_a:1;
      uint32_t pad3:1;
      uint32_t alpha_to_coverage_dither:1;
      uint32_t alpha_to_one:1;
      uint32_t alpha_to_coverage:1;
   } blend1;
};

struct gen6_color_calc_state
{
   struct {
      uint32_t alpha_test_format:1;
      uint32_t pad0:14;
      uint32_t round_disable:1;
      uint32_t bf_stencil_ref:8;
      uint32_t stencil_ref:8;
   } cc0;

   union {
      float alpha_ref_f;
      struct {
	 uint32_t ui:8;
	 uint32_t pad0:24;
      } alpha_ref_fi;
   } cc1;

   float constant_r;
   float constant_g;
   float constant_b;
   float constant_a;
};

struct gen6_depth_stencil_state
{
   struct {
      uint32_t pad0:3;
      uint32_t bf_stencil_pass_depth_pass_op:3;
      uint32_t bf_stencil_pass_depth_fail_op:3;
      uint32_t bf_stencil_fail_op:3;
      uint32_t bf_stencil_func:3;
      uint32_t bf_stencil_enable:1;
      uint32_t pad1:2;
      uint32_t stencil_write_enable:1;
      uint32_t stencil_pass_depth_pass_op:3;
      uint32_t stencil_pass_depth_fail_op:3;
      uint32_t stencil_fail_op:3;
      uint32_t stencil_func:3;
      uint32_t stencil_enable:1;
   } ds0;

   struct {
      uint32_t bf_stencil_write_mask:8;
      uint32_t bf_stencil_test_mask:8;
      uint32_t stencil_write_mask:8;
      uint32_t stencil_test_mask:8;
   } ds1;

   struct {
      uint32_t pad0:26;
      uint32_t depth_write_enable:1;
      uint32_t depth_test_func:3;
      uint32_t pad1:1;
      uint32_t depth_test_enable:1;
   } ds2;
};

struct brw_cc_unit_state
{
   struct
   {
      uint32_t pad0:3;
      uint32_t bf_stencil_pass_depth_pass_op:3; 
      uint32_t bf_stencil_pass_depth_fail_op:3; 
      uint32_t bf_stencil_fail_op:3; 
      uint32_t bf_stencil_func:3; 
      uint32_t bf_stencil_enable:1; 
      uint32_t pad1:2;
      uint32_t stencil_write_enable:1; 
      uint32_t stencil_pass_depth_pass_op:3; 
      uint32_t stencil_pass_depth_fail_op:3; 
      uint32_t stencil_fail_op:3; 
      uint32_t stencil_func:3; 
      uint32_t stencil_enable:1; 
   } cc0;

   
   struct
   {
      uint32_t bf_stencil_ref:8; 
      uint32_t stencil_write_mask:8; 
      uint32_t stencil_test_mask:8; 
      uint32_t stencil_ref:8; 
   } cc1;

   
   struct
   {
      uint32_t logicop_enable:1; 
      uint32_t pad0:10;
      uint32_t depth_write_enable:1; 
      uint32_t depth_test_function:3; 
      uint32_t depth_test:1; 
      uint32_t bf_stencil_write_mask:8; 
      uint32_t bf_stencil_test_mask:8; 
   } cc2;

   
   struct
   {
      uint32_t pad0:8;
      uint32_t alpha_test_func:3; 
      uint32_t alpha_test:1; 
      uint32_t blend_enable:1; 
      uint32_t ia_blend_enable:1; 
      uint32_t pad1:1;
      uint32_t alpha_test_format:1;
      uint32_t pad2:16;
   } cc3;
   
   struct
   {
      uint32_t pad0:5; 
      uint32_t cc_viewport_state_offset:27; /* Offset from GENERAL_STATE_BASE */
   } cc4;
   
   struct
   {
      uint32_t pad0:2;
      uint32_t ia_dest_blend_factor:5; 
      uint32_t ia_src_blend_factor:5; 
      uint32_t ia_blend_function:3; 
      uint32_t statistics_enable:1; 
      uint32_t logicop_func:4; 
      uint32_t pad1:11;
      uint32_t dither_enable:1; 
   } cc5;

   struct
   {
      uint32_t clamp_post_alpha_blend:1; 
      uint32_t clamp_pre_alpha_blend:1; 
      uint32_t clamp_range:2; 
      uint32_t pad0:11;
      uint32_t y_dither_offset:2; 
      uint32_t x_dither_offset:2; 
      uint32_t dest_blend_factor:5; 
      uint32_t src_blend_factor:5; 
      uint32_t blend_function:3; 
   } cc6;

   struct {
      union {
	 float f;  
	 uint8_t ub[4];
      } alpha_ref;
   } cc7;
};

struct brw_sf_unit_state
{
   struct thread0 thread0;
   struct thread1 thread1;
   struct thread2 thread2;
   struct thread3 thread3;

   struct
   {
      uint32_t pad0:10;
      uint32_t stats_enable:1; 
      uint32_t nr_urb_entries:7; 
      uint32_t pad1:1;
      uint32_t urb_entry_allocation_size:5; 
      uint32_t pad2:1;
      uint32_t max_threads:6; 
      uint32_t pad3:1;
   } thread4;   

   struct
   {
      uint32_t front_winding:1; 
      uint32_t viewport_transform:1; 
      uint32_t pad0:3;
      uint32_t sf_viewport_state_offset:27; /* Offset from GENERAL_STATE_BASE */
   } sf5;
   
   struct
   {
      uint32_t pad0:9;
      uint32_t dest_org_vbias:4; 
      uint32_t dest_org_hbias:4; 
      uint32_t scissor:1; 
      uint32_t disable_2x2_trifilter:1; 
      uint32_t disable_zero_pix_trifilter:1; 
      uint32_t point_rast_rule:2; 
      uint32_t line_endcap_aa_region_width:2; 
      uint32_t line_width:4; 
      uint32_t fast_scissor_disable:1; 
      uint32_t cull_mode:2; 
      uint32_t aa_enable:1; 
   } sf6;

   struct
   {
      uint32_t point_size:11; 
      uint32_t use_point_size_state:1; 
      uint32_t subpixel_precision:1; 
      uint32_t sprite_point:1; 
      uint32_t pad0:10;
      uint32_t aa_line_distance_mode:1;
      uint32_t trifan_pv:2; 
      uint32_t linestrip_pv:2; 
      uint32_t tristrip_pv:2; 
      uint32_t line_last_pixel_enable:1; 
   } sf7;

};

struct gen6_scissor_rect
{
   uint32_t xmin:16;
   uint32_t ymin:16;
   uint32_t xmax:16;
   uint32_t ymax:16;
};

struct brw_gs_unit_state
{
   struct thread0 thread0;
   struct thread1 thread1;
   struct thread2 thread2;
   struct thread3 thread3;

   struct
   {
      uint32_t pad0:8;
      uint32_t rendering_enable:1; /* for Ironlake */
      uint32_t pad4:1;
      uint32_t stats_enable:1; 
      uint32_t nr_urb_entries:7; 
      uint32_t pad1:1;
      uint32_t urb_entry_allocation_size:5; 
      uint32_t pad2:1;
      uint32_t max_threads:5; 
      uint32_t pad3:2;
   } thread4;   
      
   struct
   {
      uint32_t sampler_count:3; 
      uint32_t pad0:2;
      uint32_t sampler_state_pointer:27; 
   } gs5;

   
   struct
   {
      uint32_t max_vp_index:4; 
      uint32_t pad0:12;
      uint32_t svbi_post_inc_value:10;
      uint32_t pad1:1;
      uint32_t svbi_post_inc_enable:1;
      uint32_t svbi_payload:1;
      uint32_t discard_adjaceny:1;
      uint32_t reorder_enable:1; 
      uint32_t pad2:1;
   } gs6;
};


struct brw_vs_unit_state
{
   struct thread0 thread0;
   struct thread1 thread1;
   struct thread2 thread2;
   struct thread3 thread3;
   
   struct
   {
      uint32_t pad0:10;
      uint32_t stats_enable:1; 
      uint32_t nr_urb_entries:7; 
      uint32_t pad1:1;
      uint32_t urb_entry_allocation_size:5; 
      uint32_t pad2:1;
      uint32_t max_threads:6; 
      uint32_t pad3:1;
   } thread4;   

   struct
   {
      uint32_t sampler_count:3; 
      uint32_t pad0:2;
      uint32_t sampler_state_pointer:27; 
   } vs5;

   struct
   {
      uint32_t vs_enable:1; 
      uint32_t vert_cache_disable:1; 
      uint32_t pad0:30;
   } vs6;
};


struct brw_wm_unit_state
{
   struct thread0 thread0;
   struct thread1 thread1;
   struct thread2 thread2;
   struct thread3 thread3;
   
   struct {
      uint32_t stats_enable:1; 
      uint32_t depth_buffer_clear:1;
      uint32_t sampler_count:3; 
      uint32_t sampler_state_pointer:27; 
   } wm4;
   
   struct
   {
      uint32_t enable_8_pix:1; 
      uint32_t enable_16_pix:1; 
      uint32_t enable_32_pix:1; 
      uint32_t enable_con_32_pix:1;
      uint32_t enable_con_64_pix:1;
      uint32_t pad0:1;

      /* These next four bits are for Ironlake+ */
      uint32_t fast_span_coverage_enable:1;
      uint32_t depth_buffer_clear:1;
      uint32_t depth_buffer_resolve_enable:1;
      uint32_t hierarchical_depth_buffer_resolve_enable:1;

      uint32_t legacy_global_depth_bias:1; 
      uint32_t line_stipple:1; 
      uint32_t depth_offset:1; 
      uint32_t polygon_stipple:1; 
      uint32_t line_aa_region_width:2; 
      uint32_t line_endcap_aa_region_width:2; 
      uint32_t early_depth_test:1; 
      uint32_t thread_dispatch_enable:1; 
      uint32_t program_uses_depth:1; 
      uint32_t program_computes_depth:1; 
      uint32_t program_uses_killpixel:1; 
      uint32_t legacy_line_rast: 1; 
      uint32_t transposed_urb_read_enable:1; 
      uint32_t max_threads:7; 
   } wm5;
   
   float global_depth_offset_constant;  
   float global_depth_offset_scale;   
   
   /* for Ironlake only */
   struct {
      uint32_t pad0:1;
      uint32_t grf_reg_count_1:3; 
      uint32_t pad1:2;
      uint32_t kernel_start_pointer_1:26;
   } wm8;       

   struct {
      uint32_t pad0:1;
      uint32_t grf_reg_count_2:3; 
      uint32_t pad1:2;
      uint32_t kernel_start_pointer_2:26;
   } wm9;       

   struct {
      uint32_t pad0:1;
      uint32_t grf_reg_count_3:3; 
      uint32_t pad1:2;
      uint32_t kernel_start_pointer_3:26;
   } wm10;       
};

struct brw_sampler_default_color {
   float color[4];
};

struct gen5_sampler_default_color {
   uint8_t ub[4];
   float f[4];
   uint16_t hf[4];
   uint16_t us[4];
   int16_t s[4];
   uint8_t b[4];
};

struct brw_sampler_state
{
   
   struct
   {
      uint32_t shadow_function:3; 
      uint32_t lod_bias:11; 
      uint32_t min_filter:3; 
      uint32_t mag_filter:3; 
      uint32_t mip_filter:2; 
      uint32_t base_level:5; 
      uint32_t min_mag_neq:1;
      uint32_t lod_preclamp:1; 
      uint32_t default_color_mode:1; 
      uint32_t pad0:1;
      uint32_t disable:1; 
   } ss0;

   struct
   {
      uint32_t r_wrap_mode:3; 
      uint32_t t_wrap_mode:3; 
      uint32_t s_wrap_mode:3; 
      uint32_t cube_control_mode:1;
      uint32_t pad:2;
      uint32_t max_lod:10; 
      uint32_t min_lod:10; 
   } ss1;

   
   struct
   {
      uint32_t pad:5;
      uint32_t default_color_pointer:27; 
   } ss2;
   
   struct
   {
      uint32_t non_normalized_coord:1;
      uint32_t pad:12;
      uint32_t address_round:6;
      uint32_t max_aniso:3; 
      uint32_t chroma_key_mode:1; 
      uint32_t chroma_key_index:2; 
      uint32_t chroma_key_enable:1; 
      uint32_t monochrome_filter_width:3; 
      uint32_t monochrome_filter_height:3; 
   } ss3;
};

struct gen7_sampler_state
{
   struct
   {
      uint32_t aniso_algorithm:1;
      uint32_t lod_bias:13;
      uint32_t min_filter:3;
      uint32_t mag_filter:3;
      uint32_t mip_filter:2;
      uint32_t base_level:5;
      uint32_t pad1:1;
      uint32_t lod_preclamp:1;
      uint32_t default_color_mode:1;
      uint32_t pad0:1;
      uint32_t disable:1;
   } ss0;

   struct
   {
      uint32_t cube_control_mode:1;
      uint32_t shadow_function:3;
      uint32_t pad:4;
      uint32_t max_lod:12;
      uint32_t min_lod:12;
   } ss1;

   struct
   {
      uint32_t pad:5;
      uint32_t default_color_pointer:27;
   } ss2;

   struct
   {
      uint32_t r_wrap_mode:3;
      uint32_t t_wrap_mode:3;
      uint32_t s_wrap_mode:3;
      uint32_t pad:1;
      uint32_t non_normalized_coord:1;
      uint32_t trilinear_quality:2;
      uint32_t address_round:6;
      uint32_t max_aniso:3;
      uint32_t chroma_key_mode:1;
      uint32_t chroma_key_index:2;
      uint32_t chroma_key_enable:1;
      uint32_t pad0:6;
   } ss3;
};

struct brw_clipper_viewport
{
   float xmin;  
   float xmax;  
   float ymin;  
   float ymax;  
};

struct brw_cc_viewport
{
   float min_depth;  
   float max_depth;  
};

struct brw_sf_viewport
{
   struct {
      float m00;  
      float m11;  
      float m22;  
      float m30;  
      float m31;  
      float m32;  
   } viewport;

   /* scissor coordinates are inclusive */
   struct {
      int16_t xmin;
      int16_t ymin;
      int16_t xmax;
      int16_t ymax;
   } scissor;
};

struct gen6_sf_viewport {
   float m00;
   float m11;
   float m22;
   float m30;
   float m31;
   float m32;
};

struct gen7_sf_clip_viewport {
   struct {
      float m00;
      float m11;
      float m22;
      float m30;
      float m31;
      float m32;
   } viewport;

   uint32_t pad0[2];

   struct {
      float xmin;
      float xmax;
      float ymin;
      float ymax;
   } guardband;

   float pad1[4];
};

/* volume 5c Shared Functions - 1.13.4.1.2 */
struct gen7_surface_state
{
   struct {
      uint32_t cube_pos_z:1;
      uint32_t cube_neg_z:1;
      uint32_t cube_pos_y:1;
      uint32_t cube_neg_y:1;
      uint32_t cube_pos_x:1;
      uint32_t cube_neg_x:1;
      uint32_t pad2:2;
      uint32_t render_cache_read_write:1;
      uint32_t pad1:1;
      uint32_t surface_array_spacing:1;
      uint32_t vert_line_stride_ofs:1;
      uint32_t vert_line_stride:1;
      uint32_t tile_walk:1;
      uint32_t tiled_surface:1;
      uint32_t horizontal_alignment:1;
      uint32_t vertical_alignment:2;
      uint32_t surface_format:9;     /**< BRW_SURFACEFORMAT_x */
      uint32_t pad0:1;
      uint32_t is_array:1;
      uint32_t surface_type:3;       /**< BRW_SURFACE_1D/2D/3D/CUBE */
   } ss0;

   struct {
      uint32_t base_addr;
   } ss1;

   struct {
      uint32_t width:14;
      uint32_t pad1:2;
      uint32_t height:14;
      uint32_t pad0:2;
   } ss2;

   struct {
      uint32_t pitch:18;
      uint32_t pad:3;
      uint32_t depth:11;
   } ss3;

   struct {
      uint32_t multisample_position_palette_index:3;
      uint32_t num_multisamples:3;
      uint32_t multisampled_surface_storage_format:1;
      uint32_t render_target_view_extent:11;
      uint32_t min_array_elt:11;
      uint32_t rotation:2;
      uint32_t pad0:1;
   } ss4;

   struct {
      uint32_t mip_count:4;
      uint32_t min_lod:4;
      uint32_t pad1:12;
      uint32_t y_offset:4;
      uint32_t pad0:1;
      uint32_t x_offset:7;
   } ss5;

   struct {
      uint32_t pad; /* Multisample Control Surface stuff */
   } ss6;

   struct {
      uint32_t resource_min_lod:12;

      /* Only on Haswell */
      uint32_t pad0:4;
      uint32_t shader_chanel_select_a:3;
      uint32_t shader_chanel_select_b:3;
      uint32_t shader_chanel_select_g:3;
      uint32_t shader_chanel_select_r:3;

      uint32_t alpha_clear_color:1;
      uint32_t blue_clear_color:1;
      uint32_t green_clear_color:1;
      uint32_t red_clear_color:1;
   } ss7;
};


struct brw_vertex_element_state
{
   struct
   {
      uint32_t src_offset:11; 
      uint32_t pad:5;
      uint32_t src_format:9; 
      uint32_t pad0:1;
      uint32_t valid:1; 
      uint32_t vertex_buffer_index:5; 
   } ve0;
   
   struct
   {
      uint32_t dst_offset:8; 
      uint32_t pad:8;
      uint32_t vfcomponent3:4; 
      uint32_t vfcomponent2:4; 
      uint32_t vfcomponent1:4; 
      uint32_t vfcomponent0:4; 
   } ve1;
};

struct brw_urb_immediate {
   uint32_t opcode:4;
   uint32_t offset:6;
   uint32_t swizzle_control:2; 
   uint32_t pad:1;
   uint32_t allocate:1;
   uint32_t used:1;
   uint32_t complete:1;
   uint32_t response_length:4;
   uint32_t msg_length:4;
   uint32_t msg_target:4;
   uint32_t pad1:3;
   uint32_t end_of_thread:1;
};

/* Instruction format for the execution units:
 */
 
struct brw_instruction
{
   struct 
   {
      uint32_t opcode:7;
      uint32_t pad:1;
      uint32_t access_mode:1;
      uint32_t mask_control:1;
      uint32_t dependency_control:2;
      uint32_t compression_control:2; /* gen6: quater control */
      uint32_t thread_control:2;
      uint32_t predicate_control:4;
      uint32_t predicate_inverse:1;
      uint32_t execution_size:3;
      /**
       * Conditional Modifier for most instructions.  On Gen6+, this is also
       * used for the SEND instruction's Message Target/SFID.
       */
      uint32_t destreg__conditionalmod:4;
      uint32_t acc_wr_control:1;
      uint32_t cmpt_control:1;
      uint32_t debug_control:1;
      uint32_t saturate:1;
   } header;

   union {
      struct
      {
	 uint32_t dest_reg_file:2;
	 uint32_t dest_reg_type:3;
	 uint32_t src0_reg_file:2;
	 uint32_t src0_reg_type:3;
	 uint32_t src1_reg_file:2;
	 uint32_t src1_reg_type:3;
	 uint32_t pad:1;
	 uint32_t dest_subreg_nr:5;
	 uint32_t dest_reg_nr:8;
	 uint32_t dest_horiz_stride:2;
	 uint32_t dest_address_mode:1;
      } da1;

      struct
      {
	 uint32_t dest_reg_file:2;
	 uint32_t dest_reg_type:3;
	 uint32_t src0_reg_file:2;
	 uint32_t src0_reg_type:3;
	 uint32_t src1_reg_file:2;        /* 0x00000c00 */
	 uint32_t src1_reg_type:3;        /* 0x00007000 */
	 uint32_t pad:1;
	 int32_t dest_indirect_offset:10;	/* offset against the deref'd address reg */
	 uint32_t dest_subreg_nr:3; /* subnr for the address reg a0.x */
	 uint32_t dest_horiz_stride:2;
	 uint32_t dest_address_mode:1;
      } ia1;

      struct
      {
	 uint32_t dest_reg_file:2;
	 uint32_t dest_reg_type:3;
	 uint32_t src0_reg_file:2;
	 uint32_t src0_reg_type:3;
	 uint32_t src1_reg_file:2;
	 uint32_t src1_reg_type:3;
	 uint32_t pad:1;
	 uint32_t dest_writemask:4;
	 uint32_t dest_subreg_nr:1;
	 uint32_t dest_reg_nr:8;
	 uint32_t dest_horiz_stride:2;
	 uint32_t dest_address_mode:1;
      } da16;

      struct
      {
	 uint32_t dest_reg_file:2;
	 uint32_t dest_reg_type:3;
	 uint32_t src0_reg_file:2;
	 uint32_t src0_reg_type:3;
	 uint32_t pad0:6;
	 uint32_t dest_writemask:4;
	 int32_t dest_indirect_offset:6;
	 uint32_t dest_subreg_nr:3;
	 uint32_t dest_horiz_stride:2;
	 uint32_t dest_address_mode:1;
      } ia16;

      struct {
	 uint32_t dest_reg_file:2;
	 uint32_t dest_reg_type:3;
	 uint32_t src0_reg_file:2;
	 uint32_t src0_reg_type:3;
	 uint32_t src1_reg_file:2;
	 uint32_t src1_reg_type:3;
	 uint32_t pad:1;

	 int32_t jump_count:16;
      } branch_gen6;

      struct {
	 uint32_t dest_reg_file:1;
	 uint32_t flag_subreg_num:1;
	 uint32_t pad0:2;
	 uint32_t src0_abs:1;
	 uint32_t src0_negate:1;
	 uint32_t src1_abs:1;
	 uint32_t src1_negate:1;
	 uint32_t src2_abs:1;
	 uint32_t src2_negate:1;
	 uint32_t pad1:7;
	 uint32_t dest_writemask:4;
	 uint32_t dest_subreg_nr:3;
	 uint32_t dest_reg_nr:8;
      } da3src;
   } bits1;


   union {
      struct
      {
	 uint32_t src0_subreg_nr:5;
	 uint32_t src0_reg_nr:8;
	 uint32_t src0_abs:1;
	 uint32_t src0_negate:1;
	 uint32_t src0_address_mode:1;
	 uint32_t src0_horiz_stride:2;
	 uint32_t src0_width:3;
	 uint32_t src0_vert_stride:4;
	 uint32_t flag_reg_nr:1;
	 uint32_t pad:6;
      } da1;

      struct
      {
	 int32_t src0_indirect_offset:10;
	 uint32_t src0_subreg_nr:3;
	 uint32_t src0_abs:1;
	 uint32_t src0_negate:1;
	 uint32_t src0_address_mode:1;
	 uint32_t src0_horiz_stride:2;
	 uint32_t src0_width:3;
	 uint32_t src0_vert_stride:4;
	 uint32_t flag_reg_nr:1;
	 uint32_t pad:6;	
      } ia1;

      struct
      {
	 uint32_t src0_swz_x:2;
	 uint32_t src0_swz_y:2;
	 uint32_t src0_subreg_nr:1;
	 uint32_t src0_reg_nr:8;
	 uint32_t src0_abs:1;
	 uint32_t src0_negate:1;
	 uint32_t src0_address_mode:1;
	 uint32_t src0_swz_z:2;
	 uint32_t src0_swz_w:2;
	 uint32_t pad0:1;
	 uint32_t src0_vert_stride:4;
	 uint32_t flag_reg_nr:1;
	 uint32_t pad1:6;
      } da16;

      struct
      {
	 uint32_t src0_swz_x:2;
	 uint32_t src0_swz_y:2;
	 int32_t src0_indirect_offset:6;
	 uint32_t src0_subreg_nr:3;
	 uint32_t src0_abs:1;
	 uint32_t src0_negate:1;
	 uint32_t src0_address_mode:1;
	 uint32_t src0_swz_z:2;
	 uint32_t src0_swz_w:2;
	 uint32_t pad0:1;
	 uint32_t src0_vert_stride:4;
	 uint32_t flag_reg_nr:1;
	 uint32_t pad1:6;
      } ia16;

      /* Extended Message Descriptor for Ironlake (Gen5) SEND instruction.
       *
       * Does not apply to Gen6+.  The SFID/message target moved to bits
       * 27:24 of the header (destreg__conditionalmod); EOT is in bits3.
       */
       struct 
       {
           uint32_t pad:26;
           uint32_t end_of_thread:1;
           uint32_t pad1:1;
           uint32_t sfid:4;
       } send_gen5;  /* for Ironlake only */

      struct {
	 uint32_t src0_rep_ctrl:1;
	 uint32_t src0_swizzle:8;
	 uint32_t src0_subreg_nr:3;
	 uint32_t src0_reg_nr:8;
	 uint32_t pad0:1;
	 uint32_t src1_rep_ctrl:1;
	 uint32_t src1_swizzle:8;
	 uint32_t src1_subreg_nr_low:2;
      } da3src;
   } bits2;

   union
   {
      struct
      {
	 uint32_t src1_subreg_nr:5;
	 uint32_t src1_reg_nr:8;
	 uint32_t src1_abs:1;
	 uint32_t src1_negate:1;
	 uint32_t src1_address_mode:1;
	 uint32_t src1_horiz_stride:2;
	 uint32_t src1_width:3;
	 uint32_t src1_vert_stride:4;
	 uint32_t pad0:7;
      } da1;

      struct
      {
	 uint32_t src1_swz_x:2;
	 uint32_t src1_swz_y:2;
	 uint32_t src1_subreg_nr:1;
	 uint32_t src1_reg_nr:8;
	 uint32_t src1_abs:1;
	 uint32_t src1_negate:1;
	 uint32_t src1_address_mode:1;
	 uint32_t src1_swz_z:2;
	 uint32_t src1_swz_w:2;
	 uint32_t pad1:1;
	 uint32_t src1_vert_stride:4;
	 uint32_t pad2:7;
      } da16;

      struct
      {
	 int32_t  src1_indirect_offset:10;
	 uint32_t src1_subreg_nr:3;
	 uint32_t src1_abs:1;
	 uint32_t src1_negate:1;
	 uint32_t src1_address_mode:1;
	 uint32_t src1_horiz_stride:2;
	 uint32_t src1_width:3;
	 uint32_t src1_vert_stride:4;
	 uint32_t flag_reg_nr:1;
	 uint32_t pad1:6;	
      } ia1;

      struct
      {
	 uint32_t src1_swz_x:2;
	 uint32_t src1_swz_y:2;
	 int32_t  src1_indirect_offset:6;
	 uint32_t src1_subreg_nr:3;
	 uint32_t src1_abs:1;
	 uint32_t src1_negate:1;
	 uint32_t pad0:1;
	 uint32_t src1_swz_z:2;
	 uint32_t src1_swz_w:2;
	 uint32_t pad1:1;
	 uint32_t src1_vert_stride:4;
	 uint32_t flag_reg_nr:1;
	 uint32_t pad2:6;
      } ia16;


      struct
      {
	 int32_t  jump_count:16;	/* note: signed */
	 uint32_t  pop_count:4;
	 uint32_t  pad0:12;
      } if_else;

      /* This is also used for gen7 IF/ELSE instructions */
      struct
      {
	 /* Signed jump distance to the ip to jump to if all channels
	  * are disabled after the break or continue.  It should point
	  * to the end of the innermost control flow block, as that's
	  * where some channel could get re-enabled.
	  */
	 int jip:16;

	 /* Signed jump distance to the location to resume execution
	  * of this channel if it's enabled for the break or continue.
	  */
	 int uip:16;
      } break_cont;

      /**
       * \defgroup SEND instructions / Message Descriptors
       *
       * @{
       */

      /**
       * Generic Message Descriptor for Gen4 SEND instructions.  The structs
       * below expand function_control to something specific for their
       * message.  Due to struct packing issues, they duplicate these bits.
       *
       * See the G45 PRM, Volume 4, Table 14-15.
       */
      struct {
	 uint32_t function_control:16;
	 uint32_t response_length:4;
	 uint32_t msg_length:4;
	 uint32_t msg_target:4;
	 uint32_t pad1:3;
	 uint32_t end_of_thread:1;
      } generic;

      /**
       * Generic Message Descriptor for Gen5-7 SEND instructions.
       *
       * See the Sandybridge PRM, Volume 2 Part 2, Table 8-15.  (Sadly, most
       * of the information on the SEND instruction is missing from the public
       * Ironlake PRM.)
       *
       * The table claims that bit 31 is reserved/MBZ on Gen6+, but it lies.
       * According to the SEND instruction description:
       * "The MSb of the message description, the EOT field, always comes from
       *  bit 127 of the instruction word"...which is bit 31 of this field.
       */
      struct {
	 uint32_t function_control:19;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } generic_gen5;

      /** G45 PRM, Volume 4, Section 6.1.1.1 */
      struct {
	 uint32_t function:4;
	 uint32_t int_type:1;
	 uint32_t precision:1;
	 uint32_t saturate:1;
	 uint32_t data_type:1;
	 uint32_t pad0:8;
	 uint32_t response_length:4;
	 uint32_t msg_length:4;
	 uint32_t msg_target:4;
	 uint32_t pad1:3;
	 uint32_t end_of_thread:1;
      } math;

      /** Ironlake PRM, Volume 4 Part 1, Section 6.1.1.1 */
      struct {
	 uint32_t function:4;
	 uint32_t int_type:1;
	 uint32_t precision:1;
	 uint32_t saturate:1;
	 uint32_t data_type:1;
	 uint32_t snapshot:1;
	 uint32_t pad0:10;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } math_gen5;

      /** G45 PRM, Volume 4, Section 4.8.1.1.1 [DevBW] and [DevCL] */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t sampler:4;
	 uint32_t return_format:2; 
	 uint32_t msg_type:2;   
	 uint32_t response_length:4;
	 uint32_t msg_length:4;
	 uint32_t msg_target:4;
	 uint32_t pad1:3;
	 uint32_t end_of_thread:1;
      } sampler;

      /** G45 PRM, Volume 4, Section 4.8.1.1.2 [DevCTG] */
      struct {
         uint32_t binding_table_index:8;
         uint32_t sampler:4;
         uint32_t msg_type:4;
         uint32_t response_length:4;
         uint32_t msg_length:4;
         uint32_t msg_target:4;
         uint32_t pad1:3;
         uint32_t end_of_thread:1;
      } sampler_g4x;

      /** Ironlake PRM, Volume 4 Part 1, Section 4.11.1.1.3 */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t sampler:4;
	 uint32_t msg_type:4;
	 uint32_t simd_mode:2;
	 uint32_t pad0:1;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } sampler_gen5;

      struct {
	 uint32_t binding_table_index:8;
	 uint32_t sampler:4;
	 uint32_t msg_type:5;
	 uint32_t simd_mode:2;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } sampler_gen7;

      struct brw_urb_immediate urb;

      struct {
	 uint32_t opcode:4;
	 uint32_t offset:6;
	 uint32_t swizzle_control:2; 
	 uint32_t pad:1;
	 uint32_t allocate:1;
	 uint32_t used:1;
	 uint32_t complete:1;
	 uint32_t pad0:3;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } urb_gen5;

      struct {
	 uint32_t opcode:3;
	 uint32_t offset:11;
	 uint32_t swizzle_control:1;
	 uint32_t complete:1;
	 uint32_t per_slot_offset:1;
	 uint32_t pad0:2;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } urb_gen7;

      /** 965 PRM, Volume 4, Section 5.10.1.1: Message Descriptor */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:4;  
	 uint32_t msg_type:2;  
	 uint32_t target_cache:2;    
	 uint32_t response_length:4;
	 uint32_t msg_length:4;
	 uint32_t msg_target:4;
	 uint32_t pad1:3;
	 uint32_t end_of_thread:1;
      } dp_read;

      /** G45 PRM, Volume 4, Section 5.10.1.1.2 */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:3;
	 uint32_t msg_type:3;
	 uint32_t target_cache:2;
	 uint32_t response_length:4;
	 uint32_t msg_length:4;
	 uint32_t msg_target:4;
	 uint32_t pad1:3;
	 uint32_t end_of_thread:1;
      } dp_read_g4x;

      /** Ironlake PRM, Volume 4 Part 1, Section 5.10.2.1.2. */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:3;  
	 uint32_t msg_type:3;  
	 uint32_t target_cache:2;    
	 uint32_t pad0:3;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } dp_read_gen5;

      /** G45 PRM, Volume 4, Section 5.10.1.1.2.  For both Gen4 and G45. */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:3;
	 uint32_t last_render_target:1;
	 uint32_t msg_type:3;    
	 uint32_t send_commit_msg:1;
	 uint32_t response_length:4;
	 uint32_t msg_length:4;
	 uint32_t msg_target:4;
	 uint32_t pad1:3;
	 uint32_t end_of_thread:1;
      } dp_write;

      /** Ironlake PRM, Volume 4 Part 1, Section 5.10.2.1.2. */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:3;
	 uint32_t last_render_target:1;
	 uint32_t msg_type:3;    
	 uint32_t send_commit_msg:1;
	 uint32_t pad0:3;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } dp_write_gen5;

      /**
       * Message for the Sandybridge Sampler Cache or Constant Cache Data Port.
       *
       * See the Sandybridge PRM, Volume 4 Part 1, Section 3.9.2.1.1.
       **/
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:5;
	 uint32_t msg_type:3;
	 uint32_t pad0:3;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } gen6_dp_sampler_const_cache;

      /**
       * Message for the Sandybridge Render Cache Data Port.
       *
       * Most fields are defined in the Sandybridge PRM, Volume 4 Part 1,
       * Section 3.9.2.1.1: Message Descriptor.
       *
       * "Slot Group Select" and "Last Render Target" are part of the
       * 5-bit message control for Render Target Write messages.  See
       * Section 3.9.9.2.1 of the same volume.
       */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:3;
	 uint32_t slot_group_select:1;
	 uint32_t last_render_target:1;
	 uint32_t msg_type:4;
	 uint32_t send_commit_msg:1;
	 uint32_t pad0:1;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad1:2;
	 uint32_t end_of_thread:1;
      } gen6_dp;

      /**
       * Message for any of the Gen7 Data Port caches.
       *
       * Most fields are defined in BSpec volume 5c.2 Data Port / Messages /
       * Data Port Messages / Message Descriptor.  Once again, "Slot Group
       * Select" and "Last Render Target" are part of the 6-bit message
       * control for Render Target Writes.
       */
      struct {
	 uint32_t binding_table_index:8;
	 uint32_t msg_control:3;
	 uint32_t slot_group_select:1;
	 uint32_t last_render_target:1;
	 uint32_t msg_control_pad:1;
	 uint32_t msg_type:4;
	 uint32_t pad1:1;
	 uint32_t header_present:1;
	 uint32_t response_length:5;
	 uint32_t msg_length:4;
	 uint32_t pad2:2;
	 uint32_t end_of_thread:1;
      } gen7_dp;
      /** @} */

      struct {
	 uint32_t src1_subreg_nr_high:1;
	 uint32_t src1_reg_nr:8;
	 uint32_t pad0:1;
	 uint32_t src2_rep_ctrl:1;
	 uint32_t src2_swizzle:8;
	 uint32_t src2_subreg_nr:3;
	 uint32_t src2_reg_nr:8;
	 uint32_t pad1:2;
      } da3src;

      int32_t d;
      uint32_t ud;
      float f;
   } bits3;
};


#endif
