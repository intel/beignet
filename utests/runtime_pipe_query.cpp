#include <string.h>
#include "utest_helper.hpp"
static void runtime_pipe_query(void) {
  const size_t w = 16;
  const size_t sz = 8;
  cl_uint retnum, retsz;
  /* pipe write kernel */
  OCL_CALL2(clCreatePipe, buf[0], ctx, 0, sz, w, NULL);
  OCL_CALL(clGetPipeInfo, buf[0], CL_PIPE_MAX_PACKETS, sizeof(retnum), &retnum, NULL);
  OCL_CALL(clGetPipeInfo, buf[0], CL_PIPE_PACKET_SIZE, sizeof(retsz), &retsz, NULL);

  /*Check result */
  OCL_ASSERT(sz == retsz && w == retnum);
}
MAKE_UTEST_FROM_FUNCTION(runtime_pipe_query);
