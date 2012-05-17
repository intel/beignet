__kernel void
compiler_function_argument0(__global int *dst, short value)
{
  int id = (int)get_global_id(0);
  dst[id] = value;
}

