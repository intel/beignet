struct hop { int a, x[16]; };

__kernel void
compiler_argument_structure_indirect(__global int *dst, struct hop h)
{
  int id = (int)get_global_id(0);
  dst[id] = h.x[get_local_id(0)];
}

