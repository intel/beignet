__kernel void
compiler_lower_return2(__global int *src, __global int *dst) {
  const int id = get_global_id(0);
  dst[id] = id;
  while (dst[id] > src[id]) {
    if (dst[id] > 10) return;
    dst[id]--;
  }
  dst[id] += 2;
}

