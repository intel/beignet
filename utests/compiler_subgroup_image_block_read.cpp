#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

using namespace std;

/* set to 1 for debug, output of input-expected data */
#define DEBUG_STDOUT    0

/* NDRANGE */
#define WG_GLOBAL_SIZE  32
#define WG_LOCAL_SIZE   32
/*
 * Generic compute-expected function for meida block read
 */
template<class T>
static void compute_expected(T* input,
                             T* expected,
                             size_t VEC_SIZE)
{
  for(uint32_t i = 0; i < WG_GLOBAL_SIZE; i++)
    for(uint32_t j = 0; j < VEC_SIZE; j++)
      expected[i * VEC_SIZE + j] = input[WG_GLOBAL_SIZE * 4 / sizeof(T) * j + i];
}

/*
 * Generic input-expected generate function for media block read
 */
template<class T>
static void generate_data(T* &input,
                          T* &expected,
                          size_t VEC_SIZE)
{
  /* allocate input and expected arrays */
  int* input_ui = new int[WG_GLOBAL_SIZE * VEC_SIZE];
  input = (T*)input_ui;
  expected = new T[WG_GLOBAL_SIZE * VEC_SIZE];

  /* base value for all data types */
  T base_val = (int)7 << (sizeof(T) * 5 - 3);

  /* seed for random inputs */
  srand (time(NULL));

#if DEBUG_STDOUT
    cout << endl << "IN: " << endl;
#endif
  uint32_t rpitch = sizeof(uint32_t) * WG_GLOBAL_SIZE / sizeof(T);
  /* generate inputs and expected values */
  for(uint32_t h = 0; h < VEC_SIZE; ++h) {
    for(uint32_t w = 0; w < WG_GLOBAL_SIZE; ++w)
    {
      /* initially 0, augment after */
      input[w + h * rpitch] = ((rand() % 2 - 1) * base_val) + (rand() % 112);
      //input[w + h * rpitch] = w + h * WG_GLOBAL_SIZE;

#if DEBUG_STDOUT
      /* output generated input */
      cout << setw(4) << input[w + h * rpitch] << ", " ;
      if((w+ 1) % 8 == 0)
            cout << endl;
#endif
    }
  }
  /* expected values */
  compute_expected(input, expected, VEC_SIZE);

#if DEBUG_STDOUT
  /* output expected input */
  cout << endl << "EXP: " << endl;
  for(uint32_t gid = 0; gid < WG_GLOBAL_SIZE; gid++)
  {
    cout << "(";
    for(uint32_t vsz = 0; vsz < VEC_SIZE; vsz++)
      cout << setw(4) << expected[gid* VEC_SIZE + vsz] << ", " ;
    cout << ")";
    if((gid + 1) % 8 == 0)
        cout << endl;
    cout << endl;
  }
#endif
}

/*
 * Generic subgroup utest function for media block read
 */
template<class T>
static void subgroup_generic(T* input,
                             T* expected,
                             size_t VEC_SIZE)
{
  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  /* get simd size */
  globals[0] = WG_GLOBAL_SIZE;
  locals[0] = WG_LOCAL_SIZE;
  size_t SIMD_SIZE = 0;
  OCL_CALL(utestclGetKernelSubGroupInfoKHR,kernel,device,CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR,sizeof(size_t)*1,locals,sizeof(size_t),&SIMD_SIZE,NULL);

  size_t buf_sz = VEC_SIZE * WG_GLOBAL_SIZE;
  /* input and expected data */
  generate_data(input, expected, VEC_SIZE);

  /* prepare input for datatype */
  format.image_channel_order = CL_R;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = WG_GLOBAL_SIZE;
  desc.image_height = VEC_SIZE;
  desc.image_row_pitch = WG_GLOBAL_SIZE * sizeof(uint32_t);

  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, input);
  OCL_CREATE_BUFFER(buf[1], 0, buf_sz * sizeof(T), NULL);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  /* run the kernel on GPU */
  OCL_NDRANGE(1);

  /* check if mismatch */
  OCL_MAP_BUFFER(1);
  uint32_t mismatches = 0;

  for (uint32_t i = 0; i < buf_sz; i++)
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
  free(input);
  free(expected);
}

/*
 * sub_group image block read functions
 */
void compiler_subgroup_image_block_read_ui1(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_read",
                              "compiler_subgroup_image_block_read_ui1");
  subgroup_generic(input, expected, 1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_ui1);
void compiler_subgroup_image_block_read_ui2(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_read",
                              "compiler_subgroup_image_block_read_ui2");
  subgroup_generic(input, expected, 2);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_ui2);
void compiler_subgroup_image_block_read_ui4(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_read",
                              "compiler_subgroup_image_block_read_ui4");
  subgroup_generic(input, expected, 4);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_ui4);
void compiler_subgroup_image_block_read_ui8(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_read",
                              "compiler_subgroup_image_block_read_ui8");
  subgroup_generic(input, expected, 8);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_ui8);
void compiler_subgroup_image_block_read_us1(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_read.cl",
                           "compiler_subgroup_image_block_read_us1",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_us1);
void compiler_subgroup_image_block_read_us2(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_read.cl",
                           "compiler_subgroup_image_block_read_us2",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 2);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_us2);
void compiler_subgroup_image_block_read_us4(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_read.cl",
                           "compiler_subgroup_image_block_read_us4",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 4);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_us4);
void compiler_subgroup_image_block_read_us8(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_read.cl",
                           "compiler_subgroup_image_block_read_us8",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 8);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_read_us8);
