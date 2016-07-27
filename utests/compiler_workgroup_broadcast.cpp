#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

using namespace std;

/* set to 1 for debug, output of input-expected data */
#define DEBUG_STDOUT    0

/* NDRANGE */
#define WG_GLOBAL_SIZE_X        16
#define WG_GLOBAL_SIZE_Y        4
#define WG_GLOBAL_SIZE_Z        4

#define WG_LOCAL_SIZE_X         16
#define WG_LOCAL_SIZE_Y         2
#define WG_LOCAL_SIZE_Z         2

/* TODO debug bellow case, lid2 always stays 0, instead of 0 and 1
 *
 * #define WG_GLOBAL_SIZE_X        16
 * #define WG_GLOBAL_SIZE_Y        1
 * #define WG_GLOBAL_SIZE_Z        4
 *
 * #define WG_LOCAL_SIZE_X         16
 * #define WG_LOCAL_SIZE_Y         1
 * #define WG_LOCAL_SIZE_Z         2
 */

#define WG_LOCAL_X    5
#define WG_LOCAL_Y    0
#define WG_LOCAL_Z    0

enum WG_BROADCAST
{
  WG_BROADCAST_1D,
  WG_BROADCAST_2D,
  WG_BROADCAST_3D
};

/*
 * Generic compute-expected function for op BROADCAST type
 * and any variable type
 */
template<class T>
static void compute_expected(WG_BROADCAST wg_broadcast,
                             T* input,
                             T* expected,
                             uint32_t wg_global_size,
                             uint32_t wg_local_size)
{
  if(wg_broadcast == WG_BROADCAST_1D)
  {
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] = input[WG_LOCAL_X];
  }
  else if(wg_broadcast == WG_BROADCAST_2D)
  {
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] =
          input[WG_LOCAL_X +
                WG_LOCAL_Y * WG_LOCAL_SIZE_X];
  }
  else if(wg_broadcast == WG_BROADCAST_3D)
  {
    for(uint32_t i = 0; i < wg_local_size; i++)
      expected[i] =
        input[WG_LOCAL_X +
              WG_LOCAL_Y * WG_LOCAL_SIZE_X +
              WG_LOCAL_Z * WG_LOCAL_SIZE_X * WG_LOCAL_SIZE_Y];
  }
}

/*
 * Generic input-expected generate function for op BROADCAST type
 * and any variable type
 */
template<class T>
static void generate_data(WG_BROADCAST wg_broadcast,
                   T* &input,
                   T* &expected,
                   uint32_t &wg_global_size,
                   uint32_t &wg_local_size)
{
  if(wg_broadcast == WG_BROADCAST_1D)
  {
    wg_global_size = WG_GLOBAL_SIZE_X;
    wg_local_size = WG_LOCAL_SIZE_X;
  }
  else if(wg_broadcast == WG_BROADCAST_2D)
  {
    wg_global_size = WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y;
    wg_local_size = WG_LOCAL_SIZE_X * WG_LOCAL_SIZE_Y;
  }
  else if(wg_broadcast == WG_BROADCAST_3D)
  {
    wg_global_size = WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y * WG_GLOBAL_SIZE_Z;
    wg_local_size = WG_LOCAL_SIZE_X * WG_LOCAL_SIZE_Y * WG_LOCAL_SIZE_Z;
  }

  /* allocate input and expected arrays */
  input = new T[wg_global_size];
  expected = new T[wg_global_size];

  /* base value for all data types */
  T base_val = (long)7 << (sizeof(T) * 5 - 3);

  /* seed for random inputs */
  srand (time(NULL));

  /* generate inputs and expected values */
  for(uint32_t gid = 0; gid < wg_global_size; gid += wg_local_size)
  {
#if DEBUG_STDOUT
    cout << endl << "IN: " << endl;
#endif

    /* input values */
    for(uint32_t lid = 0; lid < wg_local_size; lid++)
    {
      /* initially 0, augment after */
      input[gid + lid] = 0;

      /* check all data types, test ideal for QWORD types */
      input[gid + lid] += ((rand() % 2 - 1) * base_val);
      /* add trailing random bits, tests GENERAL cases */
      input[gid + lid] += (rand() % 112);

#if DEBUG_STDOUT
      /* output generated input */
      cout << setw(4) << input[gid + lid] << ", " ;
      if((lid + 1) % 8 == 0)
        cout << endl;
#endif
    }

    /* expected values */
    compute_expected(wg_broadcast, input + gid, expected + gid, wg_global_size, wg_local_size);

#if DEBUG_STDOUT
    /* output expected input */
    cout << endl << "EXP: " << endl;
    for(uint32_t lid = 0; lid < wg_local_size; lid++){
      cout << setw(4) << expected[gid + lid] << ", " ;
      if((lid + 1) % 8 == 0)
        cout << endl;
    }
#endif

  }
}

/*
 * Generic workgroup utest function for op BROADCAST type
 * and any variable type
 */
template<class T>
static void workgroup_generic(WG_BROADCAST wg_broadcast,
                       T* input,
                       T* expected)
{
  uint32_t wg_global_size = 0;
  uint32_t wg_local_size = 0;

  cl_uint wg_local_x = WG_LOCAL_X;
  cl_uint wg_local_y = WG_LOCAL_Y;
  cl_uint wg_local_z = WG_LOCAL_Z;

  /* input and expected data */
  generate_data(wg_broadcast, input, expected, wg_global_size, wg_local_size);

  /* prepare input for datatype */
  OCL_CREATE_BUFFER(buf[0], 0, wg_global_size * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, wg_global_size * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_uint), &wg_local_x);
  OCL_SET_ARG(3, sizeof(cl_uint), &wg_local_y);
  OCL_SET_ARG(4, sizeof(cl_uint), &wg_local_z);

  /* set input data for GPU */
  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], input, wg_global_size * sizeof(T));
  OCL_UNMAP_BUFFER(0);

  /* run the kernel on GPU */
  if(wg_broadcast == WG_BROADCAST_1D)
  {
    globals[0] = WG_GLOBAL_SIZE_X;
    locals[0] = WG_LOCAL_SIZE_X;
    OCL_NDRANGE(1);
  }
  else if(wg_broadcast == WG_BROADCAST_2D)
  {
    globals[0] = WG_GLOBAL_SIZE_X;
    locals[0] = WG_LOCAL_SIZE_X;
    globals[1] = WG_GLOBAL_SIZE_Y;
    locals[1] = WG_LOCAL_SIZE_Y;
    OCL_NDRANGE(2);
  }
  else if(wg_broadcast == WG_BROADCAST_3D)
  {
    globals[0] = WG_GLOBAL_SIZE_X;
    locals[0] = WG_LOCAL_SIZE_X;
    globals[1] = WG_GLOBAL_SIZE_Y;
    locals[1] = WG_LOCAL_SIZE_Y;
    globals[2] = WG_GLOBAL_SIZE_Z;
    locals[2] = WG_LOCAL_SIZE_Y;
    OCL_NDRANGE(3);
  }

  /* check if mismatch */
  OCL_MAP_BUFFER(1);
  uint32_t mismatches = 0;

  for (uint32_t i = 0; i < wg_global_size; i++)
    if(((T *)buf_data[1])[i] != *(expected + i))
    {
      /* found mismatch, increment */
      mismatches++;

#if DEBUG_STDOUT
      /* output mismatch */
      cout << "Err at " << i << ", " <<
        ((T *)buf_data[1])[i] << " != " << *(expected + i) << endl;
#endif
    }

#if DEBUG_STDOUT
  /* output mismatch count */
  cout << "mismatches " << mismatches << endl;
#endif

  OCL_UNMAP_BUFFER(1);

  OCL_ASSERT(mismatches == 0);
}

/*
 * Workgroup broadcast 1D functions
 */
void compiler_workgroup_broadcast_1D_int(void)
{
  if (!cl_check_ocl20())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_broadcast",
                              "compiler_workgroup_broadcast_1D_int");
  workgroup_generic(WG_BROADCAST_1D, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_broadcast_1D_int);

void compiler_workgroup_broadcast_1D_long(void)
{
  if (!cl_check_ocl20())
    return;
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_broadcast",
                              "compiler_workgroup_broadcast_1D_long");
  workgroup_generic(WG_BROADCAST_1D, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_workgroup_broadcast_1D_long);

/*
 * Workgroup broadcast 2D functions
 */
void compiler_workgroup_broadcast_2D_int(void)
{
  if (!cl_check_ocl20())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_broadcast",
                              "compiler_workgroup_broadcast_2D_int");
  workgroup_generic(WG_BROADCAST_2D, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_broadcast_2D_int);

void compiler_workgroup_broadcast_2D_long(void)
{
  if (!cl_check_ocl20())
    return;
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_broadcast",
                              "compiler_workgroup_broadcast_2D_long");
  workgroup_generic(WG_BROADCAST_2D, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_workgroup_broadcast_2D_long);


/*
 * Workgroup broadcast 3D functions
 */
void compiler_workgroup_broadcast_3D_int(void)
{
  if (!cl_check_ocl20())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_broadcast",
                              "compiler_workgroup_broadcast_3D_int");
  workgroup_generic(WG_BROADCAST_3D, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_workgroup_broadcast_3D_int);

void compiler_workgroup_broadcast_3D_long(void)
{
  if (!cl_check_ocl20())
    return;
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_workgroup_broadcast",
                              "compiler_workgroup_broadcast_3D_long");
  workgroup_generic(WG_BROADCAST_3D, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_workgroup_broadcast_3D_long);
