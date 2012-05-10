__kernel void
compiler_short_scatter(__global short *dst)
{
  int id = (int) get_global_id(0);
  dst[id] = (short) id;
}

