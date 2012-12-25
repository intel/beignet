#include "utest_helper.hpp"

static void compiler_multiple_kernels(void)
{
	OCL_CREATE_KERNEL_FROM_FILE("compiler_multiple_kernels", "first_kernel");
}

MAKE_UTEST_FROM_FUNCTION(compiler_multiple_kernels);