__kernel void
compiler_array4(__global int4 *src4, __global int4 *dst4, int offset)
{
  int i;
  int final[16];
  __global int *dst = (__global int *)(dst4 + offset + get_global_id(0));
  __global int *src = (__global int *)(src4 + offset + get_global_id(0));
  dst[-4] = src[-4];
}
