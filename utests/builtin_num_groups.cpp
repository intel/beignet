/*
According to the OpenCL v1.1 & v1.2 chapter 6.11, the behavior of function get_num_groups should be as following:

  globals[0] = 1;
  globals[1] = 4;
  globals[2] = 9;
  locals[0] = 1;
  locals[1] = 2;
  locals[2] = 3;

#ifdef CL_VERSION_1_2 | CL_VERSION_1_1:
get_num_groups(-1) = 1 (dimension:1)
get_num_groups(0) = 1 (dimension:1)
get_num_groups(1) = 1 (dimension:1)

get_num_groups(-1) = 1 (dimension:2)
get_num_groups(0) = 1 (dimension:2)
get_num_groups(1) = 2 (dimension:2)
get_num_groups(2) = 1 (dimension:2)

get_num_groups(-1) = 1 (dimension:3)
get_num_groups(0) = 1 (dimension:3)
get_num_groups(1) = 2 (dimension:3)
get_num_groups(2) = 3 (dimension:3)
get_num_groups(3) = 1 (dimension:3)
*/

#define udebug 0
#include "utest_helper.hpp"
static void builtin_num_groups(void)
{

  // Setup kernel and buffers
  int dim, dim_arg_global, num_groups, err;
  OCL_CREATE_KERNEL("builtin_num_groups");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], CL_MEM_READ_WRITE, sizeof(int), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  globals[0] = 1;
  globals[1] = 4;
  globals[2] = 9;
  locals[0] = 1;
  locals[1] = 2;
  locals[2] = 3;

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

      err = clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(int), &num_groups, 0, NULL, NULL);
      if (err != CL_SUCCESS)
      {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
      }

#if udebug
      printf("get_num_groups(%d) = %d (dimension:%d)\n", dim_arg_global, num_groups, dim);
#endif
      if ( dim_arg_global >= 0 && dim_arg_global < dim)
        OCL_ASSERT( num_groups == dim_arg_global + 1 );
      else
      {
        OCL_ASSERT( num_groups == 1);
      }
    }
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_num_groups);
