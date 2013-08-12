__kernel void
compiler_event(__global int *dst, int value)
{
  int id = (int)get_global_id(0);
  dst[id] += value;
}
