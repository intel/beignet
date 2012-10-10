__kernel void compiler_math(__global float *dst, __global float *src) {
  const float x = src[get_global_id(0)];
  switch (get_global_id(0)) {
    case 0: dst[get_global_id(0)] = native_cos(x); break;
    case 1: dst[get_global_id(0)] = native_sin(x); break;
    case 2: dst[get_global_id(0)] = native_log2(x); break;
    case 3: dst[get_global_id(0)] = native_sqrt(x); break;
    case 4: dst[get_global_id(0)] = native_rsqrt(x); break;
    case 5: dst[get_global_id(0)] = native_recip(x); break;
    case 6: dst[get_global_id(0)] = native_tan(x); break;
    default: dst[get_global_id(0)] = 1.f; break;
  };
}

