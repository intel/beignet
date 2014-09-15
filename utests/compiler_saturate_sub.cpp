#include "utest_helper.hpp"

namespace {

constexpr int n = 16;

// declaration only, we should create each template specification for each type.
template<typename T>
T get_data(int idx, int part);

/* the format of test data is as follows:
 *   the first column is A
 *   the second column is B
 *   the third column is the expected result.
 */

#define DEF_TEMPLATE(TYPE, NAME)                                    \
template <>                                                         \
TYPE get_data<TYPE>(int idx, int part)                              \
{                                                                   \
  static TYPE test_data[n][3] = {                                   \
    { 0, 0, 0 },                                                    \
    { 0, 1, -1 },                                                   \
    { CL_##NAME##_MIN, CL_##NAME##_MIN, 0 },                        \
    { CL_##NAME##_MAX, CL_##NAME##_MAX, 0 },                        \
    { -2, CL_##NAME##_MIN, CL_##NAME##_MAX-1 },                     \
    { -1, CL_##NAME##_MIN, CL_##NAME##_MAX },                       \
    { 0, CL_##NAME##_MIN, CL_##NAME##_MAX },                        \
    { 1, CL_##NAME##_MIN, CL_##NAME##_MAX },                        \
    { -2, CL_##NAME##_MAX, CL_##NAME##_MIN },                       \
    { -1, CL_##NAME##_MAX, CL_##NAME##_MIN },                       \
    { 0, CL_##NAME##_MAX, -CL_##NAME##_MAX },                       \
    { 1, CL_##NAME##_MAX, -CL_##NAME##_MAX+1 },                     \
    { CL_##NAME##_MIN, CL_##NAME##_MAX, CL_##NAME##_MIN },          \
    { CL_##NAME##_MIN, 1, CL_##NAME##_MIN },                        \
    { CL_##NAME##_MIN, -1, CL_##NAME##_MIN+1 },                     \
    { CL_##NAME##_MAX, CL_##NAME##_MIN, CL_##NAME##_MAX },          \
  };                                                                \
  return test_data[idx][part];                                      \
}                                                                   \
                                                                    \
template <>                                                         \
u##TYPE get_data<u##TYPE>(int idx, int part)                        \
{                                                                   \
  static u##TYPE test_data[n][3] = {                                \
    { 0, 0, 0 },                                                    \
    { 0, 1, 0 },                                                    \
    { 1, 1, 0 },                                                    \
    { 1, 0, 1 },                                                    \
    { CL_U##NAME##_MAX, CL_U##NAME##_MAX, 0 },                      \
    { 0, CL_U##NAME##_MAX, 0 },                                     \
    { 1, CL_U##NAME##_MAX, 0 },                                     \
    { CL_U##NAME##_MAX, 0, CL_U##NAME##_MAX },                      \
  };                                                                \
  return test_data[idx][part];                                      \
}

DEF_TEMPLATE(int8_t, CHAR)
DEF_TEMPLATE(int16_t, SHRT)
DEF_TEMPLATE(int32_t, INT)
//DEF_TEMPLATE(int64_t, LONG)


template<typename T>
void test(const char *kernel_name)
{
  T C[n] = { 0 };
  T A[n] = { 0 };
  T B[n] = { 0 };

  for (int i = 0; i < n; i++) {
    A[i] = get_data<T>(i, 0);
    B[i] = get_data<T>(i, 1);
  }

  OCL_CREATE_KERNEL_FROM_FILE("compiler_saturate_sub", kernel_name);

  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(T), &C[0]);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_COPY_HOST_PTR, n * sizeof(T), &A[0]);
  OCL_CREATE_BUFFER(buf[2], CL_MEM_COPY_HOST_PTR, n * sizeof(T), &B[0]);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);

  globals[0] = n;
  locals[0] = n;
  OCL_NDRANGE(1);

  OCL_MAP_BUFFER(0);

  for (int i = 0; i < n; i++) {
    OCL_ASSERT(((T*)buf_data[0])[i] == get_data<T>(i, 2));
  }
  OCL_UNMAP_BUFFER(0);
}

}

#define compiler_saturate_sub(type, kernel) \
static void compiler_saturate_sub_ ##type(void)\
{\
  test<type>(# kernel);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_saturate_sub_ ## type);

compiler_saturate_sub(int8_t, test_char)
compiler_saturate_sub(uint8_t, test_uchar)
compiler_saturate_sub(int16_t, test_short)
compiler_saturate_sub(uint16_t, test_ushort)
compiler_saturate_sub(int32_t, test_int)
compiler_saturate_sub(uint32_t, test_uint)
//compiler_saturate_sub(int64_t, test_long)
//compiler_saturate_sub(uint64_t, test_ulong)
