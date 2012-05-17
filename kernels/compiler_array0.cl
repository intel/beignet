__kernel void
compiler_array0(__global int *src, __global int *dst)
{
  int i;
  int final[16];
  for (i = 0; i < 16; ++i) {
    int array[16], j;
    for (j = 0; j < 16; ++j)
      array[j] = get_global_id(0);
    for (j = 0; j < src[0]; ++j)
      array[j] = 1+src[j];
    final[i] = array[i];
  }
  dst[get_global_id(0)] = final[get_global_id(0)];
}

