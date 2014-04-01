#include "utest_helper.hpp"

enum eTestOP {
  TEST_OP_ADD =0,
  TEST_OP_SUB,
  TEST_OP_MUL,
  TEST_OP_DIV,
  TEST_OP_REM
};

template <typename T, eTestOP op>
static void test_exec(const char* kernel_name)
{
  const size_t n = 160;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("compiler_basic_arithmetic", kernel_name);
  buf_data[0] = (T*) malloc(sizeof(T) * n);
  buf_data[1] = (T*) malloc(sizeof(T) * n);
  for (uint32_t i = 0; i < n; ++i) ((T*)buf_data[0])[i] = (T) rand();
  for (uint32_t i = 0; i < n; ++i) ((T*)buf_data[1])[i] = (T) rand();
  if(op == TEST_OP_DIV || op == TEST_OP_REM) {
    for (uint32_t i = 0; i < n; ++i) {
      if(((T*)buf_data[1])[i] == 0)
       ((T*)buf_data[1])[i] = (T) 1;
    }
  }
  OCL_CREATE_BUFFER(buf[0], CL_MEM_COPY_HOST_PTR, n * sizeof(T), buf_data[0]);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_COPY_HOST_PTR, n * sizeof(T), buf_data[1]);
  OCL_CREATE_BUFFER(buf[2], 0, n * sizeof(T), NULL);

  // Run the kernel
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  globals[0] = n;
  locals[0] = 16;
  OCL_NDRANGE(1);

  // Check result
  OCL_MAP_BUFFER(2);
  if(op == TEST_OP_SUB) {
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((T*)buf_data[2])[i] == (T)(((T*)buf_data[0])[i] - ((T*)buf_data[1])[i]));
  } else if(op == TEST_OP_ADD) {
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((T*)buf_data[2])[i] == (T)(((T*)buf_data[0])[i] + ((T*)buf_data[1])[i]));
  } else if(op == TEST_OP_MUL) {
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((T*)buf_data[2])[i] == (T)(((T*)buf_data[0])[i] * ((T*)buf_data[1])[i]));
  } else if(op == TEST_OP_DIV) {
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((T*)buf_data[2])[i] == (T)(((T*)buf_data[0])[i] / ((T*)buf_data[1])[i]));
  } else {
    for (uint32_t i = 0; i < n; ++i)
      OCL_ASSERT(((T*)buf_data[2])[i] == (T)(((T*)buf_data[0])[i] % ((T*)buf_data[1])[i]));
  }
  free(buf_data[0]);
  free(buf_data[1]);
  buf_data[0] = buf_data[1] = NULL;
}

#define DECL_TEST_SUB(type, alias, keep_program) \
static void compiler_sub_ ##alias(void)\
{\
  test_exec<type, TEST_OP_SUB>("compiler_sub_" # alias);\
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_sub_ ## alias, keep_program)

#define DECL_TEST_ADD(type, alias, keep_program) \
static void compiler_add_ ##alias(void)\
{\
  test_exec<type, TEST_OP_ADD>("compiler_add_" # alias);\
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_add_ ## alias, keep_program)

#define DECL_TEST_MUL(type, alias, keep_program) \
static void compiler_mul_ ##alias(void)\
{\
  test_exec<type, TEST_OP_MUL>("compiler_mul_" # alias);\
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_mul_ ## alias, keep_program)

#define DECL_TEST_DIV(type, alias, keep_program) \
static void compiler_div_ ##alias(void)\
{\
  test_exec<type, TEST_OP_DIV>("compiler_div_" # alias);\
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_div_ ## alias, keep_program)

#define DECL_TEST_REM(type, alias, keep_program) \
static void compiler_rem_ ##alias(void)\
{\
  test_exec<type, TEST_OP_REM>("compiler_rem_" # alias);\
}\
MAKE_UTEST_FROM_FUNCTION_KEEP_PROGRAM(compiler_rem_ ## alias, keep_program)

#define _DECL_TEST_FOR_ALL_TYPE(op, keep_program) \
DECL_TEST_##op(int8_t, char, true) \
DECL_TEST_##op(uint8_t, uchar, true) \
DECL_TEST_##op(int16_t, short, true) \
DECL_TEST_##op(uint16_t, ushort, true) \
DECL_TEST_##op(int32_t, int, true) \
DECL_TEST_##op(uint32_t, uint, keep_program)

#define DECL_TEST_FOR_ALL_TYPE(op) _DECL_TEST_FOR_ALL_TYPE(op, true)

#define DECL_TEST_FOR_ALL_TYPE_END(op) _DECL_TEST_FOR_ALL_TYPE(op, false)

DECL_TEST_FOR_ALL_TYPE(SUB)
DECL_TEST_FOR_ALL_TYPE(ADD)
DECL_TEST_FOR_ALL_TYPE(MUL)
DECL_TEST_FOR_ALL_TYPE(DIV)
DECL_TEST_FOR_ALL_TYPE_END(REM)
#undef DECL_TEST_FOR_ALL_TYPE
