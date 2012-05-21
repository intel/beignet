struct hop { int x[16]; };

__kernel void
compiler_argument_structure(__global int *dst, struct hop h)
{
  int id = (int)get_global_id(0);
  dst[id] = h.x[get_local_id(0)];
}

