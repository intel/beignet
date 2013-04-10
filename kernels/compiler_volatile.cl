__kernel void compiler_volatile(__global int *dst, __local volatile int *hop) {
  hop[get_global_id(0)] = get_local_id(1);
  dst[get_global_id(0)] = hop[get_local_id(0)];
}
