#include <cstdint>
#include <cstring>
#include <iostream>
#include "utest_helper.hpp"

using namespace std;

/* set to 1 for debug, output of input-expected data */
#define DEBUG_STDOUT    0

/* NDRANGE */
#define WG_GLOBAL_SIZE_X  64
#define WG_GLOBAL_SIZE_Y  1
#define WG_LOCAL_SIZE_X   64
#define WG_LOCAL_SIZE_Y   1
/*
 * Generic compute-expected function for meida block write
 */
template<class T>
static void compute_expected_media(T* input,
                             T* expected,
                             size_t VEC_SIZE)
{
  for (uint32_t h = 0; h < VEC_SIZE * WG_GLOBAL_SIZE_Y; h++) {
    for (uint32_t w = 0; w < WG_GLOBAL_SIZE_X / (VEC_SIZE * WG_GLOBAL_SIZE_Y); w++) {
      for (uint32_t v = 0; v < (VEC_SIZE * WG_LOCAL_SIZE_Y); v++) {
        expected[w + v * WG_GLOBAL_SIZE_X / (VEC_SIZE *WG_GLOBAL_SIZE_Y) + h * WG_GLOBAL_SIZE_X] = w * VEC_SIZE + h;
      }
    }
  }
}

/*
 * Generic input-expected generate function for media block write
 */
template<class T>
static void generate_data_media(T* &input,
                          T* &expected,
                          size_t VEC_SIZE)
{
  /* allocate input and expected arrays */
  input = new T[WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y * VEC_SIZE];
  expected = new T[WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y * VEC_SIZE];

  /* seed for random inputs */
  srand (time(NULL));

#if DEBUG_STDOUT
    cout << endl << "IN: " << endl;
#endif
  /* generate inputs and expected values */
  for(uint32_t gid = 0; gid < WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y * VEC_SIZE; gid++)
  {
      input[gid] = gid % WG_GLOBAL_SIZE_X;

#if DEBUG_STDOUT
      /* output generated input */
      cout << setw(4) << (uint32_t)input[gid] << ", " ;
      if((gid+ 1) % 8 == 0)
            cout << endl;
#endif
  }
  /* expected values */
  compute_expected_media(input, expected, VEC_SIZE);

#if DEBUG_STDOUT
  /* output expected input */
  cout << endl << "EXP: " << endl;
  for(uint32_t gid = 0; gid < WG_GLOBAL_SIZE_X; gid++)
  {
    cout << "(";
    for(uint32_t vsz = 0; vsz < VEC_SIZE * WG_GLOBAL_SIZE_Y; vsz++)
      cout << setw(4) << (uint32_t)expected[gid* VEC_SIZE * WG_GLOBAL_SIZE_Y + vsz] << ", " ;
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
static void subgroup_generic_media(T* input,
                             T* expected,
                             size_t VEC_SIZE,
                             const char* kernel_name)
{
  OCL_CALL(cl_kernel_init, "compiler_subgroup_image_block_write.cl", kernel_name,
           SOURCE, "-DMEDIA_BLOCK_IO");

  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  /* get simd size */
  globals[0] = WG_GLOBAL_SIZE_X;
  globals[1] = WG_GLOBAL_SIZE_Y;
  locals[0] = WG_LOCAL_SIZE_X;
  locals[1] = WG_LOCAL_SIZE_Y;
  size_t SIMD_SIZE = 0;
  OCL_CALL(utestclGetKernelSubGroupInfoKHR,kernel,device,CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR,sizeof(size_t)*1,locals,sizeof(size_t),&SIMD_SIZE,NULL);

  if(SIMD_SIZE != 16) {
    printf("skip! this case could only run under simd16 mode!\n");
    return;
  }

  size_t buf_sz = VEC_SIZE * WG_GLOBAL_SIZE_X * WG_GLOBAL_SIZE_Y;
  /* input and expected data */
  generate_data_media(input, expected, VEC_SIZE);

  /* prepare input for datatype */
  format.image_channel_order = CL_R;
  format.image_channel_data_type = CL_UNSIGNED_INT32;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = WG_GLOBAL_SIZE_X;
  desc.image_height = VEC_SIZE * WG_GLOBAL_SIZE_Y;
  desc.image_row_pitch = WG_GLOBAL_SIZE_X * sizeof(uint32_t);

  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, input);
  OCL_CREATE_BUFFER(buf[1], 0, buf_sz * sizeof(T), NULL);

  /* set input data for GPU */
  OCL_MAP_BUFFER(1);
  memcpy(buf_data[1], input,  buf_sz* sizeof(T));
  OCL_UNMAP_BUFFER(1);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);

  /* run the kernel on GPU */
  OCL_NDRANGE(2);

  /* check if mismatch */
  OCL_MAP_BUFFER_GTT(0);
  uint32_t mismatches = 0;
  size_t image_row_pitch = 0;
  OCL_CALL(clGetImageInfo, buf[0], CL_IMAGE_ROW_PITCH, sizeof(image_row_pitch), &image_row_pitch, NULL);
  image_row_pitch /= sizeof(T);
  T *out = (T *)buf_data[0];

  for (uint32_t vsz = 0; vsz < VEC_SIZE * WG_GLOBAL_SIZE_Y; vsz++)
    for (uint32_t i = 0; i < WG_GLOBAL_SIZE_X; i++)
      if (out[vsz * image_row_pitch + i] != expected[WG_GLOBAL_SIZE_X * vsz + i])
      {
        /* found mismatch, increment */
        mismatches++;

#if DEBUG_STDOUT
        /* output mismatch */
        cout << "Err at " << WG_GLOBAL_SIZE_X * vsz + i << ", " << (uint32_t)out[vsz * image_row_pitch + i]
             << " != " << (uint32_t)expected[WG_GLOBAL_SIZE_X * vsz + i] << endl;
#endif
      }

  OCL_UNMAP_BUFFER_GTT(0);

  OCL_ASSERT(mismatches == 0);
  free(input);
  free(expected);
}

#define compiler_subgroup_media_block_write_ui(name, kernel, vec_size) \
void name(void)  \
{ \
  if(!cl_check_media_block_io()) \
    return; \
  cl_uint *input = NULL; \
  cl_uint *expected = NULL; \
  subgroup_generic_media(input, expected, vec_size, #kernel); \
}  \
MAKE_UTEST_FROM_FUNCTION(name);
compiler_subgroup_media_block_write_ui(compiler_subgroup_media_block_write_ui1, compiler_subgroup_media_block_write_ui1, 1)
compiler_subgroup_media_block_write_ui(compiler_subgroup_media_block_write_ui2, compiler_subgroup_media_block_write_ui2, 2)
compiler_subgroup_media_block_write_ui(compiler_subgroup_media_block_write_ui4, compiler_subgroup_media_block_write_ui4, 4)
compiler_subgroup_media_block_write_ui(compiler_subgroup_media_block_write_ui8, compiler_subgroup_media_block_write_ui8, 8)

#define compiler_subgroup_media_block_write_us(name, kernel, vec_size) \
void name(void)  \
{ \
  if(!cl_check_media_block_io()) \
    return; \
  cl_ushort *input = NULL; \
  cl_ushort *expected = NULL; \
  subgroup_generic_media(input, expected, vec_size, #kernel); \
}  \
MAKE_UTEST_FROM_FUNCTION(name);
compiler_subgroup_media_block_write_us(compiler_subgroup_media_block_write_us1, compiler_subgroup_media_block_write_us1, 1)
compiler_subgroup_media_block_write_us(compiler_subgroup_media_block_write_us2, compiler_subgroup_media_block_write_us2, 2)
compiler_subgroup_media_block_write_us(compiler_subgroup_media_block_write_us4, compiler_subgroup_media_block_write_us4, 4)
compiler_subgroup_media_block_write_us(compiler_subgroup_media_block_write_us8, compiler_subgroup_media_block_write_us8, 8)
//the us16 case could only run under SIMD8 mode.
//compiler_subgroup_media_block_write_us(compiler_subgroup_media_block_write_us16, compiler_subgroup_media_block_write_us16, 16)

#define compiler_subgroup_media_block_write_uc(name, kernel, vec_size) \
void name(void)  \
{ \
  if(!cl_check_media_block_io()) \
    return; \
  cl_uchar *input = NULL; \
  cl_uchar *expected = NULL; \
  subgroup_generic_media(input, expected, vec_size, #kernel); \
}  \
MAKE_UTEST_FROM_FUNCTION(name);
compiler_subgroup_media_block_write_uc(compiler_subgroup_media_block_write_uc1, compiler_subgroup_media_block_write_uc1, 1)
compiler_subgroup_media_block_write_uc(compiler_subgroup_media_block_write_uc2, compiler_subgroup_media_block_write_uc2, 2)
compiler_subgroup_media_block_write_uc(compiler_subgroup_media_block_write_uc4, compiler_subgroup_media_block_write_uc4, 4)
compiler_subgroup_media_block_write_uc(compiler_subgroup_media_block_write_uc8, compiler_subgroup_media_block_write_uc8, 8)
compiler_subgroup_media_block_write_uc(compiler_subgroup_media_block_write_uc16, compiler_subgroup_media_block_write_uc16, 16)
