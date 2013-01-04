/* test OpenCL 1.1 Integet Built-in Functions (section 6.11.3) */
__kernel void compiler_integer_builtin() {
  int i = 0, i1 = -1, i2 = -2;
  unsigned u = 1, u1 = 2, u2 = 3;
  i = CHAR_MAX;
  i = abs(u);
  i = abs_diff(u1, u2);
  i = add_sat(i1, i2);
  i = hadd(i1, i2);
  i = rhadd(i1, i2);
  i = clz(i);
  i = clamp(i, i1, i2);
  i = mad_hi(i, i1, i2);
  i = mad_sat(i, i1, i2);
  i = max(i1, i2);
  i = min(i1, i2);
  i = mul_hi(i1, i2);
  i = rotate(i1, i2);
  i = sub_sat(i1, i2);
  long l = upsample(i, u);
  i = mad24(i, i1, i2);
  i = mul24(i1, i2);
}
