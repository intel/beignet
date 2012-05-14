__kernel void
compiler_lower_return1(__global int *src, __global int *dst) {
  const int id = get_global_id(0);
  dst[id] = id;
  if (id < 11 && (src[id] > 0 || src[id+16] < 2)) return;
  dst[id] = src[id];
}

