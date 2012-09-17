__kernel void
compiler_array3(__global int *src, __global int *dst)
{
  int tmp[32];
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j)
      tmp[j] = get_global_id(0);
    for (int j = 0; j < src[0]; ++j)
      tmp[j] = 1+src[j];
    tmp[16+i] = tmp[i];
  }
  dst[get_global_id(0)] = tmp[16+get_global_id(0)];
}

