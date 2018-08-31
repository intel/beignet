__kernel __attribute__((intel_reqd_sub_group_size(16)))
void compiler_skip_check(__read_only image2d_t   src_img,
                         __read_only image2d_t   ref_img,
                         __global short2  *motion_vector_buffer,
                         __global ushort  *residual_buffer,
                         __global uint*          dwo_buffer,
                         __global uint*          pld_buffer){
  sampler_t vs = 0;

  int gr_id0 = get_group_id(0);
  int gr_id1 = get_group_id(1);

  ushort2 src_coord;

  src_coord.x = gr_id0 * 16;
  src_coord.y = gr_id1 * 16;

  intel_sub_group_avc_sic_payload_t payload =
      intel_sub_group_avc_sic_initialize(src_coord);

  //Ignore in unidirectional, so just set to 0
  uchar bidir_weight = 0;

  uint skip_block_partition_type = CLK_AVC_ME_SKIP_BLOCK_PARTITION_16x16_INTEL;
  uint skip_motion_vector_mask = CLK_AVC_ME_SKIP_BLOCK_16x16_FORWARD_ENABLE_INTEL;
  uchar skip_sad_adjustment = CLK_AVC_ME_SAD_ADJUST_MODE_NONE_INTEL;

  uint2 bi_mv;
  int mb_idx =  gr_id0  +  gr_id1 * get_num_groups(0);
  short2 input_mv = motion_vector_buffer[mb_idx];
  bi_mv.s0 = as_uint(input_mv);

  ulong mv = as_ulong(bi_mv);

  payload = intel_sub_group_avc_sic_configure_skc(
      skip_block_partition_type, skip_motion_vector_mask, mv, bidir_weight,
      skip_sad_adjustment, payload);

  intel_sub_group_avc_sic_result_t result =
      intel_sub_group_avc_sic_evaluate_with_single_reference(
          src_img, ref_img, vs, payload);

  ushort distortion = intel_sub_group_avc_sic_get_inter_distortions(result);

  int lid_x = get_local_id(0);
  if(lid_x == 0)
    residual_buffer[mb_idx] = distortion;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*0] = result.s0;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*1] = result.s1;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*2] = result.s2;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*3] = result.s3;

}
