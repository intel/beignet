__kernel void
bench_copy_buffer_uchar(__global uchar4* src, __global uchar4* dst)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  dst[y * x_sz + x] =  src[y * x_sz + x];
}

__kernel void
bench_copy_buffer_ushort(__global ushort4* src, __global ushort4* dst)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  dst[y * x_sz + x] =  src[y * x_sz + x];
}

__kernel void
bench_copy_buffer_uint(__global uint4* src, __global uint4* dst)
{
  int x = (int)get_global_id(0);
  int y = (int)get_global_id(1);
  int x_sz = (int)get_global_size(0);
  dst[y * x_sz + x] =  src[y * x_sz + x];
}

