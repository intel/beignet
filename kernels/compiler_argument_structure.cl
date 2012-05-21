struct hop { int x, y; };

__kernel void
compiler_argument_structure(__global int *dst, struct hop h)
{
  int id = (int)get_global_id(0);
  dst[id] = h.x + h.y;
}

