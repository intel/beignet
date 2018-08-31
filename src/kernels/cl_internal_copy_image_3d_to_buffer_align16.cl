kernel void __cl_copy_image_3d_to_buffer_align16(__read_only image3d_t image, global uint4 *buffer,
                                                 unsigned int region0, unsigned int region1, unsigned int region2,
                                                 unsigned int src_origin0, unsigned int src_origin1,
                                                 unsigned int src_origin2, unsigned int dst_offset) {
  int i = get_global_id(0);
  int j = get_global_id(1);
  int k = get_global_id(2);
  uint4 color;
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  int4 src_coord;
  if ((i >= region0) || (j >= region1) || (k >= region2))
    return;
  src_coord.x = src_origin0 + i;
  src_coord.y = src_origin1 + j;
  src_coord.z = src_origin2 + k;
  color = read_imageui(image, sampler, src_coord);
  dst_offset += (k * region1 + j) * region0 + i;
  *(buffer + dst_offset) = color;
}
