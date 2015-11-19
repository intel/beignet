const constant float filter_flag = 0.111111f;
__kernel void
bench_copy_buffer_uchar(__global uchar4* src, __global uchar4* dst)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  dst[y * x_sz + x] = src[y * x_sz + x];
}

__kernel void
bench_copy_buffer_ushort(__global ushort4* src, __global ushort4* dst)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  dst[y * x_sz + x] = src[y * x_sz + x];
}

__kernel void
bench_copy_buffer_uint(__global uint4* src, __global uint4* dst)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  dst[y * x_sz + x] = src[y * x_sz + x];
}

__kernel void
bench_filter_buffer_uchar(__global uchar4* src, __global uchar4* dst)
{
  float4 result;
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  int y_sz = (int)get_global_size(1);

  int x0 = x - 1; int x1 = x + 1;
  int y0 = y - 1; int y1 = y + 1 ;
  int x_left = (x0 > 0)?x0:x; int x_right = (x1 > x_sz - 1)?x:x1;
  int y_top = (y0 > 0)?y0:y; int y_bottom = (y1 > y_sz - 1)?y:y1;

  result = convert_float4(src[y_top * x_sz + x_left]) + convert_float4(src[y_top * x_sz + x]) + convert_float4(src[y_top * x_sz + x_right])
         + convert_float4(src[y * x_sz + x_left]) + convert_float4(src[y * x_sz + x]) + convert_float4(src[y * x_sz + x_right])
         + convert_float4(src[y_bottom * x_sz + x_left]) + convert_float4(src[y_bottom * x_sz + x]) + convert_float4(src[y_bottom * x_sz +x_right]);

  dst[y * x_sz + x] = convert_uchar4(result * filter_flag);
}

__kernel void
bench_filter_buffer_ushort(__global ushort4* src, __global ushort4* dst)
{
  float4 result;
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  int y_sz = (int)get_global_size(1);

  int x0 = x - 1; int x1 = x + 1;
  int y0 = y - 1; int y1 = y + 1 ;
  int x_left = (x0 > 0)?x0:x; int x_right = (x1 > x_sz - 1)?x:x1;
  int y_top = (y0 > 0)?y0:y; int y_bottom = (y1 > y_sz - 1)?y:y1;

  result = convert_float4(src[y_top * x_sz + x_left]) + convert_float4(src[y_top * x_sz + x]) + convert_float4(src[y_top * x_sz + x_right])
         + convert_float4(src[y * x_sz + x_left]) + convert_float4(src[y * x_sz + x]) + convert_float4(src[y * x_sz + x_right])
         + convert_float4(src[y_bottom * x_sz + x_left]) + convert_float4(src[y_bottom * x_sz + x]) + convert_float4(src[y_bottom * x_sz +x_right]);

  dst[y * x_sz + x] = convert_ushort4(result * filter_flag);
}

__kernel void
bench_filter_buffer_uint(__global uint4* src, __global uint4* dst)
{
  float4 result;
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  int y_sz = (int)get_global_size(1);

  int x0 = x - 1; int x1 = x + 1;
  int y0 = y - 1; int y1 = y + 1 ;
  int x_left = (x0 > 0)?x0:x; int x_right = (x1 > x_sz - 1)?x:x1;
  int y_top = (y0 > 0)?y0:y; int y_bottom = (y1 > y_sz - 1)?y:y1;

  result = convert_float4(src[y_top * x_sz + x_left]) + convert_float4(src[y_top * x_sz + x]) + convert_float4(src[y_top * x_sz + x_right])
         + convert_float4(src[y * x_sz + x_left]) + convert_float4(src[y * x_sz + x]) + convert_float4(src[y * x_sz + x_right])
         + convert_float4(src[y_bottom * x_sz + x_left]) + convert_float4(src[y_bottom * x_sz + x]) + convert_float4(src[y_bottom * x_sz +x_right]);

  dst[y * x_sz + x] = convert_uint4(result * filter_flag);
}
