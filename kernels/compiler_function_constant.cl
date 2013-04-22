__kernel void
compiler_function_constant(__constant short *c, __global int *dst, int value)
{
  int id = (int)get_global_id(0);
  dst[id] = value + c[id%69];
}
