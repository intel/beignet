kernel void compiler_long_shr(global ulong *src, global ulong *dst) {
  int i = get_global_id(0);
  if(i > 7)
    dst[i] = src[i] >> i;
  else
    dst[i] = src[i] + 1;
}
