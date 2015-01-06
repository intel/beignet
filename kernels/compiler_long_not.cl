__kernel void compiler_long_not_vec8(__global ulong8 *src, __global long8 *dst)
{
  int tid = get_global_id(0);
  dst[tid] = !src[tid];
}

