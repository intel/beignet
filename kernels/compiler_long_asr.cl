kernel void compiler_long_asr(global long *src, global long *dst) {
  int i = get_global_id(0);
  if(i > 7)
    dst[i] = src[i] >> i;
  else
    dst[i] = src[i] + 1;
}
