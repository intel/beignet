#include "utest_helper.hpp"
#include <string.h>

typedef cl_accelerator_intel (OCLCREATEACCELERATORINTEL)(cl_context, cl_accelerator_type_intel accel_type, size_t desc_sz, const void* desc, cl_int* errcode_ret);
OCLCREATEACCELERATORINTEL * oclCreateAcceleratorIntel = NULL;
typedef cl_int (OCLRELEASEACCELERATORINTEL)(cl_accelerator_intel accel_type);
OCLRELEASEACCELERATORINTEL * oclReleaseAcceleratorIntel = NULL;

void builtin_kernel_block_motion_estimate_intel(void)
{
  if (!cl_check_motion_estimation()) {
    return;
  }
  char* built_in_kernel_names;
  size_t built_in_kernels_size;
  cl_int err = CL_SUCCESS;
  size_t ret_sz;

  OCL_CALL (clGetDeviceInfo, device, CL_DEVICE_BUILT_IN_KERNELS, 0, 0, &built_in_kernels_size);
  built_in_kernel_names = (char* )malloc(built_in_kernels_size * sizeof(char) );
  OCL_CALL(clGetDeviceInfo, device, CL_DEVICE_BUILT_IN_KERNELS, built_in_kernels_size, (void*)built_in_kernel_names, &ret_sz);
  OCL_ASSERT(ret_sz == built_in_kernels_size);

  if (strstr(built_in_kernel_names, "block_motion_estimate_intel") == NULL)
  {
        free(built_in_kernel_names);
        fprintf(stderr, "Can't find block_motion_estimate_intel built-in kernel");
        OCL_ASSERT(0);
  }

  cl_program built_in_prog = clCreateProgramWithBuiltInKernels(ctx, 1, &device, built_in_kernel_names, &err);
  OCL_ASSERT(built_in_prog != NULL);
  kernel = clCreateKernel(built_in_prog, "block_motion_estimate_intel",  &err);
  OCL_ASSERT(kernel != NULL);

  cl_motion_estimation_desc_intel vmedesc = {CL_ME_MB_TYPE_16x16_INTEL,       //0x0
                                          CL_ME_SUBPIXEL_MODE_INTEGER_INTEL,  //0x0
                                          CL_ME_SAD_ADJUST_MODE_NONE_INTEL,   //0x0
                                          CL_ME_SEARCH_PATH_RADIUS_16_12_INTEL //0x5
                                          };
#ifdef CL_VERSION_1_2
  oclCreateAcceleratorIntel = (OCLCREATEACCELERATORINTEL*)clGetExtensionFunctionAddressForPlatform(platform, "clCreateAcceleratorINTEL");
#else
  oclCreateAcceleratorIntel  = (OCLCREATEACCELERATORINTEL*)clGetExtensionFunctionAddress("clCreateAcceleratorINTEL");
#endif
  if(!oclCreateAcceleratorIntel){
    fprintf(stderr, "Failed to get extension clCreateImageFromLibvaIntel\n");
    OCL_ASSERT(0);
  }
  cl_accelerator_intel accel = oclCreateAcceleratorIntel(ctx, CL_ACCELERATOR_TYPE_MOTION_ESTIMATION_INTEL,sizeof(cl_motion_estimation_desc_intel), &vmedesc, &err);
  OCL_ASSERT(accel != NULL);

  const size_t w = 71; //80
  const size_t h = 41; //48

  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  uint8_t* image_data1 = (uint8_t *)malloc(w * h);    //src
  uint8_t* image_data2 = (uint8_t *)malloc(w * h);    //ref
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      if (i >= 32 && i <= 47 && j >= 16 && j <= 31)
        image_data2[w * j + i] = image_data1[w * j + i] = 100;
      else
        image_data2[w * j + i] = image_data1[w * j + i] = 17;
    }
  }

  format.image_channel_order = CL_R;
  format.image_channel_data_type = CL_UNORM_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, image_data1);        //src
  OCL_CREATE_IMAGE(buf[1], CL_MEM_COPY_HOST_PTR, &format, &desc, image_data2);        //ref

  const size_t mv_w = (w + 15) / 16;
  const size_t mv_h = (h + 15) / 16;
  OCL_CREATE_BUFFER(buf[2], 0, mv_w * mv_h * sizeof(short) * 2, NULL);

  OCL_SET_ARG(0, sizeof(cl_accelerator_intel), &accel);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(3, sizeof(cl_mem), NULL);
  OCL_SET_ARG(4, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(5, sizeof(cl_mem), NULL);

  globals[0] = w;
  globals[1] = h;
  OCL_CALL(clEnqueueNDRangeKernel, queue, kernel, 2, NULL, globals, NULL, 0, NULL, NULL);

  OCL_MAP_BUFFER(2);
  short expected[] = {-64, -48,   //S13.2 fixed point value
                    -64, -48,
                    -64, -48,
                    -64, -48,
                    -64, -48,
                    -64, -48,
                    -64, -48,
                    0, 0,
                    0, -48,
                    -64, -48,
                    -64, -48,
                    -64, -48,
                    -64, -48,
                    0, -48,
                    -64, -48};
  short* res = (short*)buf_data[2];
  for (uint32_t j = 0; j < mv_h - 1; ++j) {
    for (uint32_t i = 0; i < mv_w - 1; ++i) {
        uint32_t index = j * mv_w * 2 + i * 2;
        OCL_ASSERT(res[index + 0] == expected[index + 0]);
        OCL_ASSERT(res[index + 1] == expected[index + 1]);
    }
  }
  OCL_UNMAP_BUFFER(2);

#ifdef CL_VERSION_1_2
  oclReleaseAcceleratorIntel = (OCLRELEASEACCELERATORINTEL*)clGetExtensionFunctionAddressForPlatform(platform, "clReleaseAcceleratorINTEL");
#else
  oclReleaseAcceleratorIntel  = (OCLRELEASEACCELERATORINTEL*)clGetExtensionFunctionAddress("clReleaseAcceleratorINTEL");
#endif
  if(!oclReleaseAcceleratorIntel){
    fprintf(stderr, "Failed to get extension clCreateImageFromLibvaIntel\n");
    OCL_ASSERT(0);
  }
  oclReleaseAcceleratorIntel(accel);
  clReleaseProgram(built_in_prog);
  free(built_in_kernel_names);
  free(image_data1);
  free(image_data2);
}

MAKE_UTEST_FROM_FUNCTION(builtin_kernel_block_motion_estimate_intel);
