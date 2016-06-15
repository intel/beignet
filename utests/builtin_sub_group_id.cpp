/*
According to the OpenCL cl_intel_subgroups.
Now define local and global size as following:
  globals[0] = 4;
  globals[1] = 9;
  globals[2] = 16;
  locals[0] = 2;
  locals[1] = 3;
  locals[2] = 4;
*/

#define udebug 0
#include "utest_helper.hpp"
static void builtin_sub_group_id(void)
{
  if(!cl_check_subgroups())
    return;

  // Setup kernel and buffers
  size_t dim, i,local_sz = 1,buf_len = 1;
  OCL_CREATE_KERNEL("builtin_sub_group_id");
  size_t max_sub_sz;


  OCL_CREATE_BUFFER(buf[0], CL_MEM_READ_WRITE, sizeof(int)*576, NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);

  for( dim=1; dim <= 3; dim++ )
  {
    buf_len = 1;
    local_sz = 1;
    for(i=1; i <= dim; i++)
    {
      locals[i - 1] = i + 1;
      globals[i - 1] = (i + 1) * (i + 1);
      buf_len *= ((i + 1) * (i + 1));
      local_sz *= i + 1;
    }
    for(i = dim+1; i <= 3; i++)
    {
      globals[i - 1] = 0;
      locals[i - 1] = 0;
    }

    OCL_CALL(utestclGetKernelSubGroupInfoKHR,kernel,device,CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR,sizeof(size_t)*dim,locals,sizeof(size_t),&max_sub_sz,NULL);
    // Run the kernel
    OCL_NDRANGE( dim );
    clFinish(queue);

    OCL_MAP_BUFFER(0);

    for( i = 0; i < buf_len; i++) {
      size_t expect_id = (i % local_sz) / max_sub_sz;
#if udebug
      printf("%zu get %d, expect %zu\n",i, ((uint32_t*)buf_data[0])[i], expect_id);
#endif
      OCL_ASSERT( ((uint32_t*)buf_data[0])[i] == expect_id);
    }
    OCL_UNMAP_BUFFER(0);
  }
}

MAKE_UTEST_FROM_FUNCTION(builtin_sub_group_id);
