constant int m[3] = {0x15b,0x25b,0x35b};
constant int t[5] = {0x45b,0x55b,0x65b,0x75b,0x85b};

__kernel void
compiler_global_constant_2(__global int *dst, int e, int r)
{
  int id = (int)get_global_id(0);
  dst[id] = m[id%3] + t[id%5] + e + r;
}
