__kernel void
test_copy_image(__read_only image2d_t src, __write_only image2d_t dst, sampler_t sampler)
{
  int2 coord;
  int4 color;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  color = read_imagei(src, sampler, coord);
  write_imagei(dst, coord, color);
}
