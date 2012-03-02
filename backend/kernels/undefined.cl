__kernel void undefined(__global int *dst)
{
  int x;
  if (x == 0)
    dst[0] = 0;
  else
    dst[0] = 1;
}

