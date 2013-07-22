/*
According to the OpenCL v1.1 & v1.2 chapter 6.11.
Now define global size as following:
  globals[0] = 3;
  globals[1] = 4;
  globals[2] = 5;

Kernel:
id = get_global_id(0) + get_global_id(1)*3 + get_global_id(2)*3*4

dimension:1
 0  1  2
dimension:2
 0  1  2
 3  4  5
 6  7  8
 9 10 11
dimension:3
 0  1  2   12 13 14   24 25 26   36 37 38   48 49 50
 3  4  5   15 16 17   27 28 29   39 40 41   51 52 53
 6  7  8   18 19 20   30 31 32   42 43 44   54 55 56
 9 10 11   21 22 23   33 34 35   45 46 47   57 58 59
*/

#define udebug 0
#include "utest_helper.hpp"
static void builtin_global_id(void)
{

  // Setup kernel and buffers
  int dim, global_id[80], err, i, buf_len=1;
  OCL_CREATE_KERNEL("builtin_global_id");

  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, sizeof(int)*80, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  for( dim=1; dim <= 3; dim++ )
  {
    buf_len = 1;
    for(i=1; i <= dim; i++)
    {
      globals[i - 1] = 2 + i;
      locals[i - 1] = 2 + i;
      buf_len *= 2 + i;
    }
    for(i=dim+1; i <= 3; i++)
    {
      globals[i - 1] = 0;
      locals[i - 1] = 0;
    }

    // Run the kernel
    OCL_NDRANGE( dim );
    clFinish(queue);

    err = clEnqueueReadBuffer( queue, buf[0], CL_TRUE, 0, sizeof(int) * buf_len, &global_id, 0, NULL, NULL);

    if (err != CL_SUCCESS)
    {
      printf("Error: Failed to read output array! %d\n", err);
      exit(1);
    }

#if udebug
    for(i = 0; i < buf_len; i++)
    {
      printf("%2d ", global_id[i]);
      if ((i + 1) % 3 == 0) printf("\n");
    }
#endif

    for( i = 0; i < buf_len; i++)
      OCL_ASSERT( global_id[i] == i);
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_global_id);
