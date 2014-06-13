__kernel void
test_fill_image_1d(__write_only image1d_t dst)
{
  int coord;
  coord = (int)get_global_id(0);
  uint4 color4 = {0, 1, 2 ,3};
  write_imageui(dst, coord, color4);
}
