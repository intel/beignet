
__kernel __attribute__((intel_reqd_sub_group_size(16)))
void  compiler_block_motion_estimate_intel(
    __read_only image2d_t   src_img,
    __read_only image2d_t   ref_img,
    __global short2*        motion_vector_buffer,
    __global ushort*        residuals_buffer,
    __global uchar*         mj_shape_buffer,
    __global uchar*         mn_shapes_buffer,
    __global uchar*         directions_buffer,
    __global uint*          dwo_buffer,
    __global uint*          pld_buffer) {

  int gr_id0 = get_group_id(0);
  int gr_id1 = get_group_id(1);

  ushort2 src_coord = 0;
  src_coord.x = gr_id0 * 16;
  src_coord.y = gr_id1 * 16;
  uchar partition_mask = CLK_AVC_ME_PARTITION_MASK_16x16_INTEL;
  uchar sad_adjustment = CLK_AVC_ME_SAD_ADJUST_MODE_NONE_INTEL;
  intel_sub_group_avc_ime_payload_t payload = intel_sub_group_avc_ime_initialize(src_coord, partition_mask, sad_adjustment);
  short2 ref_offset = 0;
  uchar search_window_config = CLK_AVC_ME_SEARCH_WINDOW_EXTRA_TINY_INTEL;
  payload = intel_sub_group_avc_ime_set_single_reference(ref_offset, search_window_config, payload);

  //mv cost penalty setting
  ulong packed_cc_delta = 0;
  uint2 packed_cost_table = intel_sub_group_avc_mce_get_default_medium_penalty_cost_table();
  uchar cost_precision = CLK_AVC_ME_COST_PRECISION_QPEL_INTEL;
  payload = intel_sub_group_avc_ime_set_motion_vector_cost_function(
      packed_cc_delta, packed_cost_table, cost_precision, payload);
  
  //ime shape penalty
  ulong packed_shape_cost = (1 << 4 | 2);
  packed_shape_cost <<= 32;
  payload = intel_sub_group_avc_ime_set_inter_shape_penalty(packed_shape_cost ,payload);

  sampler_t vs = 0;
  intel_sub_group_avc_ime_result_t i_result =
      intel_sub_group_avc_ime_evaluate_with_single_reference(src_img, ref_img, vs, payload);

  //Get ime related result
  ulong mvs = intel_sub_group_avc_ime_get_motion_vectors(i_result);
  ushort distortions = intel_sub_group_avc_ime_get_inter_distortions(i_result);
  uchar major_shape = intel_sub_group_avc_ime_get_inter_major_shape(i_result);
  uchar minor_shapes = intel_sub_group_avc_ime_get_inter_minor_shapes(i_result);
  uchar directions = intel_sub_group_avc_ime_get_inter_directions(i_result);

  int lid_x = get_local_id(0);
  int mb =  gr_id0  +  gr_id1 * get_num_groups(0);
  int2 bi_mvs = as_int2(mvs);
  if(lid_x == 0){
    motion_vector_buffer[mb] = as_short2(bi_mvs.s0);
    residuals_buffer[mb] = distortions;
    mj_shape_buffer[mb] = major_shape;
    mn_shapes_buffer[mb] = minor_shapes;
    directions_buffer[mb] = directions;
  }
  //fme setting and evaluate
  uchar pixel_mode = CLK_AVC_ME_SUBPIXEL_MODE_QPEL_INTEL;
  intel_sub_group_avc_ref_payload_t r_payload = 
      intel_sub_group_avc_fme_initialize(
          src_coord, mvs, major_shape, minor_shapes,
          directions, pixel_mode, sad_adjustment);
  intel_sub_group_avc_ref_result_t r_result =
      intel_sub_group_avc_ref_evaluate_with_single_reference(src_img, ref_img, vs, r_payload);
  mvs = intel_sub_group_avc_ref_get_motion_vectors(r_result);
  distortions = intel_sub_group_avc_ref_get_inter_distortions(r_result);

  dwo_buffer[mb*16*4 + lid_x + 16*0] = i_result.s0;
  dwo_buffer[mb*16*4 + lid_x + 16*1] = i_result.s1;
  dwo_buffer[mb*16*4 + lid_x + 16*2] = i_result.s2;
  dwo_buffer[mb*16*4 + lid_x + 16*3] = i_result.s3;

}
