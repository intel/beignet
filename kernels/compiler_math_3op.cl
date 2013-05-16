kernel void compiler_math_3op(global float *dst, global float *src1, global float *src2, global float *src3) {
  int i = get_global_id(0);
  const float x = src1[i], y = src2[i], z = src3[i];
  switch (i) {
    case 0: dst[i] = mad(x, y, z); break;
    case 1: dst[i] = fma(x, y, z); break;
    default: dst[i] = 1.f; break;
  };
}
