/* test OpenCL 1.1 Synchronization, explicit memory fence (section 6.11.9, 6.11.10) */
#include "utest_helper.hpp"

void compiler_mem_fence(void)
{
  OCL_CREATE_KERNEL("compiler_mem_fence");
  OCL_NDRANGE(1);
}

