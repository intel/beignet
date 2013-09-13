__kernel void
test_fill_image_3d_2(__write_only image3d_t dst)
{
  int4 coord;
  int4 color4 = {0x12, 0x34, 0x56, 0x78};
  coord.x = (int)get_global_id(0);
  coord.y = (int)get_global_id(1);
  coord.z = (int)get_global_id(2);
  write_imagei(dst, coord, color4);
}
