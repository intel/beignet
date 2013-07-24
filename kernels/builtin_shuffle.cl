kernel void builtin_shuffle(global float *src1, global float *src2, global float *dst1, global float *dst2) {
  int i = get_global_id(0);
  float2 src = (float2)(src1[i], src2[i]);
  uint2 mask = (uint2)(1, 0);
  float2 dst = shuffle(src, mask);
  dst1[i] = dst.s0;
  dst2[i] = dst.s1;
}
