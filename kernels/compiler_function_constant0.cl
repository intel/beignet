__kernel void
compiler_function_constant0(__constant int *c0, __constant char *c1, __global int *dst, int value)
{
  int id = (int)get_global_id(0);
  dst[id] = value + c0[id%69] + c1[0];
}
