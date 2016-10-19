#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

using namespace std;

/* set to 1 for debug, output of input-expected data */
#define DEBUG_STDOUT    0

/* NDRANGE */
#define WG_GLOBAL_SIZE  30
#define WG_LOCAL_SIZE   30
/*
 * Generic compute-expected function for op BROADCAST type
 * and any variable type
 */
template<class T>
static void compute_expected(T* input,
                             T* expected,
                             size_t SIMD_ID,
                             size_t SIMD_SIZE)
{
  for(uint32_t i = 0; i < SIMD_SIZE; i++)
    expected[i] = input[SIMD_ID];
}

/*
 * Generic input-expected generate function for op BROADCAST type
 * and any variable type
 */
template<class T>
static void generate_data(T* &input,
                          T* &expected,
                          size_t SIMD_ID,
                          size_t SIMD_SIZE)
{
  /* allocate input and expected arrays */
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
    for(uint32_t lid = 0; lid < SIMD_SIZE; lid++)
    {
      /* initially 0, augment after */
      input[gid + lid] = 0;

      if(sizeof(T) == 2) {
        input[gid + lid] = __float_to_half(as_uint((float)(gid + lid)));
      }
      else {
        /* check all data types, test ideal for QWORD types */
        input[gid + lid] += ((rand() % 2 - 1) * base_val);
        /* add trailing random bits, tests GENERAL cases */
        input[gid + lid] += (rand() % 112);
      }

#if DEBUG_STDOUT
      /* output generated input */
      cout << setw(4) << input[gid + lid] << ", " ;
      if((lid + 1) % 8 == 0)
        cout << endl;
#endif
    }

    /* expected values */
    compute_expected(input + gid, expected + gid, SIMD_ID, SIMD_SIZE);

#if DEBUG_STDOUT
    /* output expected input */
    cout << endl << "EXP: " << endl;
    for(uint32_t lid = 0; lid < SIMD_SIZE; lid++){
      cout << setw(4) << expected[gid + lid] << ", " ;
      if((lid + 1) % 8 == 0)
        cout << endl;
    }
    cout << endl;
#endif

  }
}

/*
 * Generic subgroup utest function for op BROADCAST type
 * and any variable type
 */
template<class T>
static void subgroup_generic(T* input,
                             T* expected)
{
  /* get simd size */
  globals[0] = WG_GLOBAL_SIZE;
  locals[0] = WG_LOCAL_SIZE;
  size_t SIMD_SIZE = 0;
  OCL_CALL(utestclGetKernelSubGroupInfoKHR,kernel,device,CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR,sizeof(size_t)*1,locals,sizeof(size_t),&SIMD_SIZE,NULL);

  cl_uint SIMD_ID = 2;
  /* input and expected data */
  generate_data(input, expected, SIMD_ID, SIMD_SIZE);

  /* prepare input for datatype */
  OCL_CREATE_BUFFER(buf[0], 0, WG_GLOBAL_SIZE * sizeof(T), NULL);
  OCL_CREATE_BUFFER(buf[1], 0, WG_GLOBAL_SIZE * sizeof(T), NULL);
  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_uint), &SIMD_ID);

  /* set input data for GPU */
  OCL_MAP_BUFFER(0);
  memcpy(buf_data[0], input,  WG_GLOBAL_SIZE* sizeof(T));
  OCL_UNMAP_BUFFER(0);

  /* run the kernel on GPU */
  OCL_NDRANGE(1);

  /* check if mismatch */
  OCL_MAP_BUFFER(1);
  uint32_t mismatches = 0;

  for (uint32_t i = 0; i < WG_GLOBAL_SIZE; i++)
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
void compiler_subgroup_broadcast_imm_int(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_broadcast",
                              "compiler_subgroup_broadcast_imm_int");
  subgroup_generic(input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_broadcast_imm_int);
void compiler_subgroup_broadcast_int(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_broadcast",
                              "compiler_subgroup_broadcast_int");
  subgroup_generic(input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_broadcast_int);
void compiler_subgroup_broadcast_long(void)
{
  if(!cl_check_subgroups())
    return;
  cl_int *input = NULL;
  cl_int *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_broadcast",
                              "compiler_subgroup_broadcast_long");
  subgroup_generic(input, expected);
}
MAKE_UTEST_FROM_FUNCTION_WITH_ISSUE(compiler_subgroup_broadcast_long);
void compiler_subgroup_broadcast_short(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_short *input = NULL;
  cl_short *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_broadcast",
                              "compiler_subgroup_broadcast_short");
  subgroup_generic(input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_broadcast_short);
void compiler_subgroup_broadcast_half(void)
{
  if(!cl_check_subgroups())
    return;
  if(!cl_check_half())
    return;
  cl_half *input = NULL;
  cl_half *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_broadcast.cl",
                           "compiler_subgroup_broadcast_half",
                           SOURCE, "-DHALF");
  subgroup_generic(input, expected);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_broadcast_half);
