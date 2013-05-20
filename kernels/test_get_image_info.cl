__kernel void
test_get_image_info(__write_only image3d_t src, __global int *size, __global int *fmt)
{
  int id = (int)get_global_id(0);
  int w, h, depth;
  w = get_image_width(src);
  h = get_image_height(src);
  depth = get_image_depth(src);
  int channel_data_type = get_image_channel_data_type(src);
  int channel_order = get_image_channel_order(src);
  size[id] = (w << 20 | h << 8  | depth);
  fmt[id] = (channel_data_type << 16 | channel_order);
}
