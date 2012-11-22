/* test OpenCL 1.1 Address Space Qualifiers (section 6.5) */
__constant float cf1[] = {1, 2, 3};
constant float cf2[] = {4, 5, 6};
__kernel void compiler_address_space(__global float *gf1, global float *gf2) {
  __local float lf1[4];
  local float lf2[4];
  __private float pf1[4];
  private float pf2[4];
}
