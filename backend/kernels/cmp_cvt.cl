#include <stdlib.h>

__kernel void cmp_cvt(__global int *dst, int x, int y)
{
  dst[0] = x + y < get_local_id(0) ;
}

