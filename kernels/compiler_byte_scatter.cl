__kernel void
compiler_byte_scatter(__global char *dst)
{
  int id = (int) get_global_id(0);
  dst[id] = (char) id;
}

