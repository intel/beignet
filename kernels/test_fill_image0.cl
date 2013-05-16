__kernel void
test_fill_image0(__write_only image2d_t dst)
{
  int2 coord;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  int4 color4 = {coord.y & 0xFF, (coord.y & 0xFF00) >> 8, coord.x & 0xFF, (coord.x & 0xFF00) >> 8};
  write_imagei(dst, coord, color4);
}
