
__kernel __attribute__((intel_reqd_sub_group_size(16)))
void  compiler_intra_prediction(
    __read_only image2d_t   srcImg,
    __global uchar          *luma_mode,
    __global ushort         *luma_distortion,
    __global uchar          *luma_shape,
    __global uint*          dwo_buffer,
    __global uint*          pld_buffer){

  int gr_id0 = get_group_id(0);
  int gr_id1 = get_group_id(1);

  ushort2 src_coord;
  /*src_coord.x = gr_id0  * 16;
  src_coord.y = gr_id1 * 16;*/
  src_coord.x = 2 * 16;
  src_coord.y = 1 * 16;

  intel_sub_group_avc_sic_payload_t payload = intel_sub_group_avc_sic_initialize(src_coord);

  uchar sad_adjustment = CLK_AVC_ME_SAD_ADJUST_MODE_NONE_INTEL;
  uchar intra_partition_mask = CLK_AVC_ME_INTRA_LUMA_PARTITION_MASK_16x16_INTEL;
//XXX: Different from official value?
#undef CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_RIGHT_MASK_ENABLE_INTEL
#undef CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_LEFT_MASK_ENABLE_INTEL
#define CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_RIGHT_MASK_ENABLE_INTEL 0x4
#define CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_LEFT_MASK_ENABLE_INTEL  0x8
  uint nb_avail = CLK_AVC_ME_INTRA_NEIGHBOR_LEFT_MASK_ENABLE_INTEL |
               CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_MASK_ENABLE_INTEL |
               CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_LEFT_MASK_ENABLE_INTEL |
               CLK_AVC_ME_INTRA_NEIGHBOR_UPPER_RIGHT_MASK_ENABLE_INTEL;

  uint sgl_id = get_sub_group_local_id();
  int2 nb_coord;
  float4 color;

  nb_coord.x = src_coord.x - 1;
  nb_coord.y = src_coord.y + sgl_id;
  color = read_imagef(srcImg, nb_coord);
  uchar left_edge = color.s0 * 255;

  nb_coord.x = src_coord.x - 1;
  nb_coord.y = src_coord.y - 1;
  color = read_imagef(srcImg, nb_coord);
  uchar upper_left_corner = color.s0 * 255;

  nb_coord.x = src_coord.x + sgl_id;
  nb_coord.y = src_coord.y - 1;
  color = read_imagef(srcImg, nb_coord);
  uchar upper_edge = color.s0 * 255;

  uchar upper_right_edge = 0;
  if(sgl_id < 8){
    nb_coord.x = src_coord.x + 16 + sgl_id;
    nb_coord.y = src_coord.y - 1;
    color = read_imagef(srcImg, nb_coord);
    upper_right_edge = color.s0 * 255;
  }
  payload = intel_sub_group_avc_sic_configure_ipe(
      intra_partition_mask, nb_avail, left_edge, upper_left_corner, upper_edge,
      upper_right_edge, sad_adjustment, payload);

  uchar shape_cost_16_16 = (1 << 4) | 5;
  uchar shape_cost_8_8 = (1 << 4) | 4;
  uchar shape_cost_4_4 = (1 << 4) | 3;
  uint intra_shape_cost = (shape_cost_4_4 << 24) | (shape_cost_8_8 << 16) | (shape_cost_16_16 << 8) | (0x0);
  payload = intel_sub_group_avc_sic_set_intra_luma_shape_penalty(intra_shape_cost, payload);

  sampler_t vs = 0;
  intel_sub_group_avc_sic_result_t result =
      intel_sub_group_avc_sic_evaluate_ipe(srcImg, vs, payload);

  uchar shape = intel_sub_group_avc_sic_get_ipe_luma_shape(result);
  ushort dist = intel_sub_group_avc_sic_get_best_ipe_luma_distortion(result);
  ulong modes = intel_sub_group_avc_sic_get_packed_ipe_luma_modes(result);

  int lid_x = get_local_id(0);
  int mb_idx = gr_id0 + gr_id1 * get_num_groups(0);
  if (lid_x == 0) {
    luma_shape[mb_idx] = shape;
    luma_distortion[mb_idx] = dist;
    uchar mode = modes & 0xF;
    luma_mode[mb_idx] = mode;
  }

  dwo_buffer[mb_idx*16*4 + lid_x + 16*0] = result.s0;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*1] = result.s1;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*2] = result.s2;
  dwo_buffer[mb_idx*16*4 + lid_x + 16*3] = result.s3;
}
