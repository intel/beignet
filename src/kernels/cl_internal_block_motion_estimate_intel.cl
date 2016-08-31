typedef struct _motion_estimation_desc_intel {
  uint mb_block_type;
  uint subpixel_mode;
  uint sad_adjust_mode;
  uint search_path_type;
} accelerator_intel_t;

__kernel __attribute__((reqd_work_group_size(16,1,1)))
void block_motion_estimate_intel(accelerator_intel_t accel,
                                 __read_only  image2d_t src_image,
                                 __read_only  image2d_t ref_image,
                                 __global short2 * prediction_motion_vector_buffer,
                                 __global short2 * motion_vector_buffer,
                                 __global ushort * residuals){

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

  uint8 vme_result = (0);

  int lgid_x = get_group_id(0);
  int lgid_y = get_group_id(1);

  int num_groups_x = get_num_groups(0);
  int index = lgid_y * num_groups_x + lgid_x;

  uint2 srcCoord = 0;
  short2 predict_mv = 0;
  if(prediction_motion_vector_buffer != NULL){
    predict_mv = prediction_motion_vector_buffer[index];
    predict_mv.x = predict_mv.x / 4;
    predict_mv.y = predict_mv.y / 4;
  }

  srcCoord.x = lgid_x * 16;
  srcCoord.y = lgid_y * 16;

  //CL_ME_SEARCH_PATH_RADIUS_2_2_INTEL
  if(accel.search_path_type == 0x0){
    //src_grf0_dw5 = (Ref_Height << 24) | (Ref_Width << 16) | (Ignored << 8) | (Dispatch_Id);
    src_grf0_dw5 =   (20 << 24)         | (20 << 16)        | (0 << 8)       | (0);
    //src_grf0_dw1 = (Ref1Y << 16)  | (Ref1X);
    src_grf0_dw1 =   ((-2 + predict_mv.y) << 16 ) | ((-2 + predict_mv.x) & 0x0000ffff);
    //src_grf0_dw0 = (Ref0Y << 16)  | (Ref0X);
    src_grf0_dw0 =   ((-2 + predict_mv.y) << 16 ) | ((-2 + predict_mv.x) & 0x0000ffff);
    //src_grf1_dw2 = (Start1Y << 28)                  | (Start1X << 24)                | (Start0Y << 20)
    src_grf1_dw2 =   (0 << 28)                        | (0 << 24)                      | (0 << 20)
                   //| (Start0X << 16)               | (Max_Num_SU << 8)              | (LenSP);
                     | (0 << 16)                     | (2 << 8)                       | (2);
  }
  //CL_ME_SEARCH_PATH_RADIUS_4_4_INTEL
  else if(accel.search_path_type == 0x1){
    src_grf0_dw5 =   (24 << 24)         | (24 << 16)        | (0 << 8)       | (0);
    src_grf0_dw1 =   ((-4 + predict_mv.y) << 16 ) | ((-4 + predict_mv.x) & 0x0000ffff);
    src_grf0_dw0 =   ((-4 + predict_mv.y) << 16 ) | ((-4 + predict_mv.x) & 0x0000ffff);
    src_grf1_dw2 =   (0 << 28)                        | (0 << 24)                      | (0 << 20)
                     | (0 << 16)                     | (48 << 8)                       | (48);
  }
  //CL_ME_SEARCH_PATH_RADIUS_16_12_INTEL
  else if(accel.search_path_type == 0x5){
    src_grf0_dw5 =   (40 << 24)         | (48 << 16)        | (0 << 8)       | (0);
    src_grf0_dw1 =   ((-12 + predict_mv.y) << 16 ) | ((-16 + predict_mv.x) & 0x0000ffff);
    src_grf0_dw0 =   ((-12 + predict_mv.y) << 16 ) | ((-16 +  + predict_mv.x) & 0x0000ffff);
    src_grf1_dw2 =   (0 << 28)                        | (0 << 24)                      | (0 << 20)
                     | (0 << 16)                     | (48 << 8)                       | (48);
  }

  /*Deal with mb_block_type & sad_adjust_mode & subpixel_mode*/
  uchar sub_mb_part_mask = 0;
  //CL_ME_MB_TYPE_16x16_INTEL
  if(accel.mb_block_type == 0x0)
    sub_mb_part_mask = 0x7e;
  //CL_ME_MB_TYPE_8x8_INTEL
  else if(accel.mb_block_type == 0x1)
    sub_mb_part_mask = 0x77;
  //CL_ME_MB_TYPE_4x4_INTEL
  else if(accel.mb_block_type == 0x2)
    sub_mb_part_mask = 0x3f;

  uchar inter_sad = 0;
  //CL_ME_SAD_ADJUST_MODE_NONE_INTEL
  if(accel.sad_adjust_mode == 0x0)
    inter_sad = 0;
  //CL_ME_SAD_ADJUST_MODE_HAAR_INTEL
  else if(accel.sad_adjust_mode == 0x1)
    inter_sad = 2;

  uchar sub_pel_mode = 0;
  //CL_ME_SUBPIXEL_MODE_INTEGER_INTEL
  if(accel.subpixel_mode == 0x0)
    sub_pel_mode = 0;
  //CL_ME_SUBPIXEL_MODE_HPEL_INTEL
  else if(accel.subpixel_mode == 0x1)
    sub_pel_mode = 1;
  //CL_ME_SUBPIXEL_MODE_QPEL_INTEL
  else if(accel.subpixel_mode == 0x2)
    sub_pel_mode = 3;

  //src_grf0_dw3 = (Reserved << 31)                | (Sub_Mb_Part_Mask << 24)       | (Intra_SAD << 22)
  src_grf0_dw3 =   (0 << 31)                       | (sub_mb_part_mask << 24)       | (0 << 22)
                 //| (Inter_SAD << 20)             | (BB_Skip_Enabled << 19)        | (Reserverd << 18)
                   | (inter_sad << 20)             | (0 << 19)                      | (0 << 18)
                 //| (Dis_Aligned_Src_Fetch << 17) | (Dis_Aligned_Ref_Fetch << 16)  | (Dis_Field_Cache_Alloc << 15)
                   | (0 << 17)                     | (0 << 16)                      | (0 << 15)
                 //| (Skip_Type << 14)             | (Sub_Pel_Mode << 12)           | (Dual_Search_Path_Opt << 11)
                   | (0 << 14)                     | (sub_pel_mode << 12)           | (0 << 11)
                 //| (Search_Ctrl << 8)            | (Ref_Access << 7)              | (SrcAccess << 6)
                   | (0 << 8)                      | (0 << 7)                       | (0 << 6)
                 //| (Mb_Type_Remap << 4)          | (Reserved_Workaround << 3)     | (Reserved_Workaround << 2)
                   | (0 << 4)                      | (0 << 3)                       | (0 << 2)
                 //| (Src_Size);
                   | (0);


  //src_grf0_dw7 = Debug;
  src_grf0_dw7 = 0;
  //src_grf0_dw6 = Debug;
  src_grf0_dw6 = 0;
  //src_grf0_dw5 = (Ref_Height << 24) | (Ref_Width << 16) | (Ignored << 8) | (Dispatch_Id?);
  //src_grf0_dw4 = Ignored;
  src_grf0_dw4 = 0;

  //src_grf0_dw2 = (SrcY << 16) | (SrcX);
  src_grf0_dw2 = (srcCoord.y << 16)  | (srcCoord.x);
  //src_grf0_dw1 = (Ref1Y << 16)  | (Ref1X);
  //src_grf0_dw0 = (Ref0Y << 16)  | (Ref0X);
  /*src_grf1_dw7 = (Skip_Center_Mask << 24)         | (Reserved << 22)               | (Ref1_Field_Polarity << 21)
                 | (Ref0_Field_Polarity << 20)   | (Src_Field_Polarity << 19)     | (Bilinear_Enable << 18)
                 | (MV_Cost_Scale_Factor << 16)  | (Mb_Intra_Struct << 8)         | (Intra_Corner_Swap << 7)
                 | (Non_Skip_Mode_Added << 6)    | (Non_Skip_ZMv_Added << 5)      | (IntraPartMask);*/
  src_grf1_dw7 = 0;
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
  /*src_grf2_dw7 = Ref1_SkipCenter_3_Delta_XY;
  src_grf2_dw6 = Ref0_SkipCenter_3_Delta_XY;
  src_grf2_dw5 = Ref1_SkipCenter_2_Delta_XY;
  src_grf2_dw4 = Ref0_SkipCenter_3_Delta_XY;
  src_grf2_dw3 = Ref1_SkipCenter_1_Delta_XY;
  src_grf2_dw2 = Ref0_SkipCenter_1_Delta_XY;
  src_grf2_dw1 = Ref1_SkipCenter_0_Delta_XY;
  src_grf2_dw0 = (Ref0_Skip_Center_0_Delta_Y << 16)  | (Ref0_Skip_Center_0_Delta_X);
  src_grf3_dw7 = Neighbor pixel Luma value [23, -1] to [20, -1];
  src_grf3_dw6 = Neighbor pixel Luma value [19, -1] to [16, -1];
  src_grf3_dw5 = Neighbor pixel Luma value [15, -1] to [12, -1];
  src_grf3_dw4 = Neighbor pixel Luma value [11, -1] to [8, -1];
  src_grf3_dw3 = Neighbor pixel Luma value [7, -1] to [4, -1];
  src_grf3_dw2 = (Neighbor pixel Luma value [3, -1] << 24)    | (Neighbor pixel Luma value [2, -1] << 16)
                 | (Neighbor pixel Luma value [1, -1] << 8)  | (Neighbor pixel Luma value [0, -1]);
  //src_grf3_dw1 = (?)  | (Reserved)  | ((Intra_16x16_Mode_Mask);
  src_grf3_dw0 = (Reserved<<25)  | (Intra_16x16_Mode_Mask << 16)  | (Reserved)  | (Intra_16x16_Mode_Mask);
  src_grf4_dw7 = Reserved;
  src_grf4_dw6 = Reserved;
  src_grf4_dw5 = Reserved;
  src_grf4_dw4 = (Intra_MxM_Pred_Mode_B15 << 28)    | (Intra_MxM_Pred_Mode_B14 << 24)  | (Intra_MxM_Pred_Mode_B11 << 20)
                 | (Intra_MxM_Pred_Mode_B10 << 16) | (Intra_MxM_Pred_Mode_A15 << 12)  | (Intra_MxM_Pred_Mode_A13 << 8)
                 | (Intra_MxM_Pred_Mode_A7 << 4)   | (Intra_MxM_Pred_Mode_A5);
  //src_grf4_dw3 = (?)  | (Neighbor pixel Luma value [-1, 14] to [-1, 12]);
  src_grf4_dw2 = Neighbor pixel Luma value [-1, 11] to [-1, 8];
  src_grf4_dw1 = Neighbor pixel Luma value [-1, 7] to [-1, 4];
  src_grf4_dw0 = (Neighbor pixel Luma value [-1, 3] << 24)    | (Neighbor pixel Luma value [-1, 2] << 16)
                 | (Neighbor pixel Luma value [-1, 1] << 8)  | (Neighbor pixel Luma value [-1, 0]);*/
  src_grf2_dw7 = 0;
  src_grf2_dw6 = 0;
  src_grf2_dw5 = 0;
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
  src_grf4_dw7 = 0;
  src_grf4_dw6 = 0;
  src_grf4_dw5 = 0;
  src_grf4_dw4 = 0;
  src_grf4_dw3 = 0;
  src_grf4_dw2 = 0;
  src_grf4_dw1 = 0;
  src_grf4_dw0 = 0;

  int lid_x = get_local_id(0);

  vme_result = __gen_ocl_vme(src_image, ref_image,
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
                //msg_type, vme_search_path_lut, lut_sub,
                1, 0, 0);

  barrier(CLK_LOCAL_MEM_FENCE);

  short2 mv[16];
  ushort res[16];

  uint write_back_dwx;
  uint simd_width = get_max_sub_group_size();

  /* In simd 8 mode, one kernel variable 'uint' map to 8 dword.
   * In simd 16 mode, one kernel variable 'uint' map to 16 dword.
   * That's why we should treat simd8 and simd16 differently when
   * use __gen_ocl_region.
   * */
  if(simd_width == 8){
    write_back_dwx = __gen_ocl_region(0, vme_result.s1);
    mv[0] = as_short2( write_back_dwx );

    if(accel.mb_block_type > 0x0){
      for(int i = 2, j = 1; j < 4; i += 2, j++){
        write_back_dwx = __gen_ocl_region(i, vme_result.s1);
        mv[j] = as_short2( write_back_dwx );
      }
      if(accel.mb_block_type > 0x1){
        for(int i = 0, j = 4; j < 8; i += 2, j++){
          write_back_dwx = __gen_ocl_region(i, vme_result.s2);
          mv[j] = as_short2( write_back_dwx );
        }
        for(int i = 0, j = 8; j < 12; i += 2, j++){
          write_back_dwx = __gen_ocl_region(i, vme_result.s3);
          mv[j] = as_short2( write_back_dwx );
        }
        for(int i = 0, j = 12; j < 16; i += 2, j++){
          write_back_dwx = __gen_ocl_region(i, vme_result.s4);
          mv[j] = as_short2( write_back_dwx );
        }
      }
    }
    ushort2 temp_res;
    for(int i = 0; i < 8; i++){
      write_back_dwx = __gen_ocl_region(i, vme_result.s5);
      temp_res = as_ushort2(write_back_dwx);
      res[i*2] = temp_res.s0;
      res[i*2+1] = temp_res.s1;
    }
  }
  else if(simd_width == 16){
    write_back_dwx = __gen_ocl_region(0 + 8, vme_result.s0);
    mv[0] = as_short2( write_back_dwx );

    if(accel.mb_block_type > 0x0){
      for(int i = 2, j = 1; j < 4; i += 2, j++){
        write_back_dwx = __gen_ocl_region(i + 8, vme_result.s0);
        mv[j] = as_short2( write_back_dwx );
      }
      if(accel.mb_block_type > 0x1){
        for(int i = 0, j = 4; j < 8; i += 2, j++){
          write_back_dwx = __gen_ocl_region(i, vme_result.s1);
          mv[j] = as_short2( write_back_dwx );
        }
        for(int i = 0, j = 8; j < 12; i += 2, j++){
          write_back_dwx = __gen_ocl_region(i + 8, vme_result.s1);
          mv[j] = as_short2( write_back_dwx );
        }
        for(int i = 0, j = 12; j < 16; i += 2, j++){
          write_back_dwx = __gen_ocl_region(i, vme_result.s2);
          mv[j] = as_short2( write_back_dwx );
        }
      }
    }
    ushort2 temp_res;
    for(int i = 0; i < 8; i++){
      write_back_dwx = __gen_ocl_region(i + 8, vme_result.s2);
      temp_res = as_ushort2(write_back_dwx);
      res[i*2] = temp_res.s0;
      res[i*2+1] = temp_res.s1;
    }
  }

  int mv_index;

  //CL_ME_MB_TYPE_16x16_INTEL
  if(accel.mb_block_type == 0x0){
    mv_index = index * 1;
    if( lid_x == 0 ){
      motion_vector_buffer[mv_index] = mv[lid_x];
      if(residuals)
        residuals[mv_index] = 2 * res[lid_x];
    }
  }
  //CL_ME_MB_TYPE_8x8_INTEL
  else if(accel.mb_block_type == 0x1){
    if(lid_x < 4){
      mv_index = lgid_y * num_groups_x * 4 + lgid_x * 2;
      mv_index = mv_index + num_groups_x * 2 * (lid_x / 2) + (lid_x % 2);
      motion_vector_buffer[mv_index] = mv[lid_x];
      if(residuals)
        residuals[mv_index] = 2 * res[lid_x];
    }
  }
  //CL_ME_MB_TYPE_4x4_INTEL
  else if(accel.mb_block_type == 0x2){
    if(lid_x < 16){
      mv_index = lgid_y * num_groups_x * 16 + lgid_x * 4;
      mv_index = mv_index + num_groups_x * 4 * (lid_x / 4) + (lid_x % 4);
      motion_vector_buffer[mv_index] = mv[lid_x];
      if(residuals)
        residuals[mv_index] = 2 * res[lid_x];
    }
  }

}
