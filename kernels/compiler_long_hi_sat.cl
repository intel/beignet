kernel void compiler_long_mul_hi(global long *src, global long *dst, long num0, long num1) {
    int i = get_local_id(0);
    long c;

    if (i % 2 == 0) {
      c = mul_hi(src[i],  num0);
    } else {
      c = mul_hi(src[i],  num1);
    }
    dst[i] = c;
}

kernel void compiler_long_mul_sat(global long *src, global long *dst, long num0, long num1) {
    int i = get_local_id(0);
    long c;

    c = mad_sat(src[i],  num0, num1);
    dst[i] = c;
}
