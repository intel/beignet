__kernel void
compiler_array(__global int *src, __global int *dst, int x)
{
  if (x > 10) {
    int array[x*256];
    array[get_local_id(0)] = get_global_id(0);
    dst[get_global_id(0)] = array[get_local_id(1)];
  } else
    dst[get_global_id(0)] = src[get_local_id(1)];
}


