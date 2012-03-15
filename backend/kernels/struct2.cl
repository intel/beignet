#include "stdlib.h"
struct my_struct {
  int a;
  int b[2];
};

const __constant int g[4] = {0,1,2,3};

__kernel void struct_cl (struct my_struct s, int x, __global struct my_struct *mem, int y)
{
  struct my_struct hop;
  if (y == 0) {
    hop.a = 1;
    hop.b[0] = 2;
    hop.b[1] = 2;
  } else {
    hop = s;
  }
  mem[0] = hop;
}


