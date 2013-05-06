__kernel void
test_copy_image_3d(__read_only image3d_t src, __write_only image3d_t dst, sampler_t sampler)
{
  int4 coord;
  int4 color;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  coord.z = 0;
  color = read_imagei(src, sampler, coord);
  write_imagei(dst, coord, color);
}
