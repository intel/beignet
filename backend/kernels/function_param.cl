struct struct0
{
  int hop[5];
  int x, y, z;
};

#define __global __attribute__((address_space(1)))
__kernel void param(__global struct struct0 *dst, struct struct0 s, __local int *h, int x, int y)
{
  s.hop[4] += x + h[4];
  dst[0] = s;
  dst[0].y += y;
}

