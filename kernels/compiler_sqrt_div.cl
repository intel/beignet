kernel void compiler_sqrt_div(global float *src, global float *dst) {
  int i = get_global_id(0);
  float tmp = sqrt(src[i]);
  dst[i*4] = 1.0f/tmp;
  dst[i*4+1] = (float)i/tmp;
  dst[i*4+2] = 2.0f/tmp;
  dst[i*4+3] = 1.0f/tmp  + tmp;
};
