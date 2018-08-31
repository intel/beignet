kernel void __cl_copy_buffer_to_image_3d_align16(__write_only image3d_t image, global uint4 *buffer,
                                                 unsigned int region0, unsigned int region1, unsigned int region2,
                                                 unsigned int dst_origin0, unsigned int dst_origin1,
                                                 unsigned int dst_origin2, unsigned int src_offset) {
  int i = get_global_id(0);
  int j = get_global_id(1);
  int k = get_global_id(2);
  uint4 color = (uint4)(0);
  int4 dst_coord;
  if ((i >= region0) || (j >= region1) || (k >= region2))
    return;
  dst_coord.x = dst_origin0 + i;
  dst_coord.y = dst_origin1 + j;
  dst_coord.z = dst_origin2 + k;
  src_offset += (k * region1 + j) * region0 + i;
  color = buffer[src_offset];
  write_imageui(image, dst_coord, color);
}
