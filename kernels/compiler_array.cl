__kernel void
compiler_array(__global int *src, __global int *dst)
{
  int array[16];
  int i;
  for (i = 0; i < 16; ++i) {
    if (src[0] > 10)
      array[i] = get_local_id(0);
    else
      array[15 - i] = 3 + get_local_id(1);
  }
  dst[get_global_id(0)] = array[get_local_id(0)];
}

