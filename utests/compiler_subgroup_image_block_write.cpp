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
 * Generic compute-expected function for meida block write
 */
template<class T>
static void compute_expected(T* input,
                             T* expected,
                             size_t VEC_SIZE)
{
  for(uint32_t i = 0; i < WG_GLOBAL_SIZE; i++)
    for(uint32_t j = 0; j < VEC_SIZE; j++)
      expected[WG_GLOBAL_SIZE * j + i] = input[i * VEC_SIZE + j];
}

/*
 * Generic input-expected generate function for media block write
 */
template<class T>
static void generate_data(T* &input,
                          T* &expected,
                          size_t VEC_SIZE)
{
  /* allocate input and expected arrays */
  input = new T[WG_GLOBAL_SIZE * VEC_SIZE];
  expected = new T[WG_GLOBAL_SIZE * VEC_SIZE];

  /* base value for all data types */
  T base_val = (long)7 << (sizeof(T) * 5 - 3);

  /* seed for random inputs */
  srand (time(NULL));

#if DEBUG_STDOUT
    cout << endl << "IN: " << endl;
#endif
  /* generate inputs and expected values */
  for(uint32_t gid = 0; gid < WG_GLOBAL_SIZE * VEC_SIZE; gid++)
  {
    /* initially 0, augment after */
    input[gid] = ((rand() % 2 - 1) * base_val) + (rand() % 112);
    //input[gid] = gid;

#if DEBUG_STDOUT
    /* output generated input */
    cout << setw(4) << input[gid] << ", " ;
    if((gid + 1) % 8 == 0)
          cout << endl;
#endif

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
 * Generic subgroup utest function for media block write
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
  desc.image_row_pitch = 0;

  OCL_CREATE_IMAGE(buf[0], 0, &format, &desc, NULL);
  OCL_CREATE_BUFFER(buf[1], 0, buf_sz * sizeof(T), NULL);

  /* set input data for GPU */
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[1], input,  buf_sz* sizeof(T));
  OCL_UNMAP_BUFFER(1);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  /* run the kernel on GPU */
  OCL_NDRANGE(1);

  /* check if mismatch */
  OCL_MAP_BUFFER_GTT(0);
  uint32_t mismatches = 0;
  size_t image_row_pitch = 0;
  OCL_CALL(clGetImageInfo, buf[0], CL_IMAGE_ROW_PITCH, sizeof(image_row_pitch), &image_row_pitch, NULL);
  image_row_pitch /= sizeof(T);
  T *out = (T *)buf_data[0];

  for (uint32_t vsz = 0; vsz < VEC_SIZE; vsz++)
    for (uint32_t i = 0; i < WG_GLOBAL_SIZE; i++)
      if (out[vsz * image_row_pitch + i] != expected[WG_GLOBAL_SIZE * vsz + i]) {
        /* found mismatch, increment */
        mismatches++;

#if DEBUG_STDOUT
        /* output mismatch */
        cout << "Err at " << WG_GLOBAL_SIZE * vsz + i << ", " << out[vsz * image_row_pitch + i]
             << " != " << expected[WG_GLOBAL_SIZE * vsz + i] << endl;
#endif
      }

  OCL_UNMAP_BUFFER_GTT(0);

  OCL_ASSERT(mismatches == 0);
  free(input);
  free(expected);
}

/*
 * sub_group image block write functions
 */
void compiler_subgroup_image_block_write_ui1(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_write",
                              "compiler_subgroup_image_block_write_ui1");
  subgroup_generic(input, expected, 1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_ui1);
void compiler_subgroup_image_block_write_ui2(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_write",
                              "compiler_subgroup_image_block_write_ui2");
  subgroup_generic(input, expected, 2);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_ui2);
void compiler_subgroup_image_block_write_ui4(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_write",
                              "compiler_subgroup_image_block_write_ui4");
  subgroup_generic(input, expected, 4);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_ui4);
void compiler_subgroup_image_block_write_ui8(void)
{
  if(!cl_check_subgroups())
    return;
  cl_uint *input = NULL;
  cl_uint *expected = NULL;
  OCL_CREATE_KERNEL_FROM_FILE("compiler_subgroup_image_block_write",
                              "compiler_subgroup_image_block_write_ui8");
  subgroup_generic(input, expected, 8);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_ui8);
void compiler_subgroup_image_block_write_us1(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_write.cl",
                           "compiler_subgroup_image_block_write_us1",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 1);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_us1);
void compiler_subgroup_image_block_write_us2(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_write.cl",
                           "compiler_subgroup_image_block_write_us2",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 2);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_us2);
void compiler_subgroup_image_block_write_us4(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_write.cl",
                           "compiler_subgroup_image_block_write_us4",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 4);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_us4);
void compiler_subgroup_image_block_write_us8(void)
{
  if(!cl_check_subgroups_short())
    return;
  cl_ushort *input = NULL;
  cl_ushort *expected = NULL;
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_write.cl",
                           "compiler_subgroup_image_block_write_us8",
                           SOURCE, "-DSHORT");
  subgroup_generic(input, expected, 8);
}
MAKE_UTEST_FROM_FUNCTION(compiler_subgroup_image_block_write_us8);
