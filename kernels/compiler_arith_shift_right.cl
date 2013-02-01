kernel void compiler_arith_shift_right(global int *src, global int *dst) {
    int i = get_global_id(0);
    dst[i] = src[i] >> 24;
}
