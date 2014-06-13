#include "runtime_compile_link.h"
#include "include/runtime_compile_link_inc.h"

int comp_long(long x, long y)
{
  return x < y ;
}

kernel void runtime_compile_link_a(global long *src1, global long *src2, global long *dst) {
  int i = get_global_id(0);
  int j = comp_long(src1[i], src2[i]);
  dst[i] = j ? 3 : 4;
}
