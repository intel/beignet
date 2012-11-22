/* test OpenCL 1.1 Conversions & Type Casting Examples (section 6.2) */
__kernel void compiler_type_casting() {
  float f = 1.23456789f;
  float g;

  g = (float)f;
  g = convert_float(f);
  g = as_float(f);
  
  g = convert_float_rte(f);
  g = convert_float_rtz(f);
  g = convert_float_rtp(f);
  g = convert_float_rtn(f);

  g = convert_float_sat_rte(f);
  g = convert_float_sat_rtz(f);
  g = convert_float_sat_rtp(f);
  g = convert_float_sat_rtn(f);
}
