__kernel void
compiler_group_size(__global unsigned int *dst)
{
  uint idx = (uint)get_global_id(0);
  uint idy = (uint)get_global_id(1);
  uint idz = (uint)get_global_id(2);
  uint size_x = (uint)get_global_size(0);
  uint size_y = (uint)get_global_size(1);

  dst[idz*size_x*size_y + idy*size_x + idx] = idz*size_x*size_y + idy*size_x +idx;
}

struct xyz{
  unsigned short b;
  unsigned short e;
  unsigned int o;
};

__kernel void
compiler_group_size4(__global struct xyz *src, __global unsigned int *dst, unsigned int num, unsigned int c)
{
  uint idx = (uint)get_global_id(0);
  if(idx>=num)
    return;
  struct xyz td = src[idx];
  for(unsigned x = td.b;x<=td.e;x++)
    dst[td.o+x] = c;
}

