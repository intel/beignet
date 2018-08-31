#include "utest_helper.hpp"
#include <string.h>

void compiler_intra_prediction(void)
{
  if (!cl_check_device_side_avc_motion_estimation()) {
    return;
  }
  if (!cl_check_reqd_subgroup())
    return;

  OCL_CREATE_KERNEL("compiler_intra_prediction");

  const size_t w = 80;
  const size_t h = 48;
  const size_t mv_w = (w + 15) / 16;
  const size_t mv_h = (h + 15) / 16;

  cl_image_format format;
  cl_image_desc desc;

  memset(&desc, 0x0, sizeof(cl_image_desc));
  memset(&format, 0x0, sizeof(cl_image_format));

  uint8_t *image_data1 = (uint8_t *)malloc(w * h); // src
  for (size_t j = 0; j < h; j++) {
    for (size_t i = 0; i < w; i++) {
      if (i >= 32 && i <= 47 && j >= 16 && j <= 31)
        image_data1[w * j + i] = 2;
      else
        image_data1[w * j + i] = 1;
    }
  }

  format.image_channel_order = CL_R;
  format.image_channel_data_type = CL_UNORM_INT8;
  desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  desc.image_width = w;
  desc.image_height = h;
  desc.image_row_pitch = 0;
  OCL_CREATE_IMAGE(buf[0], CL_MEM_COPY_HOST_PTR, &format, &desc, image_data1); // src

  OCL_CREATE_BUFFER(buf[1], 0, mv_w * mv_h * sizeof(uint8_t), NULL);
  OCL_CREATE_BUFFER(buf[2], 0, mv_w * mv_h * sizeof(uint16_t), NULL);
  OCL_CREATE_BUFFER(buf[3], 0, mv_w * mv_h * sizeof(uint8_t), NULL);
  OCL_CREATE_BUFFER(buf[4], 0, mv_w * mv_h * sizeof(uint32_t) * 16 * 8, NULL);
  OCL_CREATE_BUFFER(buf[5], 0, mv_w * mv_h * sizeof(uint32_t) * 8 * 8, NULL);

  OCL_SET_ARG(0, sizeof(cl_mem), &buf[0]);
  OCL_SET_ARG(1, sizeof(cl_mem), &buf[1]);
  OCL_SET_ARG(2, sizeof(cl_mem), &buf[2]);
  OCL_SET_ARG(3, sizeof(cl_mem), &buf[3]);
  OCL_SET_ARG(4, sizeof(cl_mem), &buf[4]);
  OCL_SET_ARG(5, sizeof(cl_mem), &buf[5]);

  globals[0] = w;
  globals[1] = h / 16;
  locals[0] = 16;
  locals[1] = 1;
  OCL_NDRANGE(2);

  OCL_MAP_BUFFER(1);
  OCL_MAP_BUFFER(2);
  OCL_MAP_BUFFER(3);
  OCL_MAP_BUFFER(4);
  OCL_MAP_BUFFER(5);
  uint8_t *modes = (uint8_t *)buf_data[1];
  uint16_t *residual = (uint16_t *)buf_data[2];
  uint8_t *shape = (uint8_t *)buf_data[3];
#define VME_DEBUG 0
#if VME_DEBUG
  uint32_t *dwo = (uint32_t *)buf_data[4];
  uint32_t *pld = (uint32_t *)buf_data[5];
  std::cout << std::endl;
  for (uint32_t j = 0; j <= mv_h - 1; ++j) {
    for (uint32_t i = 0; i <= mv_w - 1; ++i) {
      uint32_t mv_num = j * mv_w + i;
      std::cout << "******* mv num = " << mv_num << ": " << std::endl;
      std::cout << "payload register result: " << std::endl;
      for (uint32_t row_num = 0; row_num < 8; row_num++) {
        for (int32_t idx = 7; idx >= 0; idx--)
          printf("%.8x ", pld[mv_num * 64 + row_num * 8 + idx]);
        printf("\n");
      }
      std::cout << std::endl;
      std::cout << "writeback register result: " << std::endl;
      for (uint32_t row_num = 0; row_num < 4; row_num++) {
        for (int32_t wi = 7; wi >= 0; wi--)
          printf("%.8x ", dwo[mv_num * 16 * 4 + row_num * 16 + wi]);
        printf("\n");
        for (int32_t wi = 15; wi >= 8; wi--)
          printf("%.8x ", dwo[mv_num * 16 * 4 + row_num * 16 + wi]);
        printf("\n");
      }
      std::cout << std::endl;
      printf("modes: %u\n", modes[mv_num]);
      std::cout << std::endl;
      std::cout << "residual: " << residual[mv_num] << std::endl;
      std::cout << std::endl;
      printf("shape: %u\n", shape[mv_num]);
      std::cout << std::endl;
    }
  }
#endif
  OCL_ASSERT(modes[7] == 2);
  OCL_ASSERT(residual[7] == 266);
  OCL_ASSERT(shape[7] == 0);

  OCL_UNMAP_BUFFER(1);
  OCL_UNMAP_BUFFER(2);
  OCL_UNMAP_BUFFER(3);
  OCL_UNMAP_BUFFER(4);
  OCL_UNMAP_BUFFER(5);

  free(image_data1);
}

MAKE_UTEST_FROM_FUNCTION(compiler_intra_prediction);
