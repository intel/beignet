__kernel void compiler_bitcast_char8_to_long(__global char8 *src, __global ulong *dst)
{
  int tid = get_global_id(0);
  char8 v = src[tid];
  ulong dl = as_ulong(v);
  dst[tid] = dl;
}

__kernel void compiler_bitcast_long_to_char8(__global ulong *src,  __global uchar8 *dst)
{
  int tid = get_global_id(0);
  ulong v = src[tid];
  uchar8 dl = as_uchar8(v);
  dst[tid] = dl;
}

__kernel void compiler_bitcast_int2_to_long(__global int2 *src, __global ulong *dst)
{
  int tid = get_global_id(0);
  int2 v = src[tid];
  ulong dl = as_ulong(v);
  dst[tid] = dl;
}

__kernel void compiler_bitcast_long_to_int2(__global ulong *src,  __global uint2 *dst)
{
  int tid = get_global_id(0);
  ulong v = src[tid];
  uint2 dl = as_uint2(v);
  dst[tid] = dl;
}

__kernel void compiler_bitcast_short4_to_long(__global short4 *src, __global ulong *dst)
{
  int tid = get_global_id(0);
  short4 v = src[tid];
  ulong dl = as_ulong(v);
  dst[tid] = dl;
}

__kernel void compiler_bitcast_long_to_short4(__global ulong *src,  __global ushort4 *dst)
{
  int tid = get_global_id(0);
  ulong v = src[tid];
  ushort4 dl = as_ushort4(v);
  dst[tid] = dl;
}
