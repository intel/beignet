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

#ifndef __GENX_STRUCTS_H__
#define __GENX_STRUCTS_H__

#include <stdint.h>

struct i965_vfe_state 
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
    uint32_t children_present:1;
    uint32_t vfe_mode:4;
    uint32_t pad2:2;
    uint32_t num_urb_entries:7;
    uint32_t urb_entry_alloc_size:9;
    uint32_t max_threads:7;
  } vfe1;

  struct {
    uint32_t pad4:4;
    uint32_t interface_descriptor_base:28;
  } vfe2;
};

struct i965_vfe_state_ex 
{
  struct {
    uint32_t pad:8;
    uint32_t obj_id:24;
  } vfex0;

  struct {
    uint32_t residual_grf_offset:5;
    uint32_t pad0:3;
    uint32_t weight_grf_offset:5;
    uint32_t pad1:3;
    uint32_t residual_data_offset:8;
    uint32_t sub_field_present_flag:2;
    uint32_t residual_data_fix_offset:1;
    uint32_t pad2:5;
  }vfex1;

  struct {
    uint32_t remap_index_0:4;
    uint32_t remap_index_1:4;
    uint32_t remap_index_2:4;
    uint32_t remap_index_3:4;
    uint32_t remap_index_4:4;
    uint32_t remap_index_5:4;
    uint32_t remap_index_6:4;
    uint32_t remap_index_7:4;
  }remap_table0;

  struct {
    uint32_t remap_index_8:4;
    uint32_t remap_index_9:4;
    uint32_t remap_index_10:4;
    uint32_t remap_index_11:4;
    uint32_t remap_index_12:4;
    uint32_t remap_index_13:4;
    uint32_t remap_index_14:4;
    uint32_t remap_index_15:4;
  } remap_table1;

  struct {
    uint32_t scoreboard_mask:8;
    uint32_t pad:22;
    uint32_t type:1;
    uint32_t enable:1;
  } scoreboard0;

  struct {
    uint32_t ignore;
  } scoreboard1;

  struct {
    uint32_t ignore;
  } scoreboard2;

  uint32_t pad;
};

struct i965_vld_state 
{
  struct {
    uint32_t pad6:6;
    uint32_t scan_order:1;
    uint32_t intra_vlc_format:1;
    uint32_t quantizer_scale_type:1;
    uint32_t concealment_motion_vector:1;
    uint32_t frame_predict_frame_dct:1;
    uint32_t top_field_first:1;
    uint32_t picture_structure:2;
    uint32_t intra_dc_precision:2;
    uint32_t f_code_0_0:4;
    uint32_t f_code_0_1:4;
    uint32_t f_code_1_0:4;
    uint32_t f_code_1_1:4;
  } vld0;

  struct {
    uint32_t pad2:9;
    uint32_t picture_coding_type:2;
    uint32_t pad:21;
  } vld1;

  struct {
    uint32_t index_0:4;
    uint32_t index_1:4;
    uint32_t index_2:4;
    uint32_t index_3:4;
    uint32_t index_4:4;
    uint32_t index_5:4;
    uint32_t index_6:4;
    uint32_t index_7:4;
  } desc_remap_table0;

  struct {
    uint32_t index_8:4;
    uint32_t index_9:4;
    uint32_t index_10:4;
    uint32_t index_11:4;
    uint32_t index_12:4;
    uint32_t index_13:4;
    uint32_t index_14:4;
    uint32_t index_15:4;
  } desc_remap_table1;
};

struct i965_interface_descriptor 
{
  struct {
    uint32_t grf_reg_blocks:4;
    uint32_t pad:2;
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
    uint32_t const_urb_entry_read_offset:6;
    uint32_t const_urb_entry_read_len:6;
  } desc1;

  struct {
    uint32_t pad:2;
    uint32_t sampler_count:3;
    uint32_t sampler_state_pointer:27;
  } desc2;

  struct {
    uint32_t binding_table_entry_count:5;
    uint32_t binding_table_pointer:27;
  } desc3;
};

struct i965_surface_state 
{
  struct {
    uint32_t cube_pos_z:1;
    uint32_t cube_neg_z:1;
    uint32_t cube_pos_y:1;
    uint32_t cube_neg_y:1;
    uint32_t cube_pos_x:1;
    uint32_t cube_neg_x:1;
    uint32_t pad:3;
    uint32_t render_cache_read_mode:1;
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
    uint32_t pad:19;
    uint32_t min_array_elt:9;
    uint32_t min_lod:4;
  } ss4;

  struct {
    uint32_t pad:20;
    uint32_t y_offset:4;
    uint32_t pad2:1;
    uint32_t x_offset:7;
  } ss5;
};

#define BITFIELD_BIT(X) 1
#define BITFIELD_RANGE(X,Y) ((Y) - (X) + 1)

typedef struct i965_pipe_control
{
  struct
  {
    uint32_t length                              : BITFIELD_RANGE(0, 7);
    uint32_t reserved                            : BITFIELD_RANGE(8, 15);
    uint32_t instruction_subopcode               : BITFIELD_RANGE(16, 23);
    uint32_t instruction_opcode                  : BITFIELD_RANGE(24, 26);
    uint32_t instruction_pipeline                : BITFIELD_RANGE(27, 28);
    uint32_t instruction_type                    : BITFIELD_RANGE(29, 31);
  } dw0;

  struct
  {
    uint32_t depth_cache_flush_enable            : BITFIELD_BIT(0);
    uint32_t stall_at_pixel_scoreboard           : BITFIELD_BIT(1);
    uint32_t state_cache_invalidation_enable     : BITFIELD_BIT(2);
    uint32_t constant_cache_invalidation_enable  : BITFIELD_BIT(3);
    uint32_t vf_cache_invalidation_enable        : BITFIELD_BIT(4);
    uint32_t dc_flush_enable                     : BITFIELD_BIT(5);
    uint32_t protected_memory_app_id             : BITFIELD_BIT(6);
    uint32_t pipe_control_flush_enable           : BITFIELD_BIT(7);
    uint32_t notify_enable                       : BITFIELD_BIT(8);
    uint32_t indirect_state_pointers_disable     : BITFIELD_BIT(9);
    uint32_t texture_cache_invalidation_enable   : BITFIELD_BIT(10);
    uint32_t instruction_cache_invalidate_enable : BITFIELD_BIT(11);
    uint32_t render_target_cache_flush_enable    : BITFIELD_BIT(12);
    uint32_t depth_stall_enable                  : BITFIELD_BIT(13);
    uint32_t post_sync_operation                 : BITFIELD_RANGE(14, 15);
    uint32_t generic_media_state_clear           : BITFIELD_BIT(16);
    uint32_t synchronize_gfdt_surface            : BITFIELD_BIT(17);
    uint32_t tlb_invalidate                      : BITFIELD_BIT(18);
    uint32_t global_snapshot_count_reset         : BITFIELD_BIT(19);
    uint32_t cs_stall                            : BITFIELD_BIT(20);
    uint32_t store_data_index                    : BITFIELD_BIT(21);
    uint32_t protected_memory_enable             : BITFIELD_BIT(22);
    uint32_t reserved                            : BITFIELD_RANGE(23, 31);
  } dw1;

  struct
  {
    uint32_t reserved                            : BITFIELD_RANGE(0, 1);
    uint32_t destination_address_type            : BITFIELD_BIT(2);
    uint32_t address                             : BITFIELD_RANGE(3, 31);
  } dw2;

  struct
  {
    uint64_t data;
  } qw0;
} i965_pipe_control_t;

typedef struct i965_sampler_state
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
} i965_sampler_state_t;

typedef struct gen7_sampler_state
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
} gen7_sampler_state_t;

#undef BITFIELD_BIT
#undef BITFIELD_RANGE

#endif /* __GENX_STRUCTS_H__ */
