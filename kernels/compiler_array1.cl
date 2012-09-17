__kernel void
compiler_array1(__global int *src, __global int *dst)
{
  int final[16];
  for (int i = 0; i < 16; ++i) {
    int array[16];
    for (int j = 0; j < src[0]; ++j)
      array[j] = 1+src[0];
    for (int j = src[0]; j < 16; ++j)
      array[j] = get_global_id(0);
    final[i] = array[i];
  }
  dst[get_global_id(0)] = final[get_global_id(0)];
}

