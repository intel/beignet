__kernel void
test_get_image_info_array(__write_only image1d_array_t a1, __write_only image2d_array_t a2, __global int *result)
{
  int w, h, array_sz;

  w = get_image_width(a1);
  array_sz = (int)get_image_array_size(a1);
  int channel_data_type = get_image_channel_data_type(a1);
  int channel_order = get_image_channel_order(a1);
  result[0] = w;
  result[1] = array_sz;
  result[2] = channel_data_type;
  result[3] = channel_order;

  w = get_image_width(a2);
  h = get_image_height(a2);
  array_sz = (int)get_image_array_size(a2);
  channel_data_type = get_image_channel_data_type(a2);
  channel_order = get_image_channel_order(a2);
  result[4] = w;
  result[5] = h;
  result[6] = array_sz;
  result[7] = channel_data_type;
  result[8] = channel_order;
}
