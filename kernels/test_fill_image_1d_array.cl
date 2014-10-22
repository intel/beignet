__kernel void
test_fill_image_1d_array(__write_only image1d_array_t dst)
{
  int coordx;
  int coordy;
  coordx = (int)get_global_id(0);
  coordy = (int)get_global_id(1);
  uint4 color4 = {0, 1, 2 ,3};
  if (coordy < 7)
    write_imageui(dst, (int2)(coordx, coordy), color4);
}
