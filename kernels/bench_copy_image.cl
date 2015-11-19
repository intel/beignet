const constant float filter_flag = 0.111111f;
__kernel void
bench_copy_image(__read_only image2d_t src, __write_only image2d_t dst)
{
  uint4 color = 0;
  int2 coord;
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);

  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE| CLK_ADDRESS_CLAMP| CLK_FILTER_NEAREST;

  coord.x = x;
  coord.y = y;
  color=read_imageui(src, sampler, coord);
  write_imageui(dst, coord, color);
}

__kernel void
bench_filter_image(__read_only image2d_t src, __write_only image2d_t dst)
{
  float4 color = 0;
  int2 coord_00, coord_01, coord_02, coord_10, coord_11, coord_12, coord_20, coord_21, coord_22;
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  int y_sz = (int)get_global_size(1);

  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE| CLK_ADDRESS_CLAMP| CLK_FILTER_NEAREST;

  int x0 = x - 1; int x1 = x + 1;
  int y0 = y - 1; int y1 = y + 1 ;
  int x_left = (x > 0)?x0:x; int x_right = (x > x_sz - 2)?x:x1;
  int y_top = (y > 0)?y0:y; int y_bottom = (y > y_sz - 2)?y:y1;

  coord_00.x = x_left;  coord_00.y = y_top;
  coord_01.x = x; coord_01.y = y_top;
  coord_02.x = x_right; coord_02.y = y_top;

  coord_10.x = x_left; coord_10.y = y;
  coord_11.x = x; coord_11.y = y;
  coord_12.x = x_right; coord_12.y = y;

  coord_20.x = x_left; coord_20.y = y_bottom;
  coord_21.x = x; coord_21.y = y_bottom;
  coord_22.x = x_right; coord_22.y = y_bottom;

  color = convert_float4(read_imageui(src, sampler, coord_00)) + convert_float4(read_imageui(src, sampler, coord_01)) + convert_float4(read_imageui(src, sampler, coord_02))
        + convert_float4(read_imageui(src, sampler, coord_10)) + convert_float4(read_imageui(src, sampler, coord_11)) + convert_float4(read_imageui(src, sampler, coord_12))
        + convert_float4(read_imageui(src, sampler, coord_20)) + convert_float4(read_imageui(src, sampler, coord_21)) + convert_float4(read_imageui(src, sampler, coord_22));

  write_imageui(dst, coord_11, convert_uint4(color * filter_flag));
}
