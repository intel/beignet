kernel void compiler_math_2op(global float *dst, global float *src1, global float *src2) {
  int i = get_global_id(0);
  const float x = src1[i], y = src2[i];
  float z;
  switch (i) {
    case 0: dst[i] = native_divide(x, y); break;
    case 1: dst[i] = fdim(x, y); break;
    case 2: dst[i] = fract(x, &z); break;
    case 3: dst[i] = hypot(x, y); break;
    case 4: dst[i] = ldexp(x, y); break;
    case 5: dst[i] = pown(x, (int)y); break;
    case 6: dst[i] = remainder(x, y); break;
    case 7: dst[i] = rootn(x, (int)(y+1)); break;
    case 8: dst[i] = copysign(x, y); break;
    case 9: dst[i] = maxmag(x, y); break;
    case 10: dst[i] = minmag(x, y); break;
    default: dst[i] = 1.f; break;
  };
}
