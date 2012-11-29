#include "utest_helper.hpp"

void compiler_vector_load_store(void)
{
  OCL_CREATE_KERNEL("compiler_vector_load_store");
}

MAKE_UTEST_FROM_FUNCTION(compiler_vector_load_store);


