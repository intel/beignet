__kernel void
compiler_array2(__global int *src, __global int *dst)
{
  int final[16];
  int array[16];
  for (int j = 0; j < 16; ++j) array[j] = j;
  for (int j = 0; j < 16; ++j) final[j] = j+1;
  if (get_global_id(0) == 15)
    dst[get_global_id(0)] = final[get_global_id(0)];
  else
    dst[get_global_id(0)] = array[15 - get_global_id(0)];
}

