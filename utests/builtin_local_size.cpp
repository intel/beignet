/*
According to the OpenCL v1.1 & v1.2 chapter 6.11, the behavior of function get_local_size should be as following:

  globals[0] = 3;
  globals[1] = 4;
  globals[2] = 5;
  locals[0] = 3;
  locals[1] = 4;
  locals[2] = 5;

get_local_size(-1) = 1 (dimension:1)
get_local_size(0) = 3 (dimension:1)
get_local_size(1) = 1 (dimension:1)
get_local_size(2) = 1 (dimension:1)

get_local_size(-1) = 1 (dimension:2)
get_local_size(0) = 3 (dimension:2)
get_local_size(1) = 4 (dimension:2)
get_local_size(2) = 1 (dimension:2)
get_local_size(3) = 1 (dimension:2)

get_local_size(-1) = 1 (dimension:3)
get_local_size(0) = 3 (dimension:3)
get_local_size(1) = 4 (dimension:3)
get_local_size(2) = 5 (dimension:3)
get_local_size(3) = 1 (dimension:3)
get_local_size(4) = 1 (dimension:3)

*/
#include "utest_helper.hpp"
#define udebug 0

static void builtin_local_size(void)
{

  // Setup kernel and buffers
  int dim, dim_arg_global, local_size, err;
  OCL_CREATE_KERNEL("builtin_local_size");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  globals[0] = 3;
  globals[1] = 4;
  globals[2] = 5;
  locals[0] = 3;
  locals[1] = 4;
  locals[2] = 5;

  for( dim=1; dim <= 3; dim++ )
  {

    for( dim_arg_global = -1; dim_arg_global <= dim + 1; dim_arg_global++ )
    {

      err = clEnqueueWriteBuffer( queue, buf[1], CL_TRUE, 0, sizeof(int), &dim_arg_global, 0, NULL, NULL);
      if (err != CL_SUCCESS)
      {
        printf("Error: Failed to write to source array!\n");
        exit(1);
      }

      // Run the kernel
      OCL_NDRANGE( dim );

      err = clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(int), &local_size, 0, NULL, NULL);
      if (err != CL_SUCCESS)
      {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
      }

#if udebug
      printf("get_local_size(%d) = %d (dimension:%d)\n", dim_arg_global, local_size, dim);
#endif
      if ( dim_arg_global >= 0 && dim_arg_global < dim)
        OCL_ASSERT( local_size == dim_arg_global + 3);
      else
      {
        OCL_ASSERT( local_size == 1);
      }
    }
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_local_size);
