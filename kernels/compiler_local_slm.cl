#if 0
__kernel void compiler_local_slm(__global int *dst, __local int *hop) {
#else
__kernel void compiler_local_slm(__global int *dst) {
  __local int hop[10];
#endif
  hop[get_global_id(0)] = get_local_id(1);
  dst[get_global_id(0)] = hop[get_local_id(0)];
}

