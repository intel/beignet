__kernel void
test_fill_image(__write_only image2d_t dst, uint color)
{
  int2 coord;
  int4 color4;
  color4.s0  = (color >> 24) & 0xFF;
  color4.s1  = (color >> 16) & 0xFF;
  color4.s2  = (color >> 8) & 0xFF;
  color4.s3  = color & 0xFF;
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  write_imagei(dst, coord, color4);
}
