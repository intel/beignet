__kernel void
test_get_image_size(__write_only image2d_t src, __global int *info)
{
  int id = (int)get_global_id(0);
  int w, h;
  w = get_image_width(src);
  h = get_image_height(src);
  info[id] = (w << 16 | h);
}
