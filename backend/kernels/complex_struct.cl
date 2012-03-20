#include "stdlib.h"
struct hop { float x, y; };
struct my_struct {
  int a;
  struct hop b[5];
};

__kernel void struct_cl (__global struct my_struct *dst,
                         __global struct my_struct *src)
{
  dst[0].b[2].y = src[1].b[3].x;
}

