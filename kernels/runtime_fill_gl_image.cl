__kernel void
runtime_fill_gl_image(image2d_t img, int color)
{
  int2 coord;
  float4 color_v4;
  int lgid_x = get_group_id(0);
  int lgid_y = get_group_id(1);
  int num_groups_x = get_num_groups(0);
  int num_groups_y = get_num_groups(1);

  coord.x = get_global_id(0);
  coord.y = get_global_id(1);
  color_v4 = (float4)( lgid_x/(float)num_groups_x, lgid_y/(float)num_groups_y, 1.0, 1.0);
  write_imagef(img, coord, color_v4);
}
