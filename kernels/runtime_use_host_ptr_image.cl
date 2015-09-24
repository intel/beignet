__kernel void
runtime_use_host_ptr_image(__read_only image2d_t src, __write_only image2d_t dst)
{
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
  int2 coord;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  float4 data = read_imagef(src, sampler, coord);
  write_imagef(dst, coord, data);
}
