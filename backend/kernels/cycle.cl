__kernel void cycle(global int *dst)
{
  int x, y;

hop0:
  x = y;

hop1:
  y = x;
  goto hop0;

  dst[0] = x;
}

