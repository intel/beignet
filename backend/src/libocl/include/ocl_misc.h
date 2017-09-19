/*
 * Copyright © 2012 - 2014 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __OCL_MISC_H__
#define __OCL_MISC_H__

#include "ocl_types.h"
#include "ocl_workitem.h"
#include "ocl_simd.h"
#include "ocl_printf.h"
#include "ocl_as.h"

#define DEC2(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle(XTYPE x, MASKTYPE##2 mask);

#define DEC4(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle(XTYPE x, MASKTYPE##4 mask);

#define DEC8(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle(XTYPE x, MASKTYPE##8 mask);

#define DEC16(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle(XTYPE x, MASKTYPE##16 mask);

#define DEFMASK(TYPE, MASKTYPE) \
  DEC2(TYPE, TYPE##2, MASKTYPE); DEC2(TYPE, TYPE##4, MASKTYPE); DEC2(TYPE, TYPE##8, MASKTYPE); DEC2(TYPE, TYPE##16, MASKTYPE) \
  DEC4(TYPE, TYPE##2, MASKTYPE); DEC4(TYPE, TYPE##4, MASKTYPE); DEC4(TYPE, TYPE##8, MASKTYPE); DEC4(TYPE, TYPE##16, MASKTYPE) \
  DEC8(TYPE, TYPE##2, MASKTYPE); DEC8(TYPE, TYPE##4, MASKTYPE); DEC8(TYPE, TYPE##8, MASKTYPE); DEC8(TYPE, TYPE##16, MASKTYPE) \
  DEC16(TYPE, TYPE##2, MASKTYPE); DEC16(TYPE, TYPE##4, MASKTYPE); DEC16(TYPE, TYPE##8, MASKTYPE); DEC16(TYPE, TYPE##16, MASKTYPE)

#define DEF(TYPE) \
  DEFMASK(TYPE, uchar) \
  DEFMASK(TYPE, ushort) \
  DEFMASK(TYPE, uint) \
  DEFMASK(TYPE, ulong)

DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(half)
DEF(int)
DEF(uint)
DEF(float)
DEF(long)
DEF(ulong)
DEF(double)
#undef DEF
#undef DEFMASK
#undef DEC2
#undef DEC4
#undef DEC8
#undef DEC16

#define DEC2(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##2 mask);

#define DEC2X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##2 mask);

#define DEC4(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##4 mask);

#define DEC4X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##4 mask);

#define DEC8(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##8 mask);

#define DEC8X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##8 mask);

#define DEC16(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##16 mask);

#define DEC16X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##16 mask);

#define DEFMASK(TYPE, MASKTYPE) \
  DEC2(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC2(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC2(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC2X(TYPE, MASKTYPE) \
  DEC4(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC4(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC4(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC4X(TYPE, MASKTYPE) \
  DEC8(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC8(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC8(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC8X(TYPE, MASKTYPE) \
  DEC16(TYPE, TYPE##2, TYPE##4, MASKTYPE) \
  DEC16(TYPE, TYPE##4, TYPE##8, MASKTYPE) \
  DEC16(TYPE, TYPE##8, TYPE##16, MASKTYPE) \
  DEC16X(TYPE, MASKTYPE)

#define DEF(TYPE) \
  DEFMASK(TYPE, uchar) \
  DEFMASK(TYPE, ushort) \
  DEFMASK(TYPE, uint) \
  DEFMASK(TYPE, ulong)

DEF(char)
DEF(uchar)
DEF(short)
DEF(ushort)
DEF(half)
DEF(int)
DEF(uint)
DEF(float)
DEF(long)
DEF(ulong)
DEF(double)
#undef DEF
#undef DEFMASK
#undef DEC2
#undef DEC2X
#undef DEC4
#undef DEC4X
#undef DEC8
#undef DEC8X
#undef DEC16
#undef DEC16X

struct time_stamp {
  // time tick
  ulong tick;
  // If context-switch or frequency change occurs since last read of tm,
  // event will be non-zero, otherwise, it will be zero.
  uint event;
};

//Interlaced image field polarity values:
#define CLK_AVC_ME_INTERLACED_SCAN_TOP_FIELD_INTEL    0x0
#define CLK_AVC_ME_INTERLACED_SCAN_BOTTOM_FIELD_INTEL 0x1

//Inter macro-block major shape values:
#define CLK_AVC_ME_MAJOR_16x16_INTEL 0x0
#define CLK_AVC_ME_MAJOR_16x8_INTEL  0x1
#define CLK_AVC_ME_MAJOR_8x16_INTEL  0x2
#define CLK_AVC_ME_MAJOR_8x8_INTEL   0x3

//Inter macro-block minor shape values:
#define CLK_AVC_ME_MINOR_8x8_INTEL 0x0
#define CLK_AVC_ME_MINOR_8x4_INTEL 0x1
#define CLK_AVC_ME_MINOR_4x8_INTEL 0x2
#define CLK_AVC_ME_MINOR_4x4_INTEL 0x3

//Inter macro-block major direction values:
#define CLK_AVC_ME_MAJOR_FORWARD_INTEL       0x0
#define CLK_AVC_ME_MAJOR_BACKWARD_INTEL      0x1
#define CLK_AVC_ME_MAJOR_BIDIRECTIONAL_INTEL 0x2

//Inter (IME) partition mask values:
#define CLK_AVC_ME_PARTITION_MASK_ALL_INTEL   0x0
#define CLK_AVC_ME_PARTITION_MASK_16x16_INTEL 0x7E
#define CLK_AVC_ME_PARTITION_MASK_16x8_INTEL  0x7D
#define CLK_AVC_ME_PARTITION_MASK_8x16_INTEL  0x7B
#define CLK_AVC_ME_PARTITION_MASK_8x8_INTEL   0x77
#define CLK_AVC_ME_PARTITION_MASK_8x4_INTEL   0x6F
#define CLK_AVC_ME_PARTITION_MASK_4x8_INTEL   0x5F
#define CLK_AVC_ME_PARTITION_MASK_4x4_INTEL   0x3F

//Slice type values:
#define CLK_AVC_ME_SLICE_TYPE_PRED_INTEL  0x0
#define CLK_AVC_ME_SLICE_TYPE_BPRED_INTEL 0x1
#define CLK_AVC_ME_SLICE_TYPE_INTRA_INTEL 0x2

//Search window configuration:
#define CLK_AVC_ME_SEARCH_WINDOW_EXHAUSTIVE_INTEL    0x0
#define CLK_AVC_ME_SEARCH_WINDOW_SMALL_INTEL         0x1
#define CLK_AVC_ME_SEARCH_WINDOW_TINY_INTEL          0x2
#define CLK_AVC_ME_SEARCH_WINDOW_EXTRA_TINY_INTEL    0x3
#define CLK_AVC_ME_SEARCH_WINDOW_DIAMOND_INTEL       0x4
#define CLK_AVC_ME_SEARCH_WINDOW_LARGE_DIAMOND_INTEL 0x5
#define CLK_AVC_ME_SEARCH_WINDOW_RESERVED0_INTEL     0x6
#define CLK_AVC_ME_SEARCH_WINDOW_RESERVED1_INTEL     0x7

//SAD adjustment mode:
#define CLK_AVC_ME_SAD_ADJUST_MODE_NONE_INTEL 0x0
#define CLK_AVC_ME_SAD_ADJUST_MODE_HAAR_INTEL 0x2

//Pixel resolution:
#define CLK_AVC_ME_SUBPIXEL_MODE_INTEGER_INTEL 0x0
#define CLK_AVC_ME_SUBPIXEL_MODE_HPEL_INTEL    0x1
#define CLK_AVC_ME_SUBPIXEL_MODE_QPEL_INTEL    0x3

//Cost precision values:
#define CLK_AVC_ME_COST_PRECISION_QPEL_INTEL 0x0
#define CLK_AVC_ME_COST_PRECISION_HPEL_INTEL 0x1
#define CLK_AVC_ME_COST_PRECISION_PEL_INTEL  0x2
#define CLK_AVC_ME_COST_PRECISION_DPEL_INTEL 0x3

//Inter bidirectional weights:
#define CLK_AVC_ME_BIDIR_WEIGHT_QUARTER_INTEL       0x10
#define CLK_AVC_ME_BIDIR_WEIGHT_THIRD_INTEL         0x15
#define CLK_AVC_ME_BIDIR_WEIGHT_HALF_INTEL          0x20
#define CLK_AVC_ME_BIDIR_WEIGHT_TWO_THIRD_INTEL     0x2B
#define CLK_AVC_ME_BIDIR_WEIGHT_THREE_QUARTER_INTEL 0x30

//Inter border reached values:
#define CLK_AVC_ME_BORDER_REACHED_LEFT_INTEL   0x0
#define CLK_AVC_ME_BORDER_REACHED_RIGHT_INTEL  0x2
#define CLK_AVC_ME_BORDER_REACHED_TOP_INTEL    0x4
#define CLK_AVC_ME_BORDER_REACHED_BOTTOM_INTEL 0x8

//Intra macro-block shape values:
#define CLK_AVC_ME_INTRA_16x16_INTEL  0x0
#define CLK_AVC_ME_INTRA_8x8_INTEL    0x1
#define CLK_AVC_ME_INTRA_4x4_INTEL    0x2

//Inter skip block partition type:
#define CLK_AVC_ME_SKIP_BLOCK_PARTITION_16x16_INTEL 0x0
#define CLK_AVC_ME_SKIP_BLOCK_PARTITION_8x8_INTEL   0x04000

//Inter skip motion vector mask:
#define CLK_AVC_ME_SKIP_BLOCK_16x16_FORWARD_ENABLE_INTEL   (0x1<<24)
#define CLK_AVC_ME_SKIP_BLOCK_16x16_BACKWARD_ ENABLE_INTEL (0x2<<24)
#define CLK_AVC_ME_SKIP_BLOCK_16x16_DUAL_ENABLE_INTEL      (0x3<<24)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_FORWARD_ENABLE_INTEL     (0x55<<24)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_BACKWARD_ENABLE_INTEL    (0xAA<<24)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_DUAL_ENABLE_INTEL        (0xFF<<24)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_0_FORWARD_ENABLE_INTEL   (0x1<<24)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_0_BACKWARD_ENABLE_INTEL  (0x2<<24)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_1_FORWARD_ENABLE_INTEL   (0x1<<26)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_1_BACKWARD_ENABLE_INTEL  (0x2<<26)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_2_FORWARD_ENABLE_INTEL   (0x1<<28)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_2_BACKWARD_ENABLE_INTEL  (0x2<<28)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_3_FORWARD_ENABLE_INTEL   (0x1<<30)
#define CLK_AVC_ME_SKIP_BLOCK_8x8_3_BACKWARD_ENABLE_INTEL  (0x2<<30)

//Block based skip type values:
#define CLK_AVC_ME_BLOCK_BASED_SKIP_4x4_INTEL 0x0
#define CLK_AVC_ME_BLOCK_BASED_SKIP_8x8_INTEL 0x80

//Luma intra partition mask values:
#define CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_ALL_INTEL   0x0
#define CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_16x16_INTEL 0x6
#define CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_8x8_INTEL   0x5
#define CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_4x4_INTEL   0x3

//Intra neighbor availability mask values:
#define CLK_AVC_ME_INTRA_NEIGHBOR_LEFT_MASK_ENABLE_INTEL        0x60
#define CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_MASK_ENABLE_INTEL       0x10
#define CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_RIGHT_MASK_ENABLE_INTEL 0x8
#define CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_LEFT_MASK_ENABLE_INTEL  0x4

//Luma intra modes:
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_VERTICAL_INTEL            0x0
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_HORIZONTAL_INTEL          0x1
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_DC_INTEL                  0x2
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_DIAGONAL_DOWN_LEFT_INTEL  0x3
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_DIAGONAL_DOWN_RIGHT_INTEL 0x4
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_PLANE_INTEL               0x4
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_VERTICAL_RIGHT_INTEL      0x5
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_HORIZONTAL_DOWN_INTEL     0x6
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_VERTICAL_LEFT_INTEL       0x7
#define CLK_AVC_ME_LUMA_PREDICTOR_MODE_HORIZONTAL_UP_INTEL       0x8

//Chroma intra modes:
#define CLK_AVC_ME_CHROMA_PREDICTOR_MODE_DC_INTEL         0x0
#define CLK_AVC_ME_CHROMA_PREDICTOR_MODE_HORIZONTAL_INTEL 0x1
#define CLK_AVC_ME_CHROMA_PREDICTOR_MODE_VERTICAL_INTEL   0x2
#define CLK_AVC_ME_CHROMA_PREDICTOR_MODE_PLANE_INTEL      0x3

//Reference image select values:
#define CLK_AVC_ME_FRAME_FORWARD_INTEL  0x1
#define CLK_AVC_ME_FRAME_BACKWARD_INTEL 0x2
#define CLK_AVC_ME_FRAME_DUAL_INTEL     0x3

//VME media sampler initialization value:
#define CLK_AVC_ME_INITIALIZE_INTEL 0x0

//Default IME payload initialization:
#define CLK_AVC_IME_PAYLOAD_INITIALIZE_INTEL {0x0}

//Default REF payload initialization:
#define CLK_AVC_REF_PAYLOAD_INITIALIZE_INTEL {0x0}

//Default SIC payload initialization:
#define CLK_AVC_SIC_PAYLOAD_INITIALIZE_INTEL {0x0}

//Default IME result initialization:
#define CLK_AVC_IME_RESULT_INITIALIZE_INTEL  {0x0}

//Default REF result initialization:
#define CLK_AVC_REF_RESULT_INITIALIZE_INTEL  {0x0}

//Default SIC result initialization:
#define CLK_AVC_SIC_RESULT_INITIALIZE_INTEL  {0x0}

typedef struct{
  ushort2 srcCoord;
  short2 ref_offset;
  uchar partition_mask;
  uchar sad_adjustment;
  uchar search_window_config;
  ulong cc0;
  ulong cc1;
  ulong cc2;
  ulong cc3;
  uint2 packed_cost_table;
  uchar cost_precision;
  ulong packed_shape_cost;
}intel_sub_group_avc_ime_payload_t;

typedef uint8 intel_sub_group_avc_ime_result_t;

#define REF_ENABLE_COST_PENALTY 1

typedef struct{
  ushort2 srcCoord;
  long mv;
  uchar major_shape;
  uchar minor_shapes;
  uchar directions;
  uchar pixel_mode;
  uchar sad_adjustment;
#if REF_ENABLE_COST_PENALTY 
  ulong cc0;
  ulong cc1;
  ulong cc2;
  ulong cc3;
  uint2 packed_cost_table;
  uchar cost_precision;
  ulong packed_shape_cost;
#endif
}intel_sub_group_avc_ref_payload_t;

typedef struct{
  ushort2 srcCoord;
  uint skip_block_partition_type;
  uint skip_motion_vector_mask;
  char bidirectional_weight;
  uchar skip_sad_adjustment;
  long mv;

  uchar luma_intra_partition_mask;
  uchar intra_neighbour_availabilty;
  uint l_0_3;
  uint l_4_7;
  uint l_8_11;
  uint l_12_15;
  uint u_0_3;
  uint u_4_7;
  uint u_8_11;
  uint u_12_15;
  uint ur_16_19;
  uint ur_20_23;
  uchar upper_left_corner_luma_pixel;
  uchar intra_sad_adjustment;
  uint intra_shape_cost;
}intel_sub_group_avc_sic_payload_t;

typedef uint8 intel_sub_group_avc_ref_result_t;

typedef uint8 intel_sub_group_avc_sic_result_t;

uint __gen_ocl_region(ushort offset, uint data);

struct time_stamp __gen_ocl_get_timestamp(void);

uint8 __gen_ocl_vme(image2d_t, image2d_t,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   uint, uint, uint, uint,
                   int, int, int);

intel_sub_group_avc_ime_result_t
__gen_ocl_ime(image2d_t, image2d_t,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             uint, uint, uint, uint,
             int);

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_initialize(ushort2 src_coord,
                                  uchar partition_mask,
                                  uchar sad_adjustment);

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_set_single_reference(short2 ref_offset,
                                            uchar search_window_config,
                                            intel_sub_group_avc_ime_payload_t payload);

intel_sub_group_avc_ime_result_t
intel_sub_group_avc_ime_evaluate_with_single_reference(read_only image2d_t src_image,
                                                      read_only image2d_t ref_image,
                                                      sampler_t vme_media_sampler,
                                                      intel_sub_group_avc_ime_payload_t payload);

ulong intel_sub_group_avc_ime_get_motion_vectors(intel_sub_group_avc_ime_result_t result);

ushort intel_sub_group_avc_ime_get_inter_distortions(intel_sub_group_avc_ime_result_t result);

ushort intel_sub_group_avc_ime_get_inter_distortions(intel_sub_group_avc_ime_result_t result);

uchar intel_sub_group_avc_ime_get_inter_major_shape(intel_sub_group_avc_ime_result_t result);

uchar intel_sub_group_avc_ime_get_inter_minor_shapes(intel_sub_group_avc_ime_result_t result);

uchar intel_sub_group_avc_ime_get_inter_directions(intel_sub_group_avc_ime_result_t result);

intel_sub_group_avc_ref_payload_t
intel_sub_group_avc_fme_initialize(ushort2 src_coord,
                                  ulong motion_vectors,
                                  uchar major_shapes,
                                  uchar minor_shapes,
                                  uchar directions,
                                  uchar pixel_resolution,
                                  uchar sad_adjustment );

intel_sub_group_avc_ref_result_t
intel_sub_group_avc_ref_evaluate_with_single_reference(read_only image2d_t src_image,
                                                      read_only image2d_t ref_image,
                                                      sampler_t vme_media_sampler,
                                                      intel_sub_group_avc_ref_payload_t payload);

ulong intel_sub_group_avc_ref_get_motion_vectors(intel_sub_group_avc_ref_result_t result);

ushort intel_sub_group_avc_ref_get_inter_distortions(intel_sub_group_avc_ref_result_t result);

uint2 intel_sub_group_avc_mce_get_default_medium_penalty_cost_table(void);

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_set_motion_vector_cost_function(ulong packed_cost_center_delta,
                                                        uint2 packed_cost_table,
                                                        uchar cost_precision,
                                                        intel_sub_group_avc_ime_payload_t payload);

#if REF_ENABLE_COST_PENALTY 
intel_sub_group_avc_ref_payload_t
intel_sub_group_avc_ref_set_motion_vector_cost_function(ulong packed_cost_center_delta,
                                                        uint2 packed_cost_table,
                                                        uchar cost_precision,
                                                        intel_sub_group_avc_ref_payload_t payload);
#endif

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_set_inter_shape_penalty(ulong packed_shape_cost,
                                                intel_sub_group_avc_ime_payload_t payload);

intel_sub_group_avc_sic_result_t
intel_sub_group_avc_sic_evaluate_ipe(read_only image2d_t src_image,
                                     sampler_t vme_media_sampler,
                                     intel_sub_group_avc_sic_payload_t payload);

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_initialize(ushort2 src_coord );

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_configure_ipe(uchar luma_intra_partition_mask,
                                      uchar intra_neighbour_availabilty,
                                      uchar left_edge_luma_pixels,
                                      uchar upper_left_corner_luma_pixel,
                                      uchar upper_edge_luma_pixels,
                                      uchar upper_right_edge_luma_pixels,
                                      uchar intra_sad_adjustment,
                                      intel_sub_group_avc_sic_payload_t payload );
intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_set_intra_luma_shape_penalty(uint packed_shape_cost,
                                                     intel_sub_group_avc_sic_payload_t payload );

uchar
intel_sub_group_avc_sic_get_ipe_luma_shape(intel_sub_group_avc_sic_result_t result);

ushort
intel_sub_group_avc_sic_get_best_ipe_luma_distortion(intel_sub_group_avc_sic_result_t result);

ulong intel_sub_group_avc_sic_get_packed_ipe_luma_modes(intel_sub_group_avc_sic_result_t result);


intel_sub_group_avc_sic_result_t
intel_sub_group_avc_sic_evaluate_with_single_reference(read_only image2d_t src_image,
                                                      read_only image2d_t ref_image,
                                                      sampler_t vme_media_sampler,
                                                      intel_sub_group_avc_sic_payload_t payload);

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_configure_skc(uint skip_block_partition_type,
                                      uint skip_motion_vector_mask,
                                      ulong motion_vectors,
                                      char bidirectional_weight,
                                      uchar skip_sad_adjustment,
                                      intel_sub_group_avc_sic_payload_t payload);

ushort
intel_sub_group_avc_sic_get_inter_distortions(intel_sub_group_avc_sic_result_t result);

bool __gen_ocl_in_local(size_t p);
bool __gen_ocl_in_private(size_t p);

#if (__OPENCL_C_VERSION__ >= 200)
local void *__to_local(generic void *p);
global void *__to_global(generic void *p);
private void *__to_private(generic void *p);
#endif
#endif
