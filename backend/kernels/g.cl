#include "stdlib.h"
struct big{
  unsigned int a, b;
};

__kernel void add(__global struct big *b, unsigned int x, unsigned int y)
{
  __private int d[3] = {0,1,2};
  b->a = x + y + d[y];
  b->b = x - y + 10;
}


