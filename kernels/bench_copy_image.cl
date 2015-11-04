__kernel void
bench_copy_image(__read_only image2d_t src, __write_only image2d_t dst)
{
  uint4 color = 0;
  int2 coord;
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);

  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE| CLK_ADDRESS_CLAMP| CLK_FILTER_NEAREST;

  coord.x = x;
  coord.y = y;
  color=read_imageui(src, sampler, coord);
  write_imageui(dst, coord, color);
}
