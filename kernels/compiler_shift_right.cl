kernel void compiler_shift_right(global uint *src, global int *dst) {
    int i = get_global_id(0);
    dst[i] = src[i] >> 24;
}
