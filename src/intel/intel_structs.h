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
 * Copyright 2009 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#ifndef __INTEL_STRUCTS_H__
#define __INTEL_STRUCTS_H__

#include <stdint.h>

typedef struct gen6_interface_descriptor
{
  struct {
    uint32_t pad6:6;
    uint32_t kernel_start_pointer:26;
  } desc0;

  struct {
    uint32_t pad:7;
    uint32_t software_exception:1;
    uint32_t pad2:3;
    uint32_t maskstack_exception:1;
    uint32_t pad3:1;
    uint32_t illegal_opcode_exception:1;
    uint32_t pad4:2;
    uint32_t floating_point_mode:1;
    uint32_t thread_priority:1;
    uint32_t single_program_flow:1;
    uint32_t pad5:1;
    uint32_t pad6:6;
    uint32_t pad7:6;
  } desc1;

  struct {
    uint32_t pad:2;
    uint32_t sampler_count:3;
    uint32_t sampler_state_pointer:27;
  } desc2;

  struct {
    uint32_t binding_table_entry_count:5;  /* prefetch entries only */
    uint32_t binding_table_pointer:27;     /* 11 bit only on IVB+ */
  } desc3;

  struct {
    uint32_t curbe_read_offset:16;         /* in GRFs */
    uint32_t curbe_read_len:16;            /* in GRFs */
  } desc4;

  struct {
    uint32_t group_threads_num:8;        /* 0..64, 0 - no barrier use */
    uint32_t barrier_return_byte:8;
    uint32_t slm_sz:5;                   /* 0..16 - 0K..64K */
    uint32_t barrier_enable:1;
    uint32_t rounding_mode:2;
    uint32_t barrier_return_grf_offset:8;
  } desc5;

  uint32_t desc6; /* unused */
  uint32_t desc7; /* unused */
} gen6_interface_descriptor_t;

typedef struct gen6_surface_state
{
  struct {
    uint32_t cube_pos_z:1;
    uint32_t cube_neg_z:1;
    uint32_t cube_pos_y:1;
    uint32_t cube_neg_y:1;
    uint32_t cube_pos_x:1;
    uint32_t cube_neg_x:1;
    uint32_t pad:2;
    uint32_t render_cache_read_mode:1;
    uint32_t cube_map_corner_mode:1;
    uint32_t mipmap_layout_mode:1;
    uint32_t vert_line_stride_ofs:1;
    uint32_t vert_line_stride:1;
    uint32_t color_blend:1;
    uint32_t writedisable_blue:1;
    uint32_t writedisable_green:1;
    uint32_t writedisable_red:1;
    uint32_t writedisable_alpha:1;
    uint32_t surface_format:9;
    uint32_t data_return_format:1;
    uint32_t pad0:1;
    uint32_t surface_type:3;
  } ss0;

  struct {
    uint32_t base_addr;
  } ss1;

  struct {
    uint32_t render_target_rotation:2;
    uint32_t mip_count:4;
    uint32_t width:13;
    uint32_t height:13;
  } ss2;

  struct {
    uint32_t tile_walk:1;
    uint32_t tiled_surface:1;
    uint32_t pad:1;
    uint32_t pitch:18;
    uint32_t depth:11;
  } ss3;

  struct {
    uint32_t multisample_pos_index:3;
    uint32_t pad:1;
    uint32_t multisample_count:3;
    uint32_t pad1:1;
    uint32_t rt_view_extent:9;
    uint32_t min_array_elt:11;
    uint32_t min_lod:4;
  } ss4;

  struct {
    uint32_t pad:16;
    uint32_t cache_control:2;  /* different values for GT and IVB */
    uint32_t gfdt:1;           /* allows selective flushing of LLC (e.g. for scanout) */
    uint32_t encrypted_data:1;
    uint32_t y_offset:4;
    uint32_t vertical_alignment:1;
    uint32_t x_offset:7;
  } ss5;

  uint32_t ss6; /* unused */
  uint32_t ss7; /* unused */
} gen6_surface_state_t;

typedef struct gen7_surface_state
{
  struct {
    uint32_t cube_pos_z:1;
    uint32_t cube_neg_z:1;
    uint32_t cube_pos_y:1;
    uint32_t cube_neg_y:1;
    uint32_t cube_pos_x:1;
    uint32_t cube_neg_x:1;
    uint32_t media_boundary_pixel_mode:2;
    uint32_t render_cache_rw_mode:1;
    uint32_t pad1:1;
    uint32_t surface_array_spacing:1;
    uint32_t vertical_line_stride_offset:1;
    uint32_t vertical_line_stride:1;
    uint32_t tile_walk:1;
    uint32_t tiled_surface:1;
    uint32_t horizontal_alignment:1;
    uint32_t vertical_alignment:2;
    uint32_t surface_format:9;
    uint32_t pad0:1;
    uint32_t surface_array:1;
    uint32_t surface_type:3;
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
    uint32_t pad0:3;
    uint32_t depth:11;
  } ss3;

  union {
    struct {
      uint32_t mulsample_pal_idx:3;
      uint32_t numer_mulsample:3;
      uint32_t mss_fmt:1;
      uint32_t rt_view_extent:11;
      uint32_t min_array_element:11;
      uint32_t rt_rotate:2;
      uint32_t pad0:1;
    } not_str_buf;
  } ss4;

  struct {
    uint32_t mip_count:4;
    uint32_t surface_min_load:4;
    uint32_t pad2:6;
    uint32_t coherence_type:1;
    uint32_t stateless_force_write_thru:1;
    uint32_t cache_control:4;
    uint32_t y_offset:4;
    uint32_t pad0:1;
    uint32_t x_offset:7;
  } ss5;

  uint32_t ss6; /* unused */

  struct {
    uint32_t min_lod:12;
    uint32_t pad0:4;
    uint32_t shader_a:3;
    uint32_t shader_b:3;
    uint32_t shader_g:3;
    uint32_t shader_r:3;
    uint32_t pad1:4;
  } ss7;
} gen7_surface_state_t;

STATIC_ASSERT(sizeof(gen6_surface_state_t) == sizeof(gen7_surface_state_t));
static const size_t surface_state_sz = sizeof(gen6_surface_state_t);

typedef struct gen6_vfe_state_inline
{
  struct {
    uint32_t per_thread_scratch_space:4;
    uint32_t pad3:3;
    uint32_t extend_vfe_state_present:1;
    uint32_t pad2:2;
    uint32_t scratch_base:22;
  } vfe0;

  struct {
    uint32_t debug_counter_control:2;
    uint32_t gpgpu_mode:1;          /* 0 for SNB!!! */
    uint32_t gateway_mmio_access:2;
    uint32_t fast_preempt:1;
    uint32_t bypass_gateway_ctl:1;  /* 0 - legacy, 1 - no open/close */
    uint32_t reset_gateway_timer:1;
    uint32_t urb_entries:8;
    uint32_t max_threads:16;
  } vfe1;

  struct {
    uint32_t pad8:8;
    uint32_t debug_object_id:24;
  } vfe2;

  struct {
    uint32_t curbe_size:16; /* in GRFs */
    uint32_t urb_size:16;  /* in GRFs */
  } vfe3;

  struct {
    uint32_t scoreboard_mask:32;  /* 1 - enable the corresponding dependency */
  } vfe4;

  struct {
    uint32_t scoreboard0_dx:4;
    uint32_t scoreboard0_dy:4;
    uint32_t scoreboard1_dx:4;
    uint32_t scoreboard1_dy:4;
    uint32_t scoreboard2_dx:4;
    uint32_t scoreboard2_dy:4;
    uint32_t scoreboard3_dx:4;
    uint32_t scoreboard3_dy:4;
  } vfe5;

  struct {
    uint32_t scoreboard4_dx:4;
    uint32_t scoreboard4_dy:4;
    uint32_t scoreboard5_dx:4;
    uint32_t scoreboard5_dy:4;
    uint32_t scoreboard6_dx:4;
    uint32_t scoreboard6_dy:4;
    uint32_t scoreboard7_dx:4;
    uint32_t scoreboard7_dy:4;
  } vfe6;
} gen6_vfe_state_inline_t;

typedef struct gen6_pipe_control
{
  struct {
    uint32_t length : BITFIELD_RANGE(0, 7);
    uint32_t reserved : BITFIELD_RANGE(8, 15);
    uint32_t instruction_subopcode : BITFIELD_RANGE(16, 23);
    uint32_t instruction_opcode : BITFIELD_RANGE(24, 26);
    uint32_t instruction_pipeline : BITFIELD_RANGE(27, 28);
    uint32_t instruction_type : BITFIELD_RANGE(29, 31);
  } dw0;

  struct {
    uint32_t depth_cache_flush_enable : BITFIELD_BIT(0);
    uint32_t stall_at_pixel_scoreboard : BITFIELD_BIT(1);
    uint32_t state_cache_invalidation_enable : BITFIELD_BIT(2);
    uint32_t constant_cache_invalidation_enable : BITFIELD_BIT(3);
    uint32_t vf_cache_invalidation_enable : BITFIELD_BIT(4);
    uint32_t dc_flush_enable : BITFIELD_BIT(5);
    uint32_t protected_memory_app_id : BITFIELD_BIT(6);
    uint32_t pipe_control_flush_enable : BITFIELD_BIT(7);
    uint32_t notify_enable : BITFIELD_BIT(8);
    uint32_t indirect_state_pointers_disable : BITFIELD_BIT(9);
    uint32_t texture_cache_invalidation_enable : BITFIELD_BIT(10);
    uint32_t instruction_cache_invalidate_enable : BITFIELD_BIT(11);
    uint32_t render_target_cache_flush_enable : BITFIELD_BIT(12);
    uint32_t depth_stall_enable : BITFIELD_BIT(13);
    uint32_t post_sync_operation : BITFIELD_RANGE(14, 15);
    uint32_t generic_media_state_clear : BITFIELD_BIT(16);
    uint32_t synchronize_gfdt_surface : BITFIELD_BIT(17);
    uint32_t tlb_invalidate : BITFIELD_BIT(18);
    uint32_t global_snapshot_count_reset : BITFIELD_BIT(19);
    uint32_t cs_stall : BITFIELD_BIT(20);
    uint32_t store_data_index : BITFIELD_BIT(21);
    uint32_t protected_memory_enable : BITFIELD_BIT(22);
    uint32_t reserved : BITFIELD_RANGE(23, 31);
  } dw1;

  struct {
    uint32_t reserved : BITFIELD_RANGE(0, 1);
    uint32_t destination_address_type : BITFIELD_BIT(2);
    uint32_t address : BITFIELD_RANGE(3, 31);
  } dw2;

  struct {
    uint32_t data;
  } dw3;

  struct {
    uint32_t data;
  } dw4;
} gen6_pipe_control_t;

typedef struct gen6_sampler_state
{
  struct {
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

  struct {
    uint32_t r_wrap_mode:3; 
    uint32_t t_wrap_mode:3; 
    uint32_t s_wrap_mode:3; 
    uint32_t cube_control_mode:1;
    uint32_t pad:2;
    uint32_t max_lod:10; 
    uint32_t min_lod:10; 
  } ss1;

  struct {
    uint32_t pad:5;
    uint32_t default_color_pointer:27; 
  } ss2;

  struct {
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
} gen6_sampler_state_t;

typedef struct gen7_sampler_border_color {
    float r,g,b,a;
} gen7_sampler_border_color_t;

typedef struct gen7_sampler_state
{
  struct {
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

  struct {
    uint32_t cube_control_mode:1;
    uint32_t shadow_function:3;
    uint32_t pad:4;
    uint32_t max_lod:12;
    uint32_t min_lod:12;
  } ss1;

  struct {
    uint32_t pad:5;
    uint32_t default_color_pointer:27;
  } ss2;

  struct {
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
} gen7_sampler_state_t;

STATIC_ASSERT(sizeof(gen6_sampler_state_t) == sizeof(gen7_sampler_state_t));

#undef BITFIELD_BIT
#undef BITFIELD_RANGE

#endif /* __INTEL_STRUCTS_H__ */

