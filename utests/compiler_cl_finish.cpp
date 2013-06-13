#include "utest_helper.hpp"
#include <sys/time.h>

#define T_GET(t)        gettimeofday(&t, NULL);
#define T_LAPSE(t1, t2) \
  ((t2.tv_sec+t2.tv_usec*0.000001) - (t1.tv_sec+t1.tv_usec*0.000001))

static void compiler_cl_finish(void)
{
  const size_t n = 16*1024*1024;
  struct timeval t1, t2;
  float t_fin, t_map_w_fin,t_map_wo_fin;

  // Setup kernel and buffers
  OCL_CREATE_KERNEL("test_cl_finish");
  OCL_CREATE_BUFFER(buf[0], 0, n * sizeof(int), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, n * sizeof(int), NULL);

  // Run the kernel
  locals[0]  = 64;
  globals[0] = 32 * locals[0];
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(int), &n);
  OCL_SET_ARG(3, sizeof(int), &globals[0]);

  // 1st time map after clFinish
  OCL_NDRANGE(1);
  T_GET(t1);
  OCL_FINISH();
  T_GET(t2);
  t_fin = T_LAPSE(t1, t2);

  T_GET(t1);
  OCL_MAP_BUFFER(0);
  T_GET(t2);
  t_map_w_fin = T_LAPSE(t1, t2);

  // 2nd time map without clFinish
  OCL_NDRANGE(1);
  T_GET(t1);
  OCL_MAP_BUFFER(0);
  T_GET(t2);
  t_map_wo_fin = T_LAPSE(t1, t2);

  OCL_ASSERT(t_fin > t_map_w_fin && t_map_wo_fin > t_map_w_fin);
  OCL_UNMAP_BUFFER(0);
}

MAKE_UTEST_FROM_FUNCTION(compiler_cl_finish);
