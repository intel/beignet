__kernel void
test_fill_image_2d_array(__write_only image2d_array_t dst)
{
  int coordx;
  int coordy;
  int coordz;
  coordx = (int)get_global_id(0);
  coordy = (int)get_global_id(1);
  coordz = (int)get_global_id(2);
  uint4 color4 = {0, 1, 2 ,3};
  if (coordz < 7)
    write_imageui(dst, (int3)(coordx, coordy, coordz), color4);
}
