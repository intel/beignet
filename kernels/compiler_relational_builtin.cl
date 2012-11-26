/* test OpenCL 1.1 Relational Built-in Functions (section 6.11.6) */
kernel void compiler_relational_builtin() {
  float x = 1, y = 2, z = 3;
  int i;
  i = isequal(x, y);
  i = isnotequal(x, y);
  i = isgreater(x, y);
  i = isgreaterequal(x, y);
  i = isless(x, y);
  i = islessequal(x, y);
  i = islessgreater(x, y);
  i = isfinite(x);
  i = isinf(x);
  i = isnan(x);
  i = isnormal(x);
  i = isordered(x, y);
  i = isunordered(x, y);
  i = signbit(x);
  long l = 12;
  i = any(l);
  i = all(l);
  bitselect(x, y, z);
  select(x, y, z);
}
