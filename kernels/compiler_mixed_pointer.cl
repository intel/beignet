
kernel void compiler_mixed_pointer(__global uint* src1, __global uint *src2, __global uint *dst) {
  int x = get_global_id(0);
  global uint * tmp = NULL;

  switch(x) {
    case 0:
    case 1:
    case 4:
      tmp = src1;
      break;
    default:
      tmp = src2;
      break;
  }
  dst[x] = tmp[x];
}

kernel void compiler_mixed_pointer1(__global uint* src, __global uint *dst1, __global uint *dst2) {
  int x = get_global_id(0);
  global uint * tmp = x < 5 ? dst1 : dst2;
  tmp[x] = src[x];
}
