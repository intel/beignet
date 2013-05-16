__kernel void compiler_math(__global float *dst, __global float *src) {
  int i = get_global_id(0);
  const float x = src[i];
  switch (i) {
    case 0: dst[i] = cos(x); break;
    case 1: dst[i] = sin(x); break;
    case 2: dst[i] = log2(x); break;
    case 3: dst[i] = sqrt(x); break;
    case 4: dst[i] = rsqrt(x); break;
    case 5: dst[i] = native_recip(x); break;
    case 6: dst[i] = tan(x); break;
    case 7: dst[i] = cbrt(x); break;
    case 8: dst[i] = ceil(x); break;
    case 9: dst[i] = cospi(x); break;
    case 10: dst[i] = exp2(x); break;
    case 11: dst[i] = exp10(x); break;
    case 12: dst[i] = expm1(x); break;
    case 13: dst[i] = log1p(x); break;
    case 14: dst[i] = logb(x); break;
    case 15: dst[i] = sinpi(x); break;
    case 16: dst[i] = tanpi(x); break;
    case 17: dst[i] = rint(x); break;
    case 18: dst[i] = sinh(x); break;
    case 19: dst[i] = cosh(x); break;
    case 20: dst[i] = tanh(x); break;
    case 21: dst[i] = asinh(x); break;
    case 22: dst[i] = acosh(x); break;
    case 23: dst[i] = atanh(x); break;
    case 24: dst[i] = asin(x); break;
    case 25: dst[i] = acos(x); break;
    case 26: dst[i] = atan(x); break;
    case 27: dst[i] = asinpi(x); break;
    case 28: dst[i] = acospi(x); break;
    case 29: dst[i] = atanpi(x); break;
    case 30: dst[i] = erf(x); break;
    case 31: dst[i] = nan((uint)x); break;
    default: dst[i] = 1.f; break;
  };
}

