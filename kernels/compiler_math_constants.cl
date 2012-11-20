/* test case for OpenCL 1.1 Math Constants (section 6.11.2) */
__kernel void compiler_math_constants()
{
  float f;
  f = MAXFLOAT;
  f = HUGE_VALF;
  f = HUGE_VAL;
  f = INFINITY;
  f = NAN;
  f = M_E_F;
  f = M_LOG2E_F;
  f = M_LOG10E_F;
  f = M_LN2_F;
  f = M_LN10_F;
  f = M_PI_F;
  f = M_PI_2_F;
  f = M_PI_4_F;
  f = M_1_PI_F;
  f = M_2_PI_F;
  f = M_2_SQRTPI_F;
  f = M_SQRT2_F;
  f = M_SQRT1_2_F;
}
