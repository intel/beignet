#include <stdlib.h>
__kernel void short_write(__global short *dst, short x, short y)
{
  dst[0] = x + y;
}

