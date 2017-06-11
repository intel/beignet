kernel void __cl_copy_buffer_rect_align4 ( global int* src, global int* dst,
                                          unsigned int region0, unsigned int region1, unsigned int region2,
                                          unsigned int src_offset, unsigned int dst_offset,
                                          unsigned int src_row_pitch, unsigned int src_slice_pitch,
                                          unsigned int dst_row_pitch, unsigned int dst_slice_pitch)
{
  int i = get_global_id(0);
  int j = get_global_id(1);
  int k = get_global_id(2);
  if((i >= region0) || (j>= region1) || (k>=region2))
    return;
  src_offset += k * src_slice_pitch + j * src_row_pitch + i;
  dst_offset += k * dst_slice_pitch + j * dst_row_pitch + i;
  dst[dst_offset] = src[src_offset];
}
