__kernel void
compiler_function_argument1(__global int *dst, char value, short value0, int value1)
{
  int id = (int)get_global_id(0);
  dst[id] = value + value0 + value1;
}

