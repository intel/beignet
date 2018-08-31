kernel void compiler_fdiv2rcp(global float *src, global float *dst) {
  int i = get_global_id(0);
  float tmp = src[i];
  dst[i*4] = 1.0f/tmp;
  dst[i*4+1] = (float)i/tmp;
  dst[i*4+2] = 2.0f/tmp;
  dst[i*4+3] = 3.0f/tmp;
};
