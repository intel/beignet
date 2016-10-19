#include <cstdint>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include "utest_helper.hpp"

using namespace std;

/* set to 1 for debug, output of input-expected data */
#define DEBUG_STDOUT    0

/* NDRANGE */
#define WG_GLOBAL_SIZE  30
#define WG_LOCAL_SIZE   30
enum WG_FUNCTION
{
  WG_ANY,
  WG_ALL,
  WG_REDUCE_ADD,
  WG_REDUCE_MIN,
  WG_REDUCE_MAX
};

/*
 * Generic compute-expected function for op REDUCE/ANY/ALL
 * and any variable type
 */
template<class T>
static void compute_expected(WG_FUNCTION wg_func,
                    T* input,
                    T* expected,
                    size_t SIMD_SIZE,
                    bool IS_HALF)
{
  if(wg_func == WG_ANY)
  {
    T wg_predicate = input[0];
    for(uint32_t i = 1; i < SIMD_SIZE; i++)
      wg_predicate = (int)wg_predicate || (int)input[i];
    for(uint32_t i = 0; i < SIMD_SIZE; i++)
      expected[i] = wg_predicate;
  }
  else if(wg_func == WG_ALL)
  {
    T wg_predicate = input[0];
    for(uint32_t i = 1; i < SIMD_SIZE; i++)
      wg_predicate = (int)wg_predicate && (int)input[i];
    for(uint32_t i = 0; i < SIMD_SIZE; i++)
      expected[i] = wg_predicate;
  }
  else if(wg_func == WG_REDUCE_ADD)
  {
    T wg_sum = input[0];
    if(IS_HALF) {
      float wg_sum_tmp = 0.0f;
      for(uint32_t i = 0; i < SIMD_SIZE; i++) {
        wg_sum_tmp += as_float(__half_to_float(input[i]));
      }
      wg_sum = __float_to_half(as_uint(wg_sum_tmp));
    }
    else {
      for(uint32_t i = 1; i < SIMD_SIZE; i++)
        wg_sum += input[i];
    }
    for(uint32_t i = 0; i < SIMD_SIZE; i++)
      expected[i] = wg_sum;
  }
  else if(wg_func == WG_REDUCE_MAX)
  {
    T wg_max = input[0];
    for(uint32_t i = 1; i < SIMD_SIZE; i++) {
      if (IS_HALF) {
        wg_max = (as_float(__half_to_float(input[i])) > as_float(__half_to_float(wg_max))) ? input[i] : wg_max;
      }
      else
        wg_max = max(input[i], wg_max);
    }
    for(uint32_t i = 0; i < SIMD_SIZE; i++)
      expected[i] = wg_max;
  }
  else if(wg_func == WG_REDUCE_MIN)
  {
    T wg_min = input[0];
    for(uint32_t i = 1; i < SIMD_SIZE; i++) {
      if (IS_HALF) {
        wg_min= (as_float(__half_to_float(input[i])) < as_float(__half_to_float(wg_min))) ? input[i] : wg_min;
      }
      else
        wg_min = min(input[i], wg_min);
    }
    for(uint32_t i = 0; i < SIMD_SIZE; i++)
      expected[i] = wg_min;
  }
}

/*
 * Generic input-expected generate function for op REDUCE/ANY/ALL
 * and any variable type
 */
template<class T>
static void generate_data(WG_FUNCTION wg_func,
                   T* &input,
                   T* &expected,
                   size_t SIMD_SIZE,
                   bool IS_HALF)
{
  input = new T[WG_GLOBAL_SIZE];
  expected = new T[WG_GLOBAL_SIZE];

  /* base value for all data types */
  T base_val = (long)7 << (sizeof(T) * 5 - 3);

  /* seed for random inputs */
  srand (time(NULL));

  /* generate inputs and expected values */
  for(uint32_t gid = 0; gid < WG_GLOBAL_SIZE; gid += SIMD_SIZE)
  {
#if DEBUG_STDOUT
    cout << endl << "IN: " << endl;
#endif
    SIMD_SIZE = (gid + SIMD_SIZE) > WG_GLOBAL_SIZE ? WG_GLOBAL_SIZE - gid : SIMD_SIZE;

    /* input values */
    for (uint32_t lid = 0; lid < SIMD_SIZE; lid++) {
      /* initially 0, augment after */
      input[gid + lid] = 0;

      if (numeric_limits<T>::is_integer) {
        /* check all data types, test ideal for QWORD types */
        input[gid + lid] += ((rand() % 2 - 1) * base_val);
        /* add trailing random bits, tests GENERAL cases */
        input[gid + lid] += (rand() % 112);
        /* always last bit is 1, ideal test ALL/ANY */
        if (IS_HALF)
          input[gid + lid] = __float_to_half(as_uint((float)input[gid + lid]/2));
      } else {
        input[gid + lid] += rand();
        input[gid + lid] += rand() / ((float)RAND_MAX + 1);
      }

#if DEBUG_STDOUT
      /* output generated input */
      cout << setw(4) << input[gid + lid] << ", " ;
      if((lid + 1) % 8 == 0)
        cout << endl;
#endif
    }

    /* expected values */
    compute_expected(wg_func, input + gid, expected + gid, SIMD_SIZE, IS_HALF);

#if DEBUG_STDOUT
    /* output expected input */
    cout << endl << "EXP: " << endl;
    for(uint32_t lid = 0; lid < SIMD_SIZE; lid++) {
      cout << setw(4) << expected[gid + lid] << ", " ;
      if((lid + 1) % 8 == 0)
        cout << endl;
    }
    cout << endl;
#endif

  }
}

/*
 * Generic subgroup utest function for op REDUCE/ANY/ALL
 * and any variable type
 */
template<class T>
static void subgroup_generic(WG_FUNCTION wg_func,
                       T* input,
                       T* expected,
                       bool IS_HALF = false)
{
  /* get simd size */
  globals[0] = WG_GLOBAL_SIZE;
  locals[0] = WG_LOCAL_SIZE;
  size_t SIMD_SIZE = 0;
  OCL_CALL(utestclGetKernelSubGroupInfoKHR,kernel,device,CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR,sizeof(size_t)*1,locals,sizeof(size_t),&SIMD_SIZE,NULL);

  /* input and expected data */
  generate_data(wg_func, input, expected, SIMD_SIZE, IS_HALF);

  /* prepare input for data type */
  OCL_CREATE_BUFFER(buf[0], 0, WG_GLOBAL_SIZE * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, WG_GLOBAL_SIZE * sizeof(T), NULL);

  /* set input data for GPU */
  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], input, WG_GLOBAL_SIZE * sizeof(T));
  OCL_UNMAP_BUFFER(0);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  /* run the kernel on GPU */
  OCL_NDRANGE(1);

  /* check if mismatch */
  OCL_MAP_BUFFER(1);
  uint32_t mismatches = 0;

  for (uint32_t i = 0; i < WG_GLOBAL_SIZE; i++)
    if(((T *)buf_data[1])[i] != *(expected + i))
    {
      if (IS_HALF) {
        float num_computed = as_float(__half_to_float(((T *)buf_data[1])[i]));
        float num_expected = as_float(__half_to_float(*(expected + i)));
        float num_diff = abs(num_computed - num_expected) / abs(num_expected);
        if (num_diff > 0.03f) {
          mismatches++;
        }
#if DEBUG_STDOUT
          /* output mismatch */
          cout << "Err at " << i << ", " << num_computed
               << " != " << num_expected << " diff: " <<num_diff <<endl;
#endif
        //}
      }
      /* found mismatch on integer, increment */
      else if (numeric_limits<T>::is_integer) {
        mismatches++;

#if DEBUG_STDOUT
        /* output mismatch */
        cout << "Err at " << i << ", " << ((T *)buf_data[1])[i]
             << " != " << *(expected + i) << endl;
#endif
      }
      /* float error is tolerable though */
      else {
        float num_computed = ((T *)buf_data[1])[i];
        float num_expected = *(expected + i);
        float num_diff = abs(num_computed - num_expected) / abs(num_expected);
        if (num_diff > 0.01f) {
          mismatches++;

#if DEBUG_STDOUT
          /* output mismatch */
          cout << "Err at " << i << ", " << ((T *)buf_data[1])[i]
               << " != " << *(expected + i) << endl;
#endif
        }
      }
    }

#if DEBUG_STDOUT
  /* output mismatch count */
  cout << "mismatches " << mismatches << endl;
#endif

  OCL_UNMAP_BUFFER(1);

  OCL_ASSERT(mismatches == 0);
}

/*
 * Workgroup any/all utest functions
 */
void compiler_subgroup_any(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_any");
  subgroup_generic(WG_ANY, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_any);
void compiler_subgroup_all(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_all");
  subgroup_generic(WG_ALL, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_all);
/*
 * Workgroup reduce add utest functions
 */
void compiler_subgroup_reduce_add_int(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_int");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_add_int);
void compiler_subgroup_reduce_add_uint(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_uint");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_add_uint);
void compiler_subgroup_reduce_add_long(void)
{
  if(!cl_check_subgroups())
    return;
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_long");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_reduce_add_long);
void compiler_subgroup_reduce_add_ulong(void)
{
  if(!cl_check_subgroups())
    return;
  cl_ulong *input = NULL;
  cl_ulong *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_ulong");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_reduce_add_ulong);
void compiler_subgroup_reduce_add_float(void)
{
  if(!cl_check_subgroups())
    return;
  cl_float *input = NULL;
  cl_float *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_float");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_add_float);
void compiler_subgroup_reduce_add_half(void)
{
  if(!cl_check_subgroups())
    return;
  if(!cl_check_half())
    return;
  cl_half *input = NULL;
  cl_half *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_reduce.cl",
                           "compiler_subgroup_reduce_add_half",
                           SOURCE, "-DHALF");
  subgroup_generic(WG_REDUCE_ADD, input, expected, true);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_add_half);
void compiler_subgroup_reduce_add_short(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_short *input = NULL;
  cl_short *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_short");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_add_short);
void compiler_subgroup_reduce_add_ushort(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_add_ushort");
  subgroup_generic(WG_REDUCE_ADD, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_add_ushort);

/*
 * Workgroup reduce max utest functions
 */
void compiler_subgroup_reduce_max_int(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_int");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_max_int);
void compiler_subgroup_reduce_max_uint(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_uint");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_max_uint);
void compiler_subgroup_reduce_max_long(void)
{
  if(!cl_check_subgroups())
    return;
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_long");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_reduce_max_long);
void compiler_subgroup_reduce_max_ulong(void)
{
  if(!cl_check_subgroups())
    return;
  cl_ulong *input = NULL;
  cl_ulong *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_ulong");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_reduce_max_ulong);
void compiler_subgroup_reduce_max_float(void)
{
  if(!cl_check_subgroups())
    return;
  cl_float *input = NULL;
  cl_float *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_float");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_max_float);
void compiler_subgroup_reduce_max_half(void)
{
  if(!cl_check_subgroups())
    return;
  if(!cl_check_half())
    return;
  cl_half *input = NULL;
  cl_half *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_reduce.cl",
                           "compiler_subgroup_reduce_max_half",
                           SOURCE, "-DHALF");
  subgroup_generic(WG_REDUCE_MAX, input, expected, true);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_max_half);
void compiler_subgroup_reduce_max_short(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_short *input = NULL;
  cl_short *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_short");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_max_short);
void compiler_subgroup_reduce_max_ushort(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_max_ushort");
  subgroup_generic(WG_REDUCE_MAX, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_max_ushort);

/*
 * Workgroup reduce min utest functions
 */
void compiler_subgroup_reduce_min_int(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_int");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_min_int);
void compiler_subgroup_reduce_min_uint(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_uint");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_min_uint);
void compiler_subgroup_reduce_min_long(void)
{
  if(!cl_check_subgroups())
    return;
  cl_long *input = NULL;
  cl_long *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_long");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_reduce_min_long);
void compiler_subgroup_reduce_min_ulong(void)
{
  if(!cl_check_subgroups())
    return;
  cl_ulong *input = NULL;
  cl_ulong *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_ulong");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_reduce_min_ulong);
void compiler_subgroup_reduce_min_float(void)
{
  if(!cl_check_subgroups())
    return;
  cl_float *input = NULL;
  cl_float *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_float");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_min_float);
void compiler_subgroup_reduce_min_half(void)
{
  if(!cl_check_subgroups())
    return;
  if(!cl_check_half())
    return;
  cl_half *input = NULL;
  cl_half *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_reduce.cl",
                           "compiler_subgroup_reduce_min_half",
                           SOURCE, "-DHALF");
  subgroup_generic(WG_REDUCE_MIN, input, expected, true);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_min_half);
void compiler_subgroup_reduce_min_short(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_short *input = NULL;
  cl_short *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_short");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_min_short);
void compiler_subgroup_reduce_min_ushort(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_reduce",
                              "compiler_subgroup_reduce_min_ushort");
  subgroup_generic(WG_REDUCE_MIN, input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_reduce_min_ushort);
