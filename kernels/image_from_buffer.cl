__kernel void image_from_buffer(__read_only image2d_t src, __write_only image2d_t dst)
{
  int2 coord;
  int4 color;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);

  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

  color = read_imagei(src, sampler, coord);
  write_imagei(dst, coord, color);
}
