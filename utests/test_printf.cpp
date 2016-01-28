#include "utest_helper.hpp"

void test_printf(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL("test_printf");
  globals[0] = 16;
  locals[0] = 16;
  globals[1] = 4;
  locals[1] = 4;
  globals[2] = 8;
  locals[2] = 2;

  // Run the kernel on GPU
  OCL_NDRANGE(3);
}

MAKE_UTEST_FROM_FUNCTION(test_printf);

void test_printf_1(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("test_printf", "test_printf_1");
  globals[0] = 1;
  locals[0] = 1;

  // Run the kernel on GPU
  OCL_NDRANGE(1);
}

MAKE_UTEST_FROM_FUNCTION(test_printf_1);

void test_printf_2(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("test_printf", "test_printf_2");
  globals[0] = 4;
  locals[0] = 2;

  // Run the kernel on GPU
  OCL_NDRANGE(1);
}

MAKE_UTEST_FROM_FUNCTION(test_printf_2);

void test_printf_3(void)
{
  char c = '@';
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("test_printf", "test_printf_3");
  globals[0] = 1;
  locals[0] = 1;
  OCL_SET_ARG(0, sizeof(char), &c);

  // Run the kernel on GPU
  OCL_NDRANGE(1);
}

MAKE_UTEST_FROM_FUNCTION(test_printf_3);

void test_printf_4(void)
{
  // Setup kernel and buffers
  OCL_CREATE_KERNEL_FROM_FILE("test_printf", "test_printf_4");
  globals[0] = 1;
  locals[0] = 1;

  // Run the kernel on GPU
  OCL_NDRANGE(1);
}

MAKE_UTEST_FROM_FUNCTION(test_printf_4);
