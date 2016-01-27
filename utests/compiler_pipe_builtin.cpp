#include <string.h>
#include "utest_helper.hpp"
typedef struct{
  int a;
  uint b;
}mystruct;

#define PIPE_BUILTIN(TYPE,GROUP) \
static void compiler_pipe_##GROUP##_##TYPE(void) \
{ \
  const size_t w = 16;  \
  uint32_t ans_host = 0;  \
  uint32_t ans_device = 0;  \
  /* pipe write kernel*/  \
  OCL_CREATE_KERNEL_FROM_FILE("compiler_pipe_builtin", "compiler_pipe_"#GROUP"_write_"#TYPE);  \
  OCL_CALL2(clCreatePipe, buf[0], ctx, 0, sizeof(TYPE), w, NULL);\
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, w * sizeof(TYPE), NULL);\
  OCL_MAP_BUFFER(1);\
  for (uint32_t i = 0; i < w; i++)\
      ((uint32_t*)buf_data[1])[i] = i;\
  OCL_UNMAP_BUFFER(1);\
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);\
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);\
  globals[0] = w;\
  locals[0] = 16;\
  OCL_NDRANGE(1);\
  OCL_CALL(clReleaseKernel, kernel);\
  /* pipe read kernel */\
  OCL_CREATE_KERNEL_FROM_FILE("compiler_pipe_builtin", "compiler_pipe_"#GROUP"_read_"#TYPE);\
  OCL_CREATE_BUFFER(buf[2], CL_MEM_READ_WRITE, w * sizeof(TYPE), NULL);\
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);\
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[2]);\
  OCL_NDRANGE(1);\
  /* Check result */\
  OCL_MAP_BUFFER(2);\
  for (uint32_t i = 0; i < w; i++) {\
      ans_device += ((uint32_t*)buf_data[2])[i];\
      ans_host += i;\
  }\
  OCL_UNMAP_BUFFER(2);\
  OCL_ASSERT(ans_host == ans_device);\
}\
MAKE_UTEST_FROM_FUNCTION(compiler_pipe_##GROUP##_##TYPE);

PIPE_BUILTIN(int, convenience)
PIPE_BUILTIN(mystruct, convenience)
PIPE_BUILTIN(int, reserve)
PIPE_BUILTIN(mystruct, reserve)
PIPE_BUILTIN(int, workgroup)
PIPE_BUILTIN(mystruct, workgroup)

static void compiler_pipe_query(void) {
  const size_t w = 32;
  const size_t sz = 16;
  /* pipe write kernel */
  OCL_CREATE_KERNEL_FROM_FILE("compiler_pipe_builtin", "compiler_pipe_query");
  OCL_CALL2(clCreatePipe, buf[0], ctx, 0, sizeof(uint32_t), w, NULL);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, sz * sizeof(uint32_t), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  globals[0] = sz;
  locals[0] = 16;
  OCL_NDRANGE(1);
  /*Check result */
  OCL_MAP_BUFFER(1);
  OCL_ASSERT(sz == ((uint32_t *)buf_data[1])[0] && w == ((uint32_t *)buf_data[1])[1]);
  OCL_UNMAP_BUFFER(2);
}
MAKE_UTEST_FROM_FUNCTION(compiler_pipe_query);
