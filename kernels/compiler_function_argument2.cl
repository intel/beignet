__kernel void compiler_function_argument2(
char8 c, uchar8 uc, short8 s, ushort8 us, int8 i, uint8 ui, float8 f,
__global float8 *result)
{
  result[0] = convert_float8(c);
  result[1] = convert_float8(uc);
  result[2] = convert_float8(s);
  result[3] = convert_float8(us);
  result[4] = convert_float8(i);
  result[5] = convert_float8(ui);
  result[6] = f;
}
