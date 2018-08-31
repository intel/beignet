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
#include "ocl_misc.h"

#define DEC2(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle(XTYPE x, MASKTYPE##2 mask) { \
    TYPE##2 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC4(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle(XTYPE x, MASKTYPE##4 mask) { \
    TYPE##4 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC8(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle(XTYPE x, MASKTYPE##8 mask) { \
    TYPE##8 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & (vec_step(x) - 1)]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & (vec_step(x) - 1)]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & (vec_step(x) - 1)]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & (vec_step(x) - 1)]; \
    return y; \
  }

#define DEC16(TYPE, XTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle(XTYPE x, MASKTYPE##16 mask) { \
    TYPE##16 y; \
    y.s0 = ((TYPE *) &x)[mask.s0 & (vec_step(x) - 1)]; \
    y.s1 = ((TYPE *) &x)[mask.s1 & (vec_step(x) - 1)]; \
    y.s2 = ((TYPE *) &x)[mask.s2 & (vec_step(x) - 1)]; \
    y.s3 = ((TYPE *) &x)[mask.s3 & (vec_step(x) - 1)]; \
    y.s4 = ((TYPE *) &x)[mask.s4 & (vec_step(x) - 1)]; \
    y.s5 = ((TYPE *) &x)[mask.s5 & (vec_step(x) - 1)]; \
    y.s6 = ((TYPE *) &x)[mask.s6 & (vec_step(x) - 1)]; \
    y.s7 = ((TYPE *) &x)[mask.s7 & (vec_step(x) - 1)]; \
    y.s8 = ((TYPE *) &x)[mask.s8 & (vec_step(x) - 1)]; \
    y.s9 = ((TYPE *) &x)[mask.s9 & (vec_step(x) - 1)]; \
    y.sA = ((TYPE *) &x)[mask.sA & (vec_step(x) - 1)]; \
    y.sB = ((TYPE *) &x)[mask.sB & (vec_step(x) - 1)]; \
    y.sC = ((TYPE *) &x)[mask.sC & (vec_step(x) - 1)]; \
    y.sD = ((TYPE *) &x)[mask.sD & (vec_step(x) - 1)]; \
    y.sE = ((TYPE *) &x)[mask.sE & (vec_step(x) - 1)]; \
    y.sF = ((TYPE *) &x)[mask.sF & (vec_step(x) - 1)]; \
    return y; \
  }

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
  OVERLOADABLE TYPE##2 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##2 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC2X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##2 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##2 mask) { \
    TYPE##2 z; \
    z.s0 = (mask.s0 & 31) < 16 ? ((TYPE *)&x)[mask.s0 & 31] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = (mask.s1 & 31) < 16 ? ((TYPE *)&x)[mask.s1 & 31] : ((TYPE *)&y)[mask.s1 & 15]; \
    return z; \
  }

#define DEC4(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##4 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC4X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##4 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##4 mask) { \
    TYPE##4 z; \
    z.s0 = (mask.s0 & 31) < 16 ? ((TYPE *)&x)[mask.s0 & 31] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = (mask.s1 & 31) < 16 ? ((TYPE *)&x)[mask.s1 & 31] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = (mask.s2 & 31) < 16 ? ((TYPE *)&x)[mask.s2 & 31] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = (mask.s3 & 31) < 16 ? ((TYPE *)&x)[mask.s3 & 31] : ((TYPE *)&y)[mask.s3 & 15]; \
    return z; \
  }

#define DEC8(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##8 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC8X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##8 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##8 mask) { \
    TYPE##8 z; \
    z.s0 = (mask.s0 & 31) < 16 ? ((TYPE *)&x)[mask.s0 & 31] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = (mask.s1 & 31) < 16 ? ((TYPE *)&x)[mask.s1 & 31] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = (mask.s2 & 31) < 16 ? ((TYPE *)&x)[mask.s2 & 31] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = (mask.s3 & 31) < 16 ? ((TYPE *)&x)[mask.s3 & 31] : ((TYPE *)&y)[mask.s3 & 15]; \
    z.s4 = (mask.s4 & 31) < 16 ? ((TYPE *)&x)[mask.s4 & 31] : ((TYPE *)&y)[mask.s4 & 15]; \
    z.s5 = (mask.s5 & 31) < 16 ? ((TYPE *)&x)[mask.s5 & 31] : ((TYPE *)&y)[mask.s5 & 15]; \
    z.s6 = (mask.s6 & 31) < 16 ? ((TYPE *)&x)[mask.s6 & 31] : ((TYPE *)&y)[mask.s6 & 15]; \
    z.s7 = (mask.s7 & 31) < 16 ? ((TYPE *)&x)[mask.s7 & 31] : ((TYPE *)&y)[mask.s7 & 15]; \
    return z; \
  }

#define DEC16(TYPE, ARGTYPE, TEMPTYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(ARGTYPE x, ARGTYPE y, MASKTYPE##16 mask) { \
    return shuffle((TEMPTYPE)(x, y), mask); \
  }

#define DEC16X(TYPE, MASKTYPE) \
  OVERLOADABLE TYPE##16 shuffle2(TYPE##16 x, TYPE##16 y, MASKTYPE##16 mask) { \
    TYPE##16 z; \
    z.s0 = (mask.s0 & 31) < 16 ? ((TYPE *)&x)[mask.s0 & 31] : ((TYPE *)&y)[mask.s0 & 15]; \
    z.s1 = (mask.s1 & 31) < 16 ? ((TYPE *)&x)[mask.s1 & 31] : ((TYPE *)&y)[mask.s1 & 15]; \
    z.s2 = (mask.s2 & 31) < 16 ? ((TYPE *)&x)[mask.s2 & 31] : ((TYPE *)&y)[mask.s2 & 15]; \
    z.s3 = (mask.s3 & 31) < 16 ? ((TYPE *)&x)[mask.s3 & 31] : ((TYPE *)&y)[mask.s3 & 15]; \
    z.s4 = (mask.s4 & 31) < 16 ? ((TYPE *)&x)[mask.s4 & 31] : ((TYPE *)&y)[mask.s4 & 15]; \
    z.s5 = (mask.s5 & 31) < 16 ? ((TYPE *)&x)[mask.s5 & 31] : ((TYPE *)&y)[mask.s5 & 15]; \
    z.s6 = (mask.s6 & 31) < 16 ? ((TYPE *)&x)[mask.s6 & 31] : ((TYPE *)&y)[mask.s6 & 15]; \
    z.s7 = (mask.s7 & 31) < 16 ? ((TYPE *)&x)[mask.s7 & 31] : ((TYPE *)&y)[mask.s7 & 15]; \
    z.s8 = (mask.s8 & 31) < 16 ? ((TYPE *)&x)[mask.s8 & 31] : ((TYPE *)&y)[mask.s8 & 15]; \
    z.s9 = (mask.s9 & 31) < 16 ? ((TYPE *)&x)[mask.s9 & 31] : ((TYPE *)&y)[mask.s9 & 15]; \
    z.sA = (mask.sA & 31) < 16 ? ((TYPE *)&x)[mask.sA & 31] : ((TYPE *)&y)[mask.sA & 15]; \
    z.sB = (mask.sB & 31) < 16 ? ((TYPE *)&x)[mask.sB & 31] : ((TYPE *)&y)[mask.sB & 15]; \
    z.sC = (mask.sC & 31) < 16 ? ((TYPE *)&x)[mask.sC & 31] : ((TYPE *)&y)[mask.sC & 15]; \
    z.sD = (mask.sD & 31) < 16 ? ((TYPE *)&x)[mask.sD & 31] : ((TYPE *)&y)[mask.sD & 15]; \
    z.sE = (mask.sE & 31) < 16 ? ((TYPE *)&x)[mask.sE & 31] : ((TYPE *)&y)[mask.sE & 15]; \
    z.sF = (mask.sF & 31) < 16 ? ((TYPE *)&x)[mask.sF & 31] : ((TYPE *)&y)[mask.sF & 15]; \
    return z; \
  }

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

uint __gen_ocl_read_tm(void);
uint __gen_ocl_region(ushort offset, uint data);

struct time_stamp __gen_ocl_get_timestamp(void) {
  struct time_stamp val;

  uint tm = __gen_ocl_read_tm();
  val.tick = ((ulong)__gen_ocl_region(1, tm) << 32) | __gen_ocl_region(0, tm);
  val.event = __gen_ocl_region(2, tm);

  return val;
};

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_initialize(ushort2 src_coord,
                                uchar partition_mask,
                                uchar sad_adjustment){
  intel_sub_group_avc_ime_payload_t pl;
  pl.srcCoord = src_coord;
  pl.partition_mask = partition_mask;
  pl.sad_adjustment = sad_adjustment;
  pl.ref_offset = (short2)(0, 0);
  pl.search_window_config = 0;
  pl.cc0 = 0;
  pl.cc1 = 0;
  pl.cc2 = 0;
  pl.cc3 = 0;
  pl.packed_cost_table = (uint2)(0, 0);
  pl.cost_precision = 2;
  pl.packed_shape_cost = 0;
  return pl;
}

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_set_single_reference(short2 ref_offset,
                                            uchar search_window_config,
                                            intel_sub_group_avc_ime_payload_t payload){
  intel_sub_group_avc_ime_payload_t pl = payload;
  pl.ref_offset = ref_offset;
  pl.search_window_config = search_window_config;
  return pl;
}

intel_sub_group_avc_ime_result_t
intel_sub_group_avc_ime_evaluate_with_single_reference(read_only image2d_t src_image,
                                                      read_only image2d_t ref_image,
                                                      sampler_t vme_media_sampler,
                                                      intel_sub_group_avc_ime_payload_t payload){
  uint src_grf0_dw7;
  uint src_grf0_dw6;
  uint src_grf0_dw5;
  uint src_grf0_dw4;
  uint src_grf0_dw3;
  uint src_grf0_dw2;
  uint src_grf0_dw1;
  uint src_grf0_dw0;
  uint src_grf1_dw7;
  uint src_grf1_dw6;
  uint src_grf1_dw5;
  uint src_grf1_dw4;
  uint src_grf1_dw3;
  uint src_grf1_dw2;
  uint src_grf1_dw1;
  uint src_grf1_dw0;
  uint src_grf2_dw7;
  uint src_grf2_dw6;
  uint src_grf2_dw5;
  uint src_grf2_dw4;
  uint src_grf2_dw3;
  uint src_grf2_dw2;
  uint src_grf2_dw1;
  uint src_grf2_dw0;
  uint src_grf3_dw7;
  uint src_grf3_dw6;
  uint src_grf3_dw5;
  uint src_grf3_dw4;
  uint src_grf3_dw3;
  uint src_grf3_dw2;
  uint src_grf3_dw1;
  uint src_grf3_dw0;
  uint src_grf4_dw7;
  uint src_grf4_dw6;
  uint src_grf4_dw5;
  uint src_grf4_dw4;
  uint src_grf4_dw3;
  uint src_grf4_dw2;
  uint src_grf4_dw1;
  uint src_grf4_dw0;
  uint src_grf5_dw7;
  uint src_grf5_dw6;
  uint src_grf5_dw5;
  uint src_grf5_dw4;
  uint src_grf5_dw3;
  uint src_grf5_dw2;
  uint src_grf5_dw1;
  uint src_grf5_dw0;
  uint src_grf6_dw7;
  uint src_grf6_dw6;
  uint src_grf6_dw5;
  uint src_grf6_dw4;
  uint src_grf6_dw3;
  uint src_grf6_dw2;
  uint src_grf6_dw1;
  uint src_grf6_dw0;
  uint src_grf7_dw7;
  uint src_grf7_dw6;
  uint src_grf7_dw5;
  uint src_grf7_dw4;
  uint src_grf7_dw3;
  uint src_grf7_dw2;
  uint src_grf7_dw1;
  uint src_grf7_dw0;


  //src_grf0_dw7 = Debug;
  src_grf0_dw7 = 0;
  //src_grf0_dw6 = Debug;
  src_grf0_dw6 = 0;
  //src_grf0_dw4 = Ignored;
  src_grf0_dw4 = 0;

  short2 predict_mv = payload.ref_offset;
  //CL_ME_SEARCH_PATH_RADIUS_2_2_INTEL
  //src_grf0_dw5 = (Ref_Height << 24) | (Ref_Width << 16) | (Ignored << 8) | (Dispatch_Id);
  src_grf0_dw5 =   (20 << 24)         | (20 << 16)        | (0 << 8)       | (0);
  //src_grf0_dw1 = (Ref1Y << 16)  | (Ref1X);
  src_grf0_dw1 =   ((-2 + predict_mv.y) << 16 ) | ((-2 + predict_mv.x) & 0x0000ffff);
  //src_grf0_dw0 = (Ref0Y << 16)  | (Ref0X);
  src_grf0_dw0 =   ((-2 + predict_mv.y) << 16 ) | ((-2 + predict_mv.x) & 0x0000ffff);

  //src_grf0_dw3 = (Reserved << 31)                | (Sub_Mb_Part_Mask << 24)       | (Intra_SAD << 22)
  src_grf0_dw3 =   (0 << 31)                       | (payload.partition_mask << 24) | (0 << 22)
                 //| (Inter_SAD << 20)             | (BB_Skip_Enabled << 19)        | (Reserverd << 18)
                   | (payload.sad_adjustment << 20)| (0 << 19)                      | (0 << 18)
                 //| (Dis_Aligned_Src_Fetch << 17) | (Dis_Aligned_Ref_Fetch << 16)  | (Dis_Field_Cache_Alloc << 15)
                   | (0 << 17)                     | (0 << 16)                      | (0 << 15)
                 //| (Skip_Type << 14)             | (Sub_Pel_Mode << 12)           | (Dual_Search_Path_Opt << 11)
                   | (0 << 14)                     | (0 << 12)                      | (0 << 11)
                 //| (Search_Ctrl << 8)            | (Ref_Access << 7)              | (SrcAccess << 6)
                   | (0 << 8)                      | (0 << 7)                       | (0 << 6)
                 //| (Mb_Type_Remap << 4)          | (Reserved_Workaround << 3)     | (Reserved_Workaround << 2)
                   | (0 << 4)                      | (0 << 3)                       | (0 << 2)
                 //| (Src_Size);
                   | (0);

  //src_grf0_dw2 = (SrcY << 16) | (SrcX);
  src_grf0_dw2 = (payload.srcCoord.y << 16)  | (payload.srcCoord.x);

  /*src_grf1_dw7 = (Skip_Center_Mask << 24)         | (Reserved << 22)               | (Ref1_Field_Polarity << 21)
                 | (Ref0_Field_Polarity << 20)   | (Src_Field_Polarity << 19)     | (Bilinear_Enable << 18)
                 | (MV_Cost_Scale_Factor << 16)  | (Mb_Intra_Struct << 8)         | (Intra_Corner_Swap << 7)
                 | (Non_Skip_Mode_Added << 6)    | (Non_Skip_ZMv_Added << 5)      | (IntraPartMask);*/
  src_grf1_dw7 = (payload.cost_precision << 16);
  //src_grf1_dw6 = Reserved;
  src_grf1_dw6 = 0;
  /*src_grf1_dw5 = Reseverd for BDW+
  src_grf1_dw4 = Reseverd for BDW+*/
  src_grf1_dw5 = 0;
  src_grf1_dw4 = 0;
  //src_grf1_dw3 = Weighted SAD Control Sub-block 0...15
  src_grf1_dw3 = 0;
  //XXX: should set src_grf1_dw2
  //src_grf1_dw2 = (Start1Y << 28)                  | (Start1X << 24)                | (Start0Y << 20)
  src_grf1_dw2 =   (0 << 28)                        | (0 << 24)                      | (0 << 20)
                 //| (Start0X << 16)               | (Max_Num_SU << 8)              | (LenSP);
                   | (0 << 16)                     | (2 << 8)                       | (2);
  /*src_grf1_dw1 = (RepartEn << 31)                 | (FBPrunEn << 30)               | (AdaptiveValidationControl << 29)
                 | (Uni_Mix_Disable << 28)       | (Bi_Sub_Mb_Part_Mask << 24)    | (Reserverd << 22)
                 | (Bi_Weight << 16)             | (Reserved << 6)                | (MaxNumMVs);*/
  src_grf1_dw1 = (0 << 24) | (16);
  /*src_grf1_dw0 = (Early_Ime_Stop << 24)           | (Early_Fme_Success << 16)      | (Skip_Success << 8)
                 | (T8x8_Flag_For_Inter_En << 7) | (Quit_Inter_En << 6)           | (Early_Ime_Success_En << 5)
                 | (Early_Success_En << 4)       | (Part_Candidate_En << 3)       | (Bi_Mix_Dis << 2)
                 | (Adaptive_En  << 1)           | (SkipModeEn);*/
  src_grf1_dw0 = 0;

  //src_grf2_dw7 = SIC Forward Transform Coeff Threshold Matrix[3...6]
  src_grf2_dw7 = 0;
  //src_grf2_dw6 = SIC Forward Transform Coeff Threshold Matrix[0...2]
  src_grf2_dw6 = 0;
  //src_grf2_dw5 = (Reserved << 24) | (FBR_SubPredMode_Input << 16) | (FBR_SubMBShape_Input << 8) | (Reserved << 2) | (FBR_MbMode_Input);
  src_grf2_dw5 = 0;
  //src_grf2_dw4 = MV_4_Cost ... MV_7_Cost;
  src_grf2_dw4 = payload.packed_cost_table.s1;
  //src_grf2_dw3 = MV_0_Cost ... MV_3_Cost;
  src_grf2_dw3 = payload.packed_cost_table.s0;
  //src_grf2_dw2 = ... Mode 8 Cost;
  src_grf2_dw2 = (payload.packed_shape_cost >> 32) & 0x000000ff;
  //src_grf2_dw1 = Mode 4 Cost ... Mode 7 Cost
  src_grf2_dw1 = payload.packed_shape_cost;
  src_grf2_dw0 = 0;
  //src_grf3_dw7 = (BWDCostCenter3Y << 16) | (BWDCostCenter3X) ;
  src_grf3_dw7 = payload.cc3 >> 32;
  //src_grf3_dw6 = (FWDCostCenter3Y << 16) | (FWDCostCenter3X) ;
  src_grf3_dw6 = payload.cc3;
  //src_grf3_dw5 = (BWDCostCenter2Y << 16) | (BWDCostCenter2X) ;
  src_grf3_dw5 = payload.cc2 >> 32;
  //src_grf3_dw4 = (FWDCostCenter2Y << 16) | (FWDCostCenter2X) ;
  src_grf3_dw4 = payload.cc2;
  //src_grf3_dw3 = (BWDCostCenter1Y << 16) | (BWDCostCenter1X) ;
  src_grf3_dw3 = payload.cc1 >> 32;
  //src_grf3_dw2 = (FWDCostCenter1Y << 16) | (FWDCostCenter1X) ;
  src_grf3_dw2 = payload.cc1;
  //src_grf3_dw1 = (BWDCostCenter0Y << 16) | (BWDCostCenter0X) ;
  src_grf3_dw1 = payload.cc0 >> 32;
  //src_grf3_dw0 = (FWDCostCenter0Y << 16) | (FWDCostCenter0X) ;
  src_grf3_dw0 = payload.cc0;

  //XXX: TODO: set search path
  src_grf4_dw7 = 0;
  src_grf4_dw6 = 0;
  src_grf4_dw5 = 0;
  src_grf4_dw4 = 0;
  src_grf4_dw3 = 0;
  src_grf4_dw2 = 0;
  src_grf4_dw1 = 0;
  src_grf4_dw0 = 0;
  src_grf5_dw7 = 0;
  src_grf5_dw6 = 0;
  src_grf5_dw5 = 0;
  src_grf5_dw4 = 0;
  src_grf5_dw3 = 0;
  src_grf5_dw2 = 0;
  src_grf5_dw1 = 0;
  src_grf5_dw0 = 0;

  intel_sub_group_avc_ime_result_t ime_result;
  ime_result = __gen_ocl_ime(src_image, ref_image,
                src_grf0_dw7, src_grf0_dw6, src_grf0_dw5, src_grf0_dw4,
                src_grf0_dw3, src_grf0_dw2, src_grf0_dw1, src_grf0_dw0,
                src_grf1_dw7, src_grf1_dw6, src_grf1_dw5, src_grf1_dw4,
                src_grf1_dw3, src_grf1_dw2, src_grf1_dw1, src_grf1_dw0,
                src_grf2_dw7, src_grf2_dw6, src_grf2_dw5, src_grf2_dw4,
                src_grf2_dw3, src_grf2_dw2, src_grf2_dw1, src_grf2_dw0,
                src_grf3_dw7, src_grf3_dw6, src_grf3_dw5, src_grf3_dw4,
                src_grf3_dw3, src_grf3_dw2, src_grf3_dw1, src_grf3_dw0,
                src_grf4_dw7, src_grf4_dw6, src_grf4_dw5, src_grf4_dw4,
                src_grf4_dw3, src_grf4_dw2, src_grf4_dw1, src_grf4_dw0,
                src_grf5_dw7, src_grf5_dw6, src_grf5_dw5, src_grf5_dw4,
                src_grf5_dw3, src_grf5_dw2, src_grf5_dw1, src_grf5_dw0,
                src_grf6_dw7, src_grf6_dw6, src_grf6_dw5, src_grf6_dw4,
                src_grf6_dw3, src_grf6_dw2, src_grf6_dw1, src_grf6_dw0,
                src_grf7_dw7, src_grf7_dw6, src_grf7_dw5, src_grf7_dw4,
                src_grf7_dw3, src_grf7_dw2, src_grf7_dw1, src_grf7_dw0,
                //msg_type
                2);

  return ime_result;
}

ulong intel_sub_group_avc_ime_get_motion_vectors(intel_sub_group_avc_ime_result_t result){
  uint lid_x = get_sub_group_local_id();
  uint fwd_mv, bwd_mv;
  if(lid_x < 4){
    fwd_mv = intel_sub_group_shuffle(result.s0, 8 + lid_x*2);
    bwd_mv = intel_sub_group_shuffle(result.s0, 9 + lid_x*2);
  }
  else if(lid_x >= 4 && lid_x <= 12){
    fwd_mv = intel_sub_group_shuffle(result.s1, 0 + (lid_x-4)*2);
    bwd_mv = intel_sub_group_shuffle(result.s1, 1 + (lid_x-4)*2);
  }
  else if(lid_x < 16){
    fwd_mv = intel_sub_group_shuffle(result.s2, 0 + (lid_x-12)*2);
    bwd_mv = intel_sub_group_shuffle(result.s2, 1 + (lid_x-12)*2);
  }
  
  ulong res = (bwd_mv << 32) | (fwd_mv & 0x00000000ffffffff);
  return res;
}

ushort intel_sub_group_avc_ime_get_inter_distortions(intel_sub_group_avc_ime_result_t result){
  uint lid_x = get_sub_group_local_id();
  uint write_back_dw = intel_sub_group_shuffle(result.s2, 8 + lid_x/2);
  int start_bit = lid_x%2 * 16;
  ushort distortion = (write_back_dw >> start_bit);
  return distortion;
}

uchar intel_sub_group_avc_ime_get_inter_major_shape(intel_sub_group_avc_ime_result_t result){
  uint write_back_dw00 = intel_sub_group_shuffle(result.s0, 0);
  uchar major_shape = write_back_dw00 & 0x03;
  return major_shape;
}

uchar intel_sub_group_avc_ime_get_inter_minor_shapes(intel_sub_group_avc_ime_result_t result){
  uint write_back_dw06 = intel_sub_group_shuffle(result.s0, 6);
  uchar minor_shape = (write_back_dw06 >> 8) & 0xff;
  return minor_shape;
}

uchar intel_sub_group_avc_ime_get_inter_directions(intel_sub_group_avc_ime_result_t result){
  uint write_back_dw06 = intel_sub_group_shuffle(result.s0, 6);
  uchar direction = (write_back_dw06 >> 16) & 0xff;
  return direction;
}

intel_sub_group_avc_ref_payload_t
intel_sub_group_avc_fme_initialize(ushort2 src_coord,
                                   ulong motion_vectors,
                                   uchar major_shapes,
                                   uchar minor_shapes,
                                   uchar directions,
                                   uchar pixel_resolution,
                                   uchar sad_adjustment ){
  intel_sub_group_avc_ref_payload_t pl;
  pl.srcCoord = src_coord;
  pl.mv = motion_vectors;
  pl.major_shape = major_shapes;
  pl.minor_shapes = minor_shapes;
  pl.directions = directions;
  pl.pixel_mode = pixel_resolution;
  pl.sad_adjustment = sad_adjustment;
#if REF_ENABLE_COST_PENALTY
  pl.cc0 = 0;
  pl.cc1 = 0;
  pl.cc2 = 0;
  pl.cc3 = 0;
  pl.packed_cost_table = (uint2)(0, 0);
  pl.cost_precision = 2;
  pl.packed_shape_cost = 0;
#endif
  return pl;
}

intel_sub_group_avc_ref_result_t
intel_sub_group_avc_ref_evaluate_with_single_reference(read_only image2d_t src_image,
                                                      read_only image2d_t ref_image,
                                                      sampler_t vme_media_sampler,
                                                      intel_sub_group_avc_ref_payload_t payload){
  uint src_grf0_dw7;
  uint src_grf0_dw6;
  uint src_grf0_dw5;
  uint src_grf0_dw4;
  uint src_grf0_dw3;
  uint src_grf0_dw2;
  uint src_grf0_dw1;
  uint src_grf0_dw0;
  uint src_grf1_dw7;
  uint src_grf1_dw6;
  uint src_grf1_dw5;
  uint src_grf1_dw4;
  uint src_grf1_dw3;
  uint src_grf1_dw2;
  uint src_grf1_dw1;
  uint src_grf1_dw0;
  uint src_grf2_dw7;
  uint src_grf2_dw6;
  uint src_grf2_dw5;
  uint src_grf2_dw4;
  uint src_grf2_dw3;
  uint src_grf2_dw2;
  uint src_grf2_dw1;
  uint src_grf2_dw0;
  uint src_grf3_dw7;
  uint src_grf3_dw6;
  uint src_grf3_dw5;
  uint src_grf3_dw4;
  uint src_grf3_dw3;
  uint src_grf3_dw2;
  uint src_grf3_dw1;
  uint src_grf3_dw0;
  uint src_grf4_dw7;
  uint src_grf4_dw6;
  uint src_grf4_dw5;
  uint src_grf4_dw4;
  uint src_grf4_dw3;
  uint src_grf4_dw2;
  uint src_grf4_dw1;
  uint src_grf4_dw0;
  uint src_grf5_dw7;
  uint src_grf5_dw6;
  uint src_grf5_dw5;
  uint src_grf5_dw4;
  uint src_grf5_dw3;
  uint src_grf5_dw2;
  uint src_grf5_dw1;
  uint src_grf5_dw0;
  uint src_grf6_dw7;
  uint src_grf6_dw6;
  uint src_grf6_dw5;
  uint src_grf6_dw4;
  uint src_grf6_dw3;
  uint src_grf6_dw2;
  uint src_grf6_dw1;
  uint src_grf6_dw0;
  uint src_grf7_dw7;
  uint src_grf7_dw6;
  uint src_grf7_dw5;
  uint src_grf7_dw4;
  uint src_grf7_dw3;
  uint src_grf7_dw2;
  uint src_grf7_dw1;
  uint src_grf7_dw0;


  //src_grf0_dw7 = Debug;
  src_grf0_dw7 = 0;
  //src_grf0_dw6 = Debug;
  src_grf0_dw6 = 0;
  //src_grf0_dw5 = (Ref_Height << 24) | (Ref_Width << 16) | (Ignored << 8) | (Dispatch_Id);
  src_grf0_dw5 = 0;
  //src_grf0_dw4 = Ignored;
  src_grf0_dw4 = 0;
  //src_grf0_dw3 = (Reserved << 31)                | (Sub_Mb_Part_Mask << 24)       | (Intra_SAD << 22)
  src_grf0_dw3 =   (0 << 31)                       | (0 << 24)                      | (0 << 22)
                 //| (Inter_SAD << 20)             | (BB_Skip_Enabled << 19)        | (Reserverd << 18)
                   | (payload.sad_adjustment << 20)| (0 << 19)                      | (0 << 18)
                 //| (Dis_Aligned_Src_Fetch << 17) | (Dis_Aligned_Ref_Fetch << 16)  | (Dis_Field_Cache_Alloc << 15)
                   | (0 << 17)                     | (0 << 16)                      | (0 << 15)
                 //| (Skip_Type << 14)             | (Sub_Pel_Mode << 12)           | (Dual_Search_Path_Opt << 11)
                   | (0 << 14)                     | (payload.pixel_mode << 12)     | (0 << 11)
                 //| (Search_Ctrl << 8)            | (Ref_Access << 7)              | (SrcAccess << 6)
                   | (0 << 8)                      | (0 << 7)                       | (0 << 6)
                 //| (Mb_Type_Remap << 4)          | (Reserved_Workaround << 3)     | (Reserved_Workaround << 2)
                   | (0 << 4)                      | (0 << 3)                       | (0 << 2)
                 //| (Src_Size);
                   | (0);
  //src_grf0_dw2 = (SrcY << 16) | (SrcX);
  src_grf0_dw2 = (payload.srcCoord.y << 16)  | (payload.srcCoord.x);
  //src_grf0_dw1 = (Ref1Y << 16)  | (Ref1X);
  src_grf0_dw1 = 0;
  //src_grf0_dw0 = (Ref0Y << 16)  | (Ref0X);
  src_grf0_dw0 = 0;
  

  /*src_grf1_dw7 = (Skip_Center_Mask << 24)         | (Reserved << 22)               | (Ref1_Field_Polarity << 21)
                 | (Ref0_Field_Polarity << 20)   | (Src_Field_Polarity << 19)     | (Bilinear_Enable << 18)
                 | (MV_Cost_Scale_Factor << 16)  | (Mb_Intra_Struct << 8)         | (Intra_Corner_Swap << 7)
                 | (Non_Skip_Mode_Added << 6)    | (Non_Skip_ZMv_Added << 5)      | (IntraPartMask);*/
  src_grf1_dw7 = 0;
  //src_grf1_dw6 = Reserved;
  src_grf1_dw6 = 0;
  /*src_grf1_dw5 = Reseverd for BDW+
  src_grf1_dw4 = Reseverd for BDW+*/
  src_grf1_dw5 = 0;
  src_grf1_dw4 = 0;
  //src_grf1_dw3 = Weighted SAD Control Sub-block 0...15
  src_grf1_dw3 = 0;
  //src_grf1_dw2 = (Start1Y << 28)                  | (Start1X << 24)                | (Start0Y << 20)
                 //| (Start0X << 16)               | (Max_Num_SU << 8)              | (LenSP);
  src_grf1_dw2 = 0;
  /*src_grf1_dw1 = (RepartEn << 31)                 | (FBPrunEn << 30)               | (AdaptiveValidationControl << 29)
                 | (Uni_Mix_Disable << 28)       | (Bi_Sub_Mb_Part_Mask << 24)    | (Reserverd << 22)
                 | (Bi_Weight << 16)             | (Reserved << 6)                | (MaxNumMVs);*/
  //src_grf1_dw1 = (0 << 24) | (2);
  src_grf1_dw1 = (0 << 24) | (16);
  /*src_grf1_dw0 = (Early_Ime_Stop << 24)           | (Early_Fme_Success << 16)      | (Skip_Success << 8)
                 | (T8x8_Flag_For_Inter_En << 7) | (Quit_Inter_En << 6)           | (Early_Ime_Success_En << 5)
                 | (Early_Success_En << 4)       | (Part_Candidate_En << 3)       | (Bi_Mix_Dis << 2)
                 | (Adaptive_En  << 1)           | (SkipModeEn);*/
  src_grf1_dw0 = 0;

  //src_grf2_dw7 = SIC Forward Transform Coeff Threshold Matrix[3...6]
  src_grf2_dw7 = 0;
  //src_grf2_dw6 = SIC Forward Transform Coeff Threshold Matrix[0...2]
  src_grf2_dw6 = 0;
  //src_grf2_dw5 = (Reserved << 24) | (FBR_SubPredMode_Input << 16) | (FBR_SubMBShape_Input << 8) | (Reserved << 2) | (FBR_MbMode_Input);
  src_grf2_dw5 = (0 << 24) | (payload.directions << 16) | (payload.minor_shapes << 8) | (payload.major_shape);
#if REF_ENABLE_COST_PENALTY
  //src_grf2_dw4 = MV_4_Cost ... MV_7_Cost;
  src_grf2_dw4 = payload.packed_cost_table.s1;
  //src_grf2_dw3 = MV_0_Cost ... MV_3_Cost;
  src_grf2_dw3 = payload.packed_cost_table.s0;
  //src_grf2_dw2 = ... Mode 8 Cost;
  src_grf2_dw2 = (payload.packed_shape_cost >> 32) & 0x000000ff;
  //src_grf2_dw1 = Mode 4 Cost ... Mode 7 Cost
  src_grf2_dw1 = payload.packed_shape_cost;
  src_grf2_dw0 = 0;
  //src_grf3_dw7 = (BWDCostCenter3Y << 16) | (BWDCostCenter3X) ;
  src_grf3_dw7 = payload.cc3 >> 32;
  //src_grf3_dw6 = (FWDCostCenter3Y << 16) | (FWDCostCenter3X) ;
  src_grf3_dw6 = payload.cc3;
  //src_grf3_dw5 = (BWDCostCenter2Y << 16) | (BWDCostCenter2X) ;
  src_grf3_dw5 = payload.cc2 >> 32;
  //src_grf3_dw4 = (FWDCostCenter2Y << 16) | (FWDCostCenter2X) ;
  src_grf3_dw4 = payload.cc2;
  //src_grf3_dw3 = (BWDCostCenter1Y << 16) | (BWDCostCenter1X) ;
  src_grf3_dw3 = payload.cc1 >> 32;
  //src_grf3_dw2 = (FWDCostCenter1Y << 16) | (FWDCostCenter1X) ;
  src_grf3_dw2 = payload.cc1;
  //src_grf3_dw1 = (BWDCostCenter0Y << 16) | (BWDCostCenter0X) ;
  src_grf3_dw1 = payload.cc0 >> 32;
  //src_grf3_dw0 = (FWDCostCenter0Y << 16) | (FWDCostCenter0X) ;
  src_grf3_dw0 = payload.cc0;
#else
  src_grf2_dw4 = 0;
  src_grf2_dw3 = 0;
  src_grf2_dw2 = 0;
  src_grf2_dw1 = 0;
  src_grf2_dw0 = 0;
  src_grf3_dw7 = 0;
  src_grf3_dw6 = 0;
  src_grf3_dw5 = 0;
  src_grf3_dw4 = 0;
  src_grf3_dw3 = 0;
  src_grf3_dw2 = 0;
  src_grf3_dw1 = 0;
  src_grf3_dw0 = 0;
#endif

  //grf4...grf7 =   Ref0/1 Sub-block XY 0...15
  int2 bi_mv_temp = as_int2( payload.mv );
  int2 bi_mv = intel_sub_group_shuffle(bi_mv_temp, 3);
  src_grf4_dw7 = bi_mv.s1;
  src_grf4_dw6 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 2);
  src_grf4_dw5 = bi_mv.s1;
  src_grf4_dw4 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 1);
  src_grf4_dw3 = bi_mv.s1;
  src_grf4_dw2 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 0);
  src_grf4_dw1 = bi_mv.s1;
  src_grf4_dw0 = bi_mv.s0;

  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 7);
  src_grf5_dw7 = bi_mv.s1;
  src_grf5_dw6 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 6);
  src_grf5_dw5 = bi_mv.s1;
  src_grf5_dw4 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 5);
  src_grf5_dw3 = bi_mv.s1;
  src_grf5_dw2 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 4);
  src_grf5_dw1 = bi_mv.s1;
  src_grf5_dw0 = bi_mv.s0;

  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 11);
  src_grf6_dw7 = bi_mv.s1;
  src_grf6_dw6 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 10);
  src_grf6_dw5 = bi_mv.s1;
  src_grf6_dw4 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 9);
  src_grf6_dw3 = bi_mv.s1;
  src_grf6_dw2 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 8);
  src_grf6_dw1 = bi_mv.s1;
  src_grf6_dw0 = bi_mv.s0;

  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 15);
  src_grf7_dw7 = bi_mv.s1;
  src_grf7_dw6 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 14);
  src_grf7_dw5 = bi_mv.s1;
  src_grf7_dw4 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 13);
  src_grf7_dw3 = bi_mv.s1;
  src_grf7_dw2 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 12);
  src_grf7_dw1 = bi_mv.s1;
  src_grf7_dw0 = bi_mv.s0;

  intel_sub_group_avc_ref_result_t ref_result;
  ref_result = __gen_ocl_ime(src_image, ref_image,
                src_grf0_dw7, src_grf0_dw6, src_grf0_dw5, src_grf0_dw4,
                src_grf0_dw3, src_grf0_dw2, src_grf0_dw1, src_grf0_dw0,
                src_grf1_dw7, src_grf1_dw6, src_grf1_dw5, src_grf1_dw4,
                src_grf1_dw3, src_grf1_dw2, src_grf1_dw1, src_grf1_dw0,
                src_grf2_dw7, src_grf2_dw6, src_grf2_dw5, src_grf2_dw4,
                src_grf2_dw3, src_grf2_dw2, src_grf2_dw1, src_grf2_dw0,
                src_grf3_dw7, src_grf3_dw6, src_grf3_dw5, src_grf3_dw4,
                src_grf3_dw3, src_grf3_dw2, src_grf3_dw1, src_grf3_dw0,
                src_grf4_dw7, src_grf4_dw6, src_grf4_dw5, src_grf4_dw4,
                src_grf4_dw3, src_grf4_dw2, src_grf4_dw1, src_grf4_dw0,
                src_grf5_dw7, src_grf5_dw6, src_grf5_dw5, src_grf5_dw4,
                src_grf5_dw3, src_grf5_dw2, src_grf5_dw1, src_grf5_dw0,
                src_grf6_dw7, src_grf6_dw6, src_grf6_dw5, src_grf6_dw4,
                src_grf6_dw3, src_grf6_dw2, src_grf6_dw1, src_grf6_dw0,
                src_grf7_dw7, src_grf7_dw6, src_grf7_dw5, src_grf7_dw4,
                src_grf7_dw3, src_grf7_dw2, src_grf7_dw1, src_grf7_dw0,
                //msg_type
                3);

  return ref_result;
}

ulong intel_sub_group_avc_ref_get_motion_vectors(intel_sub_group_avc_ref_result_t result){
  uint lid_x = get_sub_group_local_id();
  uint fwd_mv, bwd_mv;
  if(lid_x < 4){
    fwd_mv = intel_sub_group_shuffle(result.s0, 8 + lid_x*2);
    bwd_mv = intel_sub_group_shuffle(result.s0, 9 + lid_x*2);
  }
  else if(lid_x >= 4 && lid_x <= 12){
    fwd_mv = intel_sub_group_shuffle(result.s1, 0 + (lid_x-4)*2);
    bwd_mv = intel_sub_group_shuffle(result.s1, 1 + (lid_x-4)*2);
  }
  else if(lid_x < 16){
    fwd_mv = intel_sub_group_shuffle(result.s2, 0 + (lid_x-12)*2);
    bwd_mv = intel_sub_group_shuffle(result.s2, 1 + (lid_x-12)*2);
  }
  
  ulong res = (bwd_mv << 32) | (fwd_mv & 0x00000000ffffffff);
  return res;
}

ushort intel_sub_group_avc_ref_get_inter_distortions(intel_sub_group_avc_ref_result_t result){
  uint lid_x = get_sub_group_local_id();
  uint write_back_dw = intel_sub_group_shuffle(result.s2, 8 + lid_x/2);
  int start_bit = lid_x%2 * 16;
  ushort distortion = (write_back_dw >> start_bit);
  return distortion;
}

uint2 intel_sub_group_avc_mce_get_default_medium_penalty_cost_table(void){
  #define COST_PENALTY(idx, base, shift)  \
            uchar cost_penalty_##idx = (shift << 4) | (base);

  COST_PENALTY(0, 1, 0)
  COST_PENALTY(1, 1, 0)
  COST_PENALTY(2, 1, 0)
  COST_PENALTY(3, 1, 0)
  COST_PENALTY(4, 1, 0)
  COST_PENALTY(5, 1, 0)
  COST_PENALTY(6, 1, 0)
  COST_PENALTY(7, 1, 0)
  uint2 cost_table;
  cost_table.s0 = cost_penalty_0 | (cost_penalty_1 << 8) | ( cost_penalty_2 << 16) | (cost_penalty_3 << 24);
  cost_table.s1 = cost_penalty_4 | (cost_penalty_5 << 8) | ( cost_penalty_6 << 16) | (cost_penalty_7 << 24);
  return cost_table;
}

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_set_motion_vector_cost_function(ulong packed_cost_center_delta,
                                                        uint2 packed_cost_table,
                                                        uchar cost_precision,
                                                        intel_sub_group_avc_ime_payload_t payload){
  intel_sub_group_avc_ime_payload_t pl = payload;
  pl.packed_cost_table = packed_cost_table;
  pl.cost_precision = cost_precision;
  
  uint lid_x = get_sub_group_local_id();
  if(lid_x == 0)
    pl.cc0 = packed_cost_center_delta;
  else if(lid_x == 1)
    pl.cc1 = packed_cost_center_delta;
  else if(lid_x == 2)
    pl.cc2 = packed_cost_center_delta;
  else if(lid_x == 3)
    pl.cc3 = packed_cost_center_delta;
  else{
  }
  return pl;
}

#if REF_ENABLE_COST_PENALTY
intel_sub_group_avc_ref_payload_t
intel_sub_group_avc_ref_set_motion_vector_cost_function(ulong packed_cost_center_delta,
                                                        uint2 packed_cost_table,
                                                        uchar cost_precision,
                                                        intel_sub_group_avc_ref_payload_t payload){
  intel_sub_group_avc_ref_payload_t pl = payload;
  pl.packed_cost_table = packed_cost_table;
  pl.cost_precision = cost_precision;
  
  uint lid_x = get_sub_group_local_id();
  if(lid_x == 0)
    pl.cc0 = packed_cost_center_delta;
  else if(lid_x == 1)
    pl.cc1 = packed_cost_center_delta;
  else if(lid_x == 2)
    pl.cc2 = packed_cost_center_delta;
  else if(lid_x == 3)
    pl.cc3 = packed_cost_center_delta;
  else{
  }
  return pl;
}

#endif

intel_sub_group_avc_ime_payload_t
intel_sub_group_avc_ime_set_inter_shape_penalty(ulong packed_shape_cost,
                                                intel_sub_group_avc_ime_payload_t payload){
  intel_sub_group_avc_ime_payload_t pl = payload;
  pl.packed_shape_cost = packed_shape_cost;  
  return pl;
}

intel_sub_group_avc_sic_result_t
intel_sub_group_avc_sic_evaluate_ipe(read_only image2d_t src_image,
                                     sampler_t vme_media_sampler,
                                     intel_sub_group_avc_sic_payload_t payload){
  uint src_grf0_dw7;
  uint src_grf0_dw6;
  uint src_grf0_dw5;
  uint src_grf0_dw4;
  uint src_grf0_dw3;
  uint src_grf0_dw2;
  uint src_grf0_dw1;
  uint src_grf0_dw0;
  uint src_grf1_dw7;
  uint src_grf1_dw6;
  uint src_grf1_dw5;
  uint src_grf1_dw4;
  uint src_grf1_dw3;
  uint src_grf1_dw2;
  uint src_grf1_dw1;
  uint src_grf1_dw0;
  uint src_grf2_dw7;
  uint src_grf2_dw6;
  uint src_grf2_dw5;
  uint src_grf2_dw4;
  uint src_grf2_dw3;
  uint src_grf2_dw2;
  uint src_grf2_dw1;
  uint src_grf2_dw0;
  uint src_grf3_dw7;
  uint src_grf3_dw6;
  uint src_grf3_dw5;
  uint src_grf3_dw4;
  uint src_grf3_dw3;
  uint src_grf3_dw2;
  uint src_grf3_dw1;
  uint src_grf3_dw0;
  uint src_grf4_dw7;
  uint src_grf4_dw6;
  uint src_grf4_dw5;
  uint src_grf4_dw4;
  uint src_grf4_dw3;
  uint src_grf4_dw2;
  uint src_grf4_dw1;
  uint src_grf4_dw0;
  uint src_grf5_dw7;
  uint src_grf5_dw6;
  uint src_grf5_dw5;
  uint src_grf5_dw4;
  uint src_grf5_dw3;
  uint src_grf5_dw2;
  uint src_grf5_dw1;
  uint src_grf5_dw0;
  uint src_grf6_dw7;
  uint src_grf6_dw6;
  uint src_grf6_dw5;
  uint src_grf6_dw4;
  uint src_grf6_dw3;
  uint src_grf6_dw2;
  uint src_grf6_dw1;
  uint src_grf6_dw0;
  uint src_grf7_dw7;
  uint src_grf7_dw6;
  uint src_grf7_dw5;
  uint src_grf7_dw4;
  uint src_grf7_dw3;
  uint src_grf7_dw2;
  uint src_grf7_dw1;
  uint src_grf7_dw0;


  //src_grf0_dw7 = Debug;
  src_grf0_dw7 = 0;
  //src_grf0_dw6 = Debug;
  src_grf0_dw6 = 0;
  //src_grf0_dw5 = (Ref_Height << 24) | (Ref_Width << 16) | (Ignored << 8) | (Dispatch_Id);
  src_grf0_dw5 = 0;
  //src_grf0_dw4 = Ignored;
  src_grf0_dw4 = 0;
  //src_grf0_dw3 = (Reserved << 31)                | (Sub_Mb_Part_Mask << 24)       | (Intra_SAD << 22)
  src_grf0_dw3 =   (0 << 31)                       | (0 << 24)                      | (payload.intra_sad_adjustment << 22)
                 //| (Inter_SAD << 20)             | (BB_Skip_Enabled << 19)        | (Reserverd << 18)
                   | (0 << 20)                     | (0 << 19)                      | (0 << 18)
                 //| (Dis_Aligned_Src_Fetch << 17) | (Dis_Aligned_Ref_Fetch << 16)  | (Dis_Field_Cache_Alloc << 15)
                   | (0 << 17)                     | (0 << 16)                      | (0 << 15)
                 //| (Skip_Type << 14)             | (Sub_Pel_Mode << 12)           | (Dual_Search_Path_Opt << 11)
                   | (0 << 14)                     | (0 << 12)                      | (0 << 11)
                 //| (Search_Ctrl << 8)            | (Ref_Access << 7)              | (SrcAccess << 6)
                   | (0 << 8)                      | (0 << 7)                       | (0 << 6)
                 //| (Mb_Type_Remap << 4)          | (Reserved_Workaround << 3)     | (Reserved_Workaround << 2)
                   | (0 << 4)                      | (0 << 3)                       | (0 << 2)
                 //| (Src_Size);
                   | (0);
  //src_grf0_dw2 = (SrcY << 16) | (SrcX);
  src_grf0_dw2 = (payload.srcCoord.y<<16)  | (payload.srcCoord.x);
  //src_grf0_dw1 = (Ref1Y << 16)  | (Ref1X);
  src_grf0_dw1 = 0;
  //src_grf0_dw0 = (Ref0Y << 16)  | (Ref0X);
  src_grf0_dw0 = 0;

  //src_grf1_dw7 = (Skip_Center_Mask << 24)         | (Reserved << 22)               | (Ref1_Field_Polarity << 21)
  src_grf1_dw7 = (0 << 24)                          | (0 << 22)                      | (0 << 21)
                 //| (Ref0_Field_Polarity << 20)   | (Src_Field_Polarity << 19)     | (Bilinear_Enable << 18)
                 | (0 << 20)                       | (0 << 19)                      | (0 << 18)
                 //| (MV_Cost_Scale_Factor << 16)  | (Mb_Intra_Struct << 8)         | (Intra_Corner_Swap << 7)
                 | (0 << 16)                       | (payload.intra_neighbour_availabilty << 8)   | (0 << 7)
                 //| (Non_Skip_Mode_Added << 6)    | (Non_Skip_ZMv_Added << 5)      | (IntraPartMask);
                 | (0 << 6)                        | (0 << 5)                       | (payload.luma_intra_partition_mask);
  //src_grf1_dw6 = Reserved;
  src_grf1_dw6 = 0;
  /*src_grf1_dw5 = Reseverd for BDW+
  src_grf1_dw4 = Reseverd for BDW+*/
  src_grf1_dw5 = 0;
  src_grf1_dw4 = 0;
  //src_grf1_dw3 = Weighted SAD Control Sub-block 0...15
  src_grf1_dw3 = 0;
  //src_grf1_dw2 = (Start1Y << 28)                  | (Start1X << 24)                | (Start0Y << 20)
                 //| (Start0X << 16)               | (Max_Num_SU << 8)              | (LenSP);
  src_grf1_dw2 = 0;
  
  /*src_grf1_dw1 = (RepartEn << 31)                 | (FBPrunEn << 30)               | (AdaptiveValidationControl << 29)
                 | (Uni_Mix_Disable << 28)       | (Bi_Sub_Mb_Part_Mask << 24)    | (Reserverd << 22)
                 | (Bi_Weight << 16)             | (Reserved << 6)                | (MaxNumMVs);*/
  src_grf1_dw1 = 0;
  /*src_grf1_dw0 = (Early_Ime_Stop << 24)           | (Early_Fme_Success << 16)      | (Skip_Success << 8)
                 | (T8x8_Flag_For_Inter_En << 7) | (Quit_Inter_En << 6)           | (Early_Ime_Success_En << 5)
                 | (Early_Success_En << 4)       | (Part_Candidate_En << 3)       | (Bi_Mix_Dis << 2)
                 | (Adaptive_En  << 1)           | (SkipModeEn);*/
  src_grf1_dw0 = 0;

  //cost related
  src_grf2_dw7 = 0;
  src_grf2_dw6 = 0;
  src_grf2_dw5 = 0;
  src_grf2_dw4 = 0;
  src_grf2_dw3 = 0;
  src_grf2_dw2 = 0;
  src_grf2_dw1 = 0;
  //src_grf2_dw0 = (MODE_INTRA_4x4 << 24) | (MODE_INTRA_8x8 << 16) | (MODE_INTRA_16x16 << 8) | (MODE_INTRA_NONPRED);
  src_grf2_dw0 = payload.intra_shape_cost;
  src_grf3_dw7 = 0;
  src_grf3_dw6 = 0;
  src_grf3_dw5 = 0;
  src_grf3_dw4 = 0;
  src_grf3_dw3 = 0;
  src_grf3_dw2 = 0;
  src_grf3_dw1 = 0;
  src_grf3_dw0 = 0;

  //Ref* SkipCenter* Delta XY
  /*src_grf4_dw7 = Ref1_SkipCenter_3_Delta_XY;
  src_grf4_dw6 = Ref0_SkipCenter_3_Delta_XY;
  src_grf4_dw5 = Ref1_SkipCenter_2_Delta_XY;
  src_grf4_dw4 = Ref0_SkipCenter_3_Delta_XY;
  src_grf4_dw3 = Ref1_SkipCenter_1_Delta_XY;
  src_grf4_dw2 = Ref0_SkipCenter_1_Delta_XY;
  src_grf4_dw1 = Ref1_SkipCenter_0_Delta_XY;
  src_grf4_dw0 = (Ref0_Skip_Center_0_Delta_Y << 16)  | (Ref0_Skip_Center_0_Delta_X);*/
  src_grf4_dw7 = 0;
  src_grf4_dw6 = 0;
  src_grf4_dw5 = 0;
  src_grf4_dw4 = 0;
  src_grf4_dw3 = 0;
  src_grf4_dw2 = 0;
  src_grf4_dw1 = 0;
  src_grf4_dw0 = 0;

  //src_grf5_dw7 = Neighbor pixel Luma value [23, -1] to [20, -1];
  src_grf5_dw7 = payload.ur_20_23;
  //src_grf5_dw6 = Neighbor pixel Luma value [19, -1] to [16, -1];
  src_grf5_dw6 = payload.ur_16_19;
  //src_grf5_dw5 = Neighbor pixel Luma value [15, -1] to [12, -1];
  src_grf5_dw5 = payload.u_12_15;
  //src_grf5_dw4 = Neighbor pixel Luma value [11, -1] to [8, -1];
  src_grf5_dw4 = payload.u_8_11;
  //src_grf5_dw3 = Neighbor pixel Luma value [7, -1] to [4, -1];
  src_grf5_dw3 = payload.u_4_7;
  //src_grf5_dw2 = (Neighbor pixel Luma value [3, -1] << 24)    | (Neighbor pixel Luma value [2, -1] << 16)
                 //| (Neighbor pixel Luma value [1, -1] << 8)  | (Neighbor pixel Luma value [0, -1]);
  src_grf5_dw2 = payload.u_0_3;
  uchar mode_mask_16_16 = 0xf;
  ushort mode_mask_8_8 = 0x01ff, mode_mask_4_4 = 0x01ff;
  if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_ALL_INTEL){
    mode_mask_16_16 = 0;
    mode_mask_8_8 = 0;
    mode_mask_4_4 = 0;
  }
  else if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_16x16_INTEL){
    mode_mask_16_16 = 0;
  }
  else if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_8x8_INTEL){
    mode_mask_8_8 = 0;
  }
  else if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_4x4_INTEL){
    mode_mask_4_4 = 0;
  }
  //src_grf5_dw1 = (Corner_Neighbor_pixel_0 << 24)  | (Reserved << 10) | (IntraComputeType << 8)
                 //| (IntraChromaModeMask << 4) | (Intra16x16ModeMask);
  src_grf5_dw1 = (payload.upper_left_corner_luma_pixel << 24)  | (0 << 10) | (1 << 8) | (0xf << 4) | (mode_mask_16_16);
  //src_grf5_dw0 = (Reserved<<25)  | (Intra_8x8_Mode_Mask << 16)  | (Reserved<<9)  | (Intra_4x4_Mode_Mask);
  src_grf5_dw0 = (0<<25)  | (mode_mask_8_8 << 16)  | (0<<9)  | (mode_mask_4_4);
  //src_grf6_dw7 = (Reserved << 24) | (Penalty_4x4_non_DC << 16) | (Penalty_8x8_non_DC << 8) | (Penalty_16x16_non_DC);
  src_grf6_dw7 = 0;
  //src_grf6_dw6 = Reserved;
  src_grf6_dw6 = 0;
  //src_grf6_dw5 = (Reserved << 16) | (Neighbor pixel Chroma value CbCr pair [-1, -1]);
  src_grf6_dw5 = 0;
  //src_grf6_dw4 = (Intra_MxM_Pred_Mode_B15 << 28)    | (Intra_MxM_Pred_Mode_B14 << 24)  | (Intra_MxM_Pred_Mode_B11 << 20)
                 //| (Intra_MxM_Pred_Mode_B10 << 16) | (Intra_MxM_Pred_Mode_A15 << 12)  | (Intra_MxM_Pred_Mode_A13 << 8)
                 //| (Intra_MxM_Pred_Mode_A7 << 4)   | (Intra_MxM_Pred_Mode_A5);
  //XXX: Which value should be set to?
  src_grf6_dw4 = (2 << 28)    | (2 << 24)  | (2 << 20)
                 | (2 << 16) | (2 << 12)  | (2 << 8)
                 | (2 << 4)   | (2);
  //src_grf6_dw3 = (Corner_Neighbor_pixel_1 << 24)  | (Neighbor pixel Luma value [-1, 14] to [-1, 12]);
  src_grf6_dw3 = payload.l_12_15;
  //src_grf6_dw2 = Neighbor pixel Luma value [-1, 11] to [-1, 8];
  src_grf6_dw2 = payload.l_8_11;
  //src_grf6_dw1 = Neighbor pixel Luma value [-1, 7] to [-1, 4];
  src_grf6_dw1 = payload.l_4_7;
  //src_grf6_dw0 = (Neighbor pixel Luma value [-1, 3] << 24)    | (Neighbor pixel Luma value [-1, 2] << 16)
                 //| (Neighbor pixel Luma value [-1, 1] << 8)  | (Neighbor pixel Luma value [-1, 0]);
  src_grf6_dw0 = payload.l_0_3;


  //chroma related
  src_grf7_dw7 = 0;
  src_grf7_dw6 = 0;
  src_grf7_dw5 = 0;
  src_grf7_dw4 = 0;
  src_grf7_dw3 = 0;
  src_grf7_dw2 = 0;
  src_grf7_dw1 = 0;
  src_grf7_dw0 = 0;


  intel_sub_group_avc_sic_result_t ime_result;
  ime_result = __gen_ocl_ime(src_image, src_image,
                src_grf0_dw7, src_grf0_dw6, src_grf0_dw5, src_grf0_dw4,
                src_grf0_dw3, src_grf0_dw2, src_grf0_dw1, src_grf0_dw0,
                src_grf1_dw7, src_grf1_dw6, src_grf1_dw5, src_grf1_dw4,
                src_grf1_dw3, src_grf1_dw2, src_grf1_dw1, src_grf1_dw0,
                src_grf2_dw7, src_grf2_dw6, src_grf2_dw5, src_grf2_dw4,
                src_grf2_dw3, src_grf2_dw2, src_grf2_dw1, src_grf2_dw0,
                src_grf3_dw7, src_grf3_dw6, src_grf3_dw5, src_grf3_dw4,
                src_grf3_dw3, src_grf3_dw2, src_grf3_dw1, src_grf3_dw0,
                src_grf4_dw7, src_grf4_dw6, src_grf4_dw5, src_grf4_dw4,
                src_grf4_dw3, src_grf4_dw2, src_grf4_dw1, src_grf4_dw0,
                src_grf5_dw7, src_grf5_dw6, src_grf5_dw5, src_grf5_dw4,
                src_grf5_dw3, src_grf5_dw2, src_grf5_dw1, src_grf5_dw0,
                src_grf6_dw7, src_grf6_dw6, src_grf6_dw5, src_grf6_dw4,
                src_grf6_dw3, src_grf6_dw2, src_grf6_dw1, src_grf6_dw0,
                src_grf7_dw7, src_grf7_dw6, src_grf7_dw5, src_grf7_dw4,
                src_grf7_dw3, src_grf7_dw2, src_grf7_dw1, src_grf7_dw0,
                //msg_type
                1);

  return ime_result;
}

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_initialize(ushort2 src_coord ){
  intel_sub_group_avc_sic_payload_t pl;
  pl.srcCoord = src_coord;
  pl.intra_shape_cost = 0;
  return pl;
}

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_configure_ipe(uchar luma_intra_partition_mask,
                                      uchar intra_neighbour_availabilty,
                                      uchar left_edge_luma_pixels,
                                      uchar upper_left_corner_luma_pixel,
                                      uchar upper_edge_luma_pixels,
                                      uchar upper_right_edge_luma_pixels,
                                      uchar intra_sad_adjustment,
                                      intel_sub_group_avc_sic_payload_t payload ){
  intel_sub_group_avc_sic_payload_t pl = payload;
  pl.luma_intra_partition_mask = luma_intra_partition_mask;
  pl.intra_neighbour_availabilty = intra_neighbour_availabilty;
  uchar pixel[16];
  for(uint i = 0; i < 16; i++)
    pixel[i] = intel_sub_group_shuffle(left_edge_luma_pixels, i);

  pl.l_0_3 = (pixel[3] << 24) | (pixel[2] << 16) | (pixel[1] << 8) | (pixel[0]);
  pl.l_4_7 = (pixel[7] << 24) | (pixel[6] << 16) | (pixel[5] << 8) | (pixel[4]);
  pl.l_8_11 = (pixel[11] << 24) | (pixel[10] << 16) | (pixel[9] << 8) | (pixel[8]);
  pl.l_12_15 = (pixel[15] << 24) | (pixel[14] << 16) | (pixel[13] << 8) | (pixel[12]);

  for(uint i = 0; i < 16; i++)
    pixel[i] = intel_sub_group_shuffle(upper_edge_luma_pixels, i);
  pl.u_0_3 = (pixel[3] << 24) | (pixel[2] << 16) | (pixel[1] << 8) | (pixel[0]);
  pl.u_4_7 = (pixel[7] << 24) | (pixel[6] << 16) | (pixel[5] << 8) | (pixel[4]);
  pl.u_8_11 = (pixel[11] << 24) | (pixel[10] << 16) | (pixel[9] << 8) | (pixel[8]);
  pl.u_12_15 = (pixel[15] << 24) | (pixel[14] << 16) | (pixel[13] << 8) | (pixel[12]);

  for(uint i = 0; i < 8; i++)
    pixel[i] = intel_sub_group_shuffle(upper_right_edge_luma_pixels, i);
  pl.ur_16_19 = (pixel[3] << 24) | (pixel[2] << 16) | (pixel[1] << 8) | (pixel[0]);
  pl.ur_20_23 = (pixel[7] << 24) | (pixel[6] << 16) | (pixel[5] << 8) | (pixel[4]);

  pl.upper_left_corner_luma_pixel = upper_left_corner_luma_pixel;
  pl.intra_sad_adjustment = intra_sad_adjustment;
  return pl;
}

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_set_intra_luma_shape_penalty(uint packed_shape_cost,
                                                     intel_sub_group_avc_sic_payload_t payload ){
  intel_sub_group_avc_sic_payload_t pl = payload;
  pl.intra_shape_cost = packed_shape_cost;
  return pl;
}

intel_sub_group_avc_sic_result_t
intel_sub_group_avc_sic_evaluate_with_single_reference(read_only image2d_t src_image,
                                                      read_only image2d_t ref_image,
                                                      sampler_t vme_media_sampler,
                                                      intel_sub_group_avc_sic_payload_t payload){
  uint src_grf0_dw7;
  uint src_grf0_dw6;
  uint src_grf0_dw5;
  uint src_grf0_dw4;
  uint src_grf0_dw3;
  uint src_grf0_dw2;
  uint src_grf0_dw1;
  uint src_grf0_dw0;
  uint src_grf1_dw7;
  uint src_grf1_dw6;
  uint src_grf1_dw5;
  uint src_grf1_dw4;
  uint src_grf1_dw3;
  uint src_grf1_dw2;
  uint src_grf1_dw1;
  uint src_grf1_dw0;
  uint src_grf2_dw7;
  uint src_grf2_dw6;
  uint src_grf2_dw5;
  uint src_grf2_dw4;
  uint src_grf2_dw3;
  uint src_grf2_dw2;
  uint src_grf2_dw1;
  uint src_grf2_dw0;
  uint src_grf3_dw7;
  uint src_grf3_dw6;
  uint src_grf3_dw5;
  uint src_grf3_dw4;
  uint src_grf3_dw3;
  uint src_grf3_dw2;
  uint src_grf3_dw1;
  uint src_grf3_dw0;
  uint src_grf4_dw7;
  uint src_grf4_dw6;
  uint src_grf4_dw5;
  uint src_grf4_dw4;
  uint src_grf4_dw3;
  uint src_grf4_dw2;
  uint src_grf4_dw1;
  uint src_grf4_dw0;
  uint src_grf5_dw7;
  uint src_grf5_dw6;
  uint src_grf5_dw5;
  uint src_grf5_dw4;
  uint src_grf5_dw3;
  uint src_grf5_dw2;
  uint src_grf5_dw1;
  uint src_grf5_dw0;
  uint src_grf6_dw7;
  uint src_grf6_dw6;
  uint src_grf6_dw5;
  uint src_grf6_dw4;
  uint src_grf6_dw3;
  uint src_grf6_dw2;
  uint src_grf6_dw1;
  uint src_grf6_dw0;
  uint src_grf7_dw7;
  uint src_grf7_dw6;
  uint src_grf7_dw5;
  uint src_grf7_dw4;
  uint src_grf7_dw3;
  uint src_grf7_dw2;
  uint src_grf7_dw1;
  uint src_grf7_dw0;


  //src_grf0_dw7 = Debug;
  src_grf0_dw7 = 0;
  //src_grf0_dw6 = Debug;
  src_grf0_dw6 = 0;
  //src_grf0_dw5 = (Ref_Height << 24) | (Ref_Width << 16) | (Ignored << 8) | (Dispatch_Id);
  src_grf0_dw5 = 0;
  //src_grf0_dw4 = Ignored;
  src_grf0_dw4 = 0;
  //src_grf0_dw3 = (Reserved << 31)                | (Sub_Mb_Part_Mask << 24)       | (Intra_SAD << 22)
  src_grf0_dw3 =   (0 << 31)                       | (0 << 24)                      | (payload.intra_sad_adjustment << 22)
                 //| (Inter_SAD << 20)             | (BB_Skip_Enabled << 19)        | (Reserverd << 18)
                   | (payload.skip_sad_adjustment << 20)  | (0 << 19)                      | (0 << 18)
                 //| (Dis_Aligned_Src_Fetch << 17) | (Dis_Aligned_Ref_Fetch << 16)  | (Dis_Field_Cache_Alloc << 15)
                   | (0 << 17)                     | (0 << 16)                      | (0 << 15)
                 //| (Skip_Type << 14)             | (Sub_Pel_Mode << 12)           | (Dual_Search_Path_Opt << 11)
                   | (0 << 14)                     | (0 << 12)                      | (0 << 11)
                 //| (Search_Ctrl << 8)            | (Ref_Access << 7)              | (SrcAccess << 6)
                   | (0 << 8)                      | (0 << 7)                       | (0 << 6)
                 //| (Mb_Type_Remap << 4)          | (Reserved_Workaround << 3)     | (Reserved_Workaround << 2)
                   | (0 << 4)                      | (0 << 3)                       | (0 << 2)
                 //| (Src_Size);
                   | (0);
  src_grf0_dw3 |= payload.skip_block_partition_type;
  //Block-Based Skip Enabled
  if(payload.skip_block_partition_type == CLK_AVC_ME_SKIP_BLOCK_PARTITION_8x8_INTEL)
    src_grf0_dw3 |= (1 << 19);
  //src_grf0_dw2 = (SrcY << 16) | (SrcX);
  src_grf0_dw2 = (payload.srcCoord.y << 16)  | (payload.srcCoord.x);
  //src_grf0_dw1 = (Ref1Y << 16)  | (Ref1X);
  src_grf0_dw1 = 0;
  //src_grf0_dw0 = (Ref0Y << 16)  | (Ref0X);
  src_grf0_dw0 = 0;

  //src_grf1_dw7 = (Skip_Center_Mask << 24)         | (Reserved << 22)               | (Ref1_Field_Polarity << 21)
  src_grf1_dw7 = (0 << 24)                          | (0 << 22)                      | (0 << 21)
                 //| (Ref0_Field_Polarity << 20)   | (Src_Field_Polarity << 19)     | (Bilinear_Enable << 18)
                 | (0 << 20)                       | (0 << 19)                      | (0 << 18)
                 //| (MV_Cost_Scale_Factor << 16)  | (Mb_Intra_Struct << 8)         | (Intra_Corner_Swap << 7)
                 | (0 << 16)                       | (payload.intra_neighbour_availabilty << 8)   | (0 << 7)
                 //| (Non_Skip_Mode_Added << 6)    | (Non_Skip_ZMv_Added << 5)      | (IntraPartMask);
                 | (0 << 6)                        | (0 << 5)                       | (payload.luma_intra_partition_mask);
  src_grf1_dw7 |= payload.skip_motion_vector_mask;
  //src_grf1_dw6 = Reserved;
  src_grf1_dw6 = 0;
  /*src_grf1_dw5 = (Cost_Center1Y << 16)  | (Cost_Center1X);
  src_grf1_dw4 = (Cost_Center0Y << 16)  | (Cost_Center0X);
  src_grf1_dw3 = (Ime_Too_Good << 24 )  | (Ime_Too_Bad << 16)  | (Part_Tolerance_Thrhd << 8) | (FBPrunThrhd);*/
  src_grf1_dw5 = 0;
  src_grf1_dw4 = 0;
  src_grf1_dw3 = 0;
  //src_grf1_dw2 = (Start1Y << 28)                  | (Start1X << 24)                | (Start0Y << 20)
                 //| (Start0X << 16)               | (Max_Num_SU << 8)              | (LenSP);
  src_grf1_dw2 = 0;
  /*src_grf1_dw1 = (RepartEn << 31)                 | (FBPrunEn << 30)               | (AdaptiveValidationControl << 29)
                 | (Uni_Mix_Disable << 28)       | (Bi_Sub_Mb_Part_Mask << 24)    | (Reserverd << 22)
                 | (Bi_Weight << 16)             | (Reserved << 6)                | (MaxNumMVs);*/
  src_grf1_dw1 = (0 << 24) | (payload.bidirectional_weight << 16) | (16);
  /*src_grf1_dw0 = (Early_Ime_Stop << 24)           | (Early_Fme_Success << 16)      | (Skip_Success << 8)
                 | (T8x8_Flag_For_Inter_En << 7) | (Quit_Inter_En << 6)           | (Early_Ime_Success_En << 5)
                 | (Early_Success_En << 4)       | (Part_Candidate_En << 3)       | (Bi_Mix_Dis << 2)
                 | (Adaptive_En  << 1)           | (SkipModeEn);*/
  src_grf1_dw0 = 1;

  //src_grf2_dw7 = SIC Forward Transform Coeff Threshold Matrix[3...6]
  src_grf2_dw7 = 0;
  //src_grf2_dw6 = SIC Forward Transform Coeff Threshold Matrix[0...2]
  src_grf2_dw6 = 0;
  //src_grf2_dw5 = (Reserved << 24) | (FBR_SubPredMode_Input << 16) | (FBR_SubMBShape_Input << 8) | (Reserved << 2) | (FBR_MbMode_Input);
  src_grf2_dw5 = 0;
  //XXX: TO DO: setting mv cost related bit filed
  //src_grf2_dw4 = MV_4_Cost ... MV_7_Cost;
  src_grf2_dw4 = 0;
  //src_grf2_dw3 = MV_0_Cost ... MV_3_Cost;
  src_grf2_dw3 = 0;
  //src_grf2_dw2 = (Chroma_Intra_Mode_Cost << 24) | (RefID_Cost << 16) | (Mode_9_Cost << 8) | (Mode_8_Cost);
  src_grf2_dw2 = 0;
  //src_grf2_dw1 = Mode 4 Cost ... Mode 7 Cost
  src_grf2_dw1 = 0;
  //src_grf2_dw0 = (MODE_INTRA_4x4 << 24) | (MODE_INTRA_8x8 << 16) | (MODE_INTRA_16x16 << 8) | (MODE_INTRA_NONPRED);
  src_grf2_dw0 = payload.intra_shape_cost;
  /*
  //src_grf3_dw7 = (BWDCostCenter3Y << 16) | (BWDCostCenter3X) ;
  src_grf3_dw7 = payload.cc3 >> 32;
  //src_grf3_dw6 = (FWDCostCenter3Y << 16) | (FWDCostCenter3X) ;
  src_grf3_dw6 = payload.cc3;
  //src_grf3_dw5 = (BWDCostCenter2Y << 16) | (BWDCostCenter2X) ;
  src_grf3_dw5 = payload.cc2 >> 32;
  //src_grf3_dw4 = (FWDCostCenter2Y << 16) | (FWDCostCenter2X) ;
  src_grf3_dw4 = payload.cc2;
  //src_grf3_dw3 = (BWDCostCenter1Y << 16) | (BWDCostCenter1X) ;
  src_grf3_dw3 = payload.cc1 >> 32;
  //src_grf3_dw2 = (FWDCostCenter1Y << 16) | (FWDCostCenter1X) ;
  src_grf3_dw2 = payload.cc1;
  //src_grf3_dw1 = (BWDCostCenter0Y << 16) | (BWDCostCenter0X) ;
  src_grf3_dw1 = payload.cc0 >> 32;
  //src_grf3_dw0 = (FWDCostCenter0Y << 16) | (FWDCostCenter0X) ;
  src_grf3_dw0 = payload.cc0;*/
  src_grf3_dw7 = 0;
  src_grf3_dw6 = 0;
  src_grf3_dw5 = 0;
  src_grf3_dw4 = 0;
  src_grf3_dw3 = 0;
  src_grf3_dw2 = 0;
  src_grf3_dw1 = 0;
  src_grf3_dw0 = 0;

  //Ref1/Ref0 SkipCenter 3...0 Delta XY
  int2 bi_mv_temp = as_int2( payload.mv );
  int2 bi_mv = intel_sub_group_shuffle(bi_mv_temp, 3);
  src_grf4_dw7 = bi_mv.s1;
  src_grf4_dw6 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 2);
  src_grf4_dw5 = bi_mv.s1;
  src_grf4_dw4 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 1);
  src_grf4_dw3 = bi_mv.s1;
  src_grf4_dw2 = bi_mv.s0;
  bi_mv = intel_sub_group_shuffle(bi_mv_temp, 0);
  src_grf4_dw1 = bi_mv.s1;
  src_grf4_dw0 = bi_mv.s0;
 
  //src_grf5_dw7 = Neighbor pixel Luma value [23, -1] to [20, -1];
  src_grf5_dw7 = payload.ur_20_23;
  //src_grf5_dw6 = Neighbor pixel Luma value [19, -1] to [16, -1];
  src_grf5_dw6 = payload.ur_16_19;
  //src_grf5_dw5 = Neighbor pixel Luma value [15, -1] to [12, -1];
  src_grf5_dw5 = payload.u_12_15;
  //src_grf5_dw4 = Neighbor pixel Luma value [11, -1] to [8, -1];
  src_grf5_dw4 = payload.u_8_11;
  //src_grf5_dw3 = Neighbor pixel Luma value [7, -1] to [4, -1];
  src_grf5_dw3 = payload.u_4_7;
  //src_grf5_dw2 = (Neighbor pixel Luma value [3, -1] << 24)    | (Neighbor pixel Luma value [2, -1] << 16)
                 //| (Neighbor pixel Luma value [1, -1] << 8)  | (Neighbor pixel Luma value [0, -1]);
  src_grf5_dw2 = payload.u_0_3;
  uchar mode_mask_16_16 = 0xf;
  ushort mode_mask_8_8 = 0x01ff, mode_mask_4_4 = 0x01ff;
  if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_ALL_INTEL){
    mode_mask_16_16 = 0;
    mode_mask_8_8 = 0;
    mode_mask_4_4 = 0;
  }
  else if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_16x16_INTEL){
    mode_mask_16_16 = 0;
  }
  else if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_8x8_INTEL){
    mode_mask_8_8 = 0;
  }
  else if(payload.luma_intra_partition_mask == CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_4x4_INTEL){
    mode_mask_4_4 = 0;
  }
  //src_grf5_dw1 = (Corner_Neighbor_pixel_0 << 24)  | (Reserved << 10) | (IntraComputeType << 8)
                 //| (IntraChromaModeMask << 4) | (Intra16x16ModeMask);
  src_grf5_dw1 = (payload.upper_left_corner_luma_pixel << 24)  | (0 << 10) | (1 << 8) | (0xf << 4) | (mode_mask_16_16);
  //src_grf5_dw1 = (payload.upper_left_corner_luma_pixel << 24)  | (0 << 10) | (1 << 8) | (0xf << 4) | (0xb);
  //src_grf5_dw0 = (Reserved<<25)  | (Intra_8x8_Mode_Mask << 16)  | (Reserved<<9)  | (Intra_4x4_Mode_Mask);
  src_grf5_dw0 = (0<<25)  | (mode_mask_8_8 << 16)  | (0<<9)  | (mode_mask_4_4);
  //src_grf6_dw7 = (Reserved << 24) | (Penalty_4x4_non_DC << 16) | (Penalty_8x8_non_DC << 8) | (Penalty_16x16_non_DC);
  src_grf6_dw7 = 0;
  //src_grf6_dw6 = Reserved;
  src_grf6_dw6 = 0;
  //src_grf6_dw5 = (Reserved << 16) | (Neighbor pixel Chroma value CbCr pair [-1, -1]);
  src_grf6_dw5 = 0;
  //src_grf6_dw4 = (Intra_MxM_Pred_Mode_B15 << 28)    | (Intra_MxM_Pred_Mode_B14 << 24)  | (Intra_MxM_Pred_Mode_B11 << 20)
                 //| (Intra_MxM_Pred_Mode_B10 << 16) | (Intra_MxM_Pred_Mode_A15 << 12)  | (Intra_MxM_Pred_Mode_A13 << 8)
                 //| (Intra_MxM_Pred_Mode_A7 << 4)   | (Intra_MxM_Pred_Mode_A5);
  //XXX: Which value should be set to?
  src_grf6_dw4 = (2 << 28)    | (2 << 24)  | (2 << 20)
                 | (2 << 16) | (2 << 12)  | (2 << 8)
                 | (2 << 4)   | (2);
  //src_grf6_dw3 = (Corner_Neighbor_pixel_1 << 24)  | (Neighbor pixel Luma value [-1, 14] to [-1, 12]);
  src_grf6_dw3 = payload.l_12_15;
  //src_grf6_dw2 = Neighbor pixel Luma value [-1, 11] to [-1, 8];
  src_grf6_dw2 = payload.l_8_11;
  //src_grf6_dw1 = Neighbor pixel Luma value [-1, 7] to [-1, 4];
  src_grf6_dw1 = payload.l_4_7;
  //src_grf6_dw0 = (Neighbor pixel Luma value [-1, 3] << 24)    | (Neighbor pixel Luma value [-1, 2] << 16)
                 //| (Neighbor pixel Luma value [-1, 1] << 8)  | (Neighbor pixel Luma value [-1, 0]);
  src_grf6_dw0 = payload.l_0_3;


  //chroma related
  src_grf7_dw7 = 0;
  src_grf7_dw6 = 0;
  src_grf7_dw5 = 0;
  src_grf7_dw4 = 0;
  src_grf7_dw3 = 0;
  src_grf7_dw2 = 0;
  src_grf7_dw1 = 0;
  src_grf7_dw0 = 0;


  intel_sub_group_avc_ref_result_t sic_result;
  sic_result = __gen_ocl_ime(src_image, ref_image,
                src_grf0_dw7, src_grf0_dw6, src_grf0_dw5, src_grf0_dw4,
                src_grf0_dw3, src_grf0_dw2, src_grf0_dw1, src_grf0_dw0,
                src_grf1_dw7, src_grf1_dw6, src_grf1_dw5, src_grf1_dw4,
                src_grf1_dw3, src_grf1_dw2, src_grf1_dw1, src_grf1_dw0,
                src_grf2_dw7, src_grf2_dw6, src_grf2_dw5, src_grf2_dw4,
                src_grf2_dw3, src_grf2_dw2, src_grf2_dw1, src_grf2_dw0,
                src_grf3_dw7, src_grf3_dw6, src_grf3_dw5, src_grf3_dw4,
                src_grf3_dw3, src_grf3_dw2, src_grf3_dw1, src_grf3_dw0,
                src_grf4_dw7, src_grf4_dw6, src_grf4_dw5, src_grf4_dw4,
                src_grf4_dw3, src_grf4_dw2, src_grf4_dw1, src_grf4_dw0,
                src_grf5_dw7, src_grf5_dw6, src_grf5_dw5, src_grf5_dw4,
                src_grf5_dw3, src_grf5_dw2, src_grf5_dw1, src_grf5_dw0,
                src_grf6_dw7, src_grf6_dw6, src_grf6_dw5, src_grf6_dw4,
                src_grf6_dw3, src_grf6_dw2, src_grf6_dw1, src_grf6_dw0,
                src_grf7_dw7, src_grf7_dw6, src_grf7_dw5, src_grf7_dw4,
                src_grf7_dw3, src_grf7_dw2, src_grf7_dw1, src_grf7_dw0,
                //msg_type
                1);

  return sic_result;
}

intel_sub_group_avc_sic_payload_t
intel_sub_group_avc_sic_configure_skc(uint skip_block_partition_type,
                                      uint skip_motion_vector_mask,
                                      ulong motion_vectors,
                                      char bidirectional_weight,
                                      uchar skip_sad_adjustment,
                                      intel_sub_group_avc_sic_payload_t payload){
  intel_sub_group_avc_sic_payload_t pl = payload;
  pl.skip_block_partition_type = skip_block_partition_type;
  pl.skip_motion_vector_mask = skip_motion_vector_mask;
  pl.bidirectional_weight = bidirectional_weight;
  pl.skip_sad_adjustment = skip_sad_adjustment;
  pl.mv = motion_vectors;
  return pl;
}

ushort
intel_sub_group_avc_sic_get_inter_distortions(intel_sub_group_avc_sic_result_t result){
  uint lid_x = get_sub_group_local_id();
  uint write_back_dw = intel_sub_group_shuffle(result.s2, 8 + lid_x/2);
  int start_bit = lid_x%2 * 16;
  ushort distortion = (write_back_dw >> start_bit);
  return distortion;
}

uchar
intel_sub_group_avc_sic_get_ipe_luma_shape(intel_sub_group_avc_sic_result_t result){
  uint write_back_dw00 = intel_sub_group_shuffle(result.s0, 0);
  uchar luma_shape = write_back_dw00 & 0x03;
  return luma_shape;
}

ushort
intel_sub_group_avc_sic_get_best_ipe_luma_distortion(intel_sub_group_avc_sic_result_t result){
  uint write_back_dw03 = intel_sub_group_shuffle(result.s0, 3);
  ushort luma_distortion = write_back_dw03;
  return luma_distortion;
}

ulong intel_sub_group_avc_sic_get_packed_ipe_luma_modes(intel_sub_group_avc_sic_result_t result){
  uint write_back_dw00 = intel_sub_group_shuffle(result.s0, 0);
  uchar luma_shape = write_back_dw00 & 0x03;
  ulong luma_modes = 0;
  uint write_back_dw04 = intel_sub_group_shuffle(result.s0, 4);
  uint write_back_dw05 = intel_sub_group_shuffle(result.s0, 5);
  if(luma_shape == CLK_AVC_ME_INTRA_16x16_INTEL)
    luma_modes |= (write_back_dw04 & 0x03);
  else if(luma_shape == CLK_AVC_ME_INTRA_8x8_INTEL){
    ulong modes_temp = write_back_dw04;
    luma_modes = (modes_temp & 0x0f) | ((modes_temp & 0x00f0) << 12) | ((modes_temp & 0x0f00) << 24) | ((modes_temp & 0x0000f000) << 36);
  }
  else if(luma_shape == CLK_AVC_ME_INTRA_4x4_INTEL){
    ulong modes_temp = write_back_dw05;
    luma_modes = (modes_temp << 32) | (write_back_dw04 & 0x00000000ffffffff);
  }
  return luma_modes;
}

bool __gen_ocl_in_local(size_t p) {
  bool cond1 = p > 0;
  bool cond2 = p < 64*1024;
  return cond1 && cond2;
}

#if (__OPENCL_C_VERSION__ >= 200)
local void *__to_local(generic void *p) {
  bool cond = __gen_ocl_in_local((size_t)p);
  return cond ? (local void*)p : NULL;
}
private void *__to_private(generic void *p) {
  bool cond = __gen_ocl_in_private((size_t)p);
  return cond ? (private void*)p : NULL;
}

global void *__to_global(generic void *p) {
  bool cond1 = __gen_ocl_in_local((size_t)p);
  bool cond2 = __gen_ocl_in_private((size_t)p);
  bool cond = cond1 || cond2;
  return !cond ? (global void*)p : NULL;
}
#endif
