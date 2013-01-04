/* test OpenCL 1.1 Vector Data Load/Store Functions (section 6.11.7) */
kernel void compiler_vector_load_store() {
  float p[16], f;
  float4 f4;
  f4 = vload4(0, p);
  vstore4(f4, 0, p);
  
  long x[16], l;
  long16 l16;
  l = vload16(0, x);
  vstore16(l16, 0, x);

  half h[16];
  half4 h4;
  f = vload_half(0, h);
  f4 = vload_half4(0, h);
  vstore_half(f, 0, h);
}
