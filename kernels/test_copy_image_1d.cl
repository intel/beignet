__kernel void
test_copy_image_1d(__read_only image1d_t src, __write_only image1d_t dst, sampler_t sampler)
{
  int coord;
  int4 color;
  coord = (int)get_global_id(0);
  color = read_imagei(src, sampler, coord);
  write_imagei(dst, coord, color);
}
