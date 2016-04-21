#include "utest_helper.hpp"

static void buildin_work_dim(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("buildin_work_dim");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  globals[0] = 1;
  globals[1] = 1;
  globals[2] = 1;
  locals[0] = 1;
  locals[1] = 1;
  locals[2] = 1;

  for( int i=1; i <= 3; i++ )
  {

    // Run the kernel
    OCL_NDRANGE(i);

    OCL_MAP_BUFFER(0);
    OCL_ASSERT( ((int*)buf_data[0])[0]== i);
    OCL_UNMAP_BUFFER(0);
  }
}

MAKE_UTEST_FROM_FUNCTION(buildin_work_dim);
