/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "cl_gen.h"
#include "gen_device_pci_id.h"
#include <sys/sysinfo.h>

extern cl_int cl_compiler_unload_gen(cl_device_id device);

static _cl_device_api __gen_device_api = {
  .compiler_unload = cl_compiler_unload_gen,
  .context_create = cl_context_create_gen,
  .context_delete = cl_context_delete_gen,
  .event_create = cl_event_create_gen,
  .event_delete = cl_event_delete_gen,
  .event_profiling = cl_event_update_timestamp_gen,
  .command_queue_create = cl_command_queue_create_gen,
  .command_queue_delete = cl_command_queue_delete_gen,
  .sampler_create = cl_sampler_create_gen,
  .sampler_delete = cl_sampler_delete_gen,
  .program_create = cl_program_create_gen,
  .program_load_binary = cl_program_load_binary_gen,
  .program_delete = cl_program_delete_gen,
  .program_get_info = cl_program_get_info_gen,
  .kernel_delete = cl_kernel_delete_gen,
  .kernel_create = cl_kernel_create_gen,
  .kernel_get_info = cl_kernel_get_info_gen,
  .nd_range_kernel = cl_enqueue_handle_nd_range_gen,
  .native_kernel = cl_enqueue_native_kernel,
  .svm_create = cl_svm_create_gen,
  .svm_delete = cl_svm_delete_gen,
  .svm_map = cl_enqueue_svm_map_gen,
  .svm_unmap = cl_enqueue_svm_unmap_gen,
  .svm_copy = cl_enqueue_svm_copy_gen,
  .svm_fill = cl_enqueue_svm_fill_gen,
  .image_format_support = cl_image_format_support_gen,
  .mem_allocate = cl_mem_allocate_gen,
  .mem_deallocate = cl_mem_deallocate_gen,
  .mem_map = cl_enqueue_map_mem_gen,
  .mem_unmap = cl_enqueue_unmap_mem_gen,
  .buffer_read = cl_enqueue_read_buffer_gen,
  .buffer_write = cl_enqueue_write_buffer_gen,
  .buffer_read_rect = cl_enqueue_read_buffer_gen,
  .buffer_write_rect = cl_enqueue_write_buffer_rect_gen,
  .image_read = cl_enqueue_read_image_gen,
  .image_write = cl_enqueue_write_image_gen,
  .buffer_copy = cl_mem_enqueue_copy_buffer_gen,
  .buffer_fill = cl_mem_enqueue_fill_buffer_gen,
  .buffer_copy_rect = cl_mem_enqueue_copy_buffer_rect_gen,
  .image_fill = cl_enqueue_image_fill_gen,
  .image_copy = cl_enqueue_image_copy_gen,
  .copy_image_to_buffer = cl_enqueue_copy_image_to_buffer_gen,
  .copy_buffer_to_image = cl_enqueue_copy_buffer_to_image_gen,
};

/* HW parameters */
#define BTI_MAX_READ_IMAGE_ARGS 128
#define BTI_MAX_WRITE_IMAGE_ARGS 8

static struct _cl_device_id intel_ivb_gt2_device = {
  .max_compute_unit = 16,
  .max_thread_per_unit = 8,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen7_device.h"
};

static struct _cl_device_id intel_ivb_gt1_device = {
  .max_compute_unit = 6,
  .max_thread_per_unit = 6,
  .sub_slice_count = 1,
  .max_work_item_sizes = {256, 256, 256},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen7_device.h"
};

static struct _cl_device_id intel_baytrail_t_device = {
  .max_compute_unit = 4,
  .max_thread_per_unit = 8,
  .sub_slice_count = 1,
  .max_work_item_sizes = {256, 256, 256},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen7_device.h"
};

/* XXX we clone IVB for HSW now */
static struct _cl_device_id intel_hsw_gt1_device = {
  .max_compute_unit = 10,
  .max_thread_per_unit = 7,
  .sub_slice_count = 1,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_hsw_gt2_device = {
  .max_compute_unit = 20,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_hsw_gt3_device = {
  .max_compute_unit = 40,
  .max_thread_per_unit = 7,
  .sub_slice_count = 4,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

/* XXX we clone IVB for HSW now */
static struct _cl_device_id intel_brw_gt1_device = {
  .max_compute_unit = 12,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen8_device.h"
};

static struct _cl_device_id intel_brw_gt2_device = {
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen8_device.h"
};

static struct _cl_device_id intel_brw_gt3_device = {
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen8_device.h"
};

//Cherryview has the same pciid, must get the max_compute_unit and max_thread_per_unit from drm
static struct _cl_device_id intel_chv_device = {
  .max_compute_unit = 8,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

/* XXX we clone brw now */
static struct _cl_device_id intel_skl_gt1_device = {
  .max_compute_unit = 6,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_skl_gt2_device = {
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_skl_gt3_device = {
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_skl_gt4_device = {
  .max_compute_unit = 72,
  .max_thread_per_unit = 7,
  .sub_slice_count = 9,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_bxt18eu_device = {
  .max_compute_unit = 18,
  .max_thread_per_unit = 6,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_bxt12eu_device = {
  .max_compute_unit = 12,
  .max_thread_per_unit = 6,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt1_device = {
  .max_compute_unit = 12,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt15_device = {
  .max_compute_unit = 18,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt2_device = {
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt3_device = {
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt4_device = {
  .max_compute_unit = 72,
  .max_thread_per_unit = 7,
  .sub_slice_count = 9,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static cl_device_id
get_gt_device(cl_platform_id platform)
{
  cl_device_id ret = NULL;
  const int device_id = intel_get_device_id();
  cl_device_id device = NULL;

  if (device != NULL)
    return device;

#define DECL_INFO_STRING(BREAK, STRUCT, FIELD, STRING) \
  STRUCT.FIELD = STRING;                               \
  STRUCT.JOIN(FIELD, _sz) = sizeof(STRING);            \
  device = &STRUCT;                                    \
  goto BREAK;

  switch (device_id) {
  case PCI_CHIP_HASWELL_D1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell GT1 Desktop");
  case PCI_CHIP_HASWELL_D2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell GT2 Desktop");
  case PCI_CHIP_HASWELL_D3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell GT3 Desktop");
  case PCI_CHIP_HASWELL_S1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell GT1 Server");
  case PCI_CHIP_HASWELL_S2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell GT2 Server");
  case PCI_CHIP_HASWELL_S3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell GT3 Server");
  case PCI_CHIP_HASWELL_M1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell GT1 Mobile");
  case PCI_CHIP_HASWELL_M2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell GT2 Mobile");
  case PCI_CHIP_HASWELL_M3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell GT3 Mobile");
  case PCI_CHIP_HASWELL_B1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell GT1 reserved");
  case PCI_CHIP_HASWELL_B2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell GT2 reserved");
  case PCI_CHIP_HASWELL_B3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell GT3 reserved");
  case PCI_CHIP_HASWELL_E1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell GT1 reserved");
  case PCI_CHIP_HASWELL_E2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell GT2 reserved");
  case PCI_CHIP_HASWELL_E3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell GT3 reserved");
  case PCI_CHIP_HASWELL_SDV_D1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT1 Desktop");
  case PCI_CHIP_HASWELL_SDV_D2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT2 Desktop");
  case PCI_CHIP_HASWELL_SDV_D3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT3 Desktop");
  case PCI_CHIP_HASWELL_SDV_S1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT1 Server");
  case PCI_CHIP_HASWELL_SDV_S2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT2 Server");
  case PCI_CHIP_HASWELL_SDV_S3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT3 Server");
  case PCI_CHIP_HASWELL_SDV_M1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT1 Mobile");
  case PCI_CHIP_HASWELL_SDV_M2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT2 Mobile");
  case PCI_CHIP_HASWELL_SDV_M3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT3 Mobile");
  case PCI_CHIP_HASWELL_SDV_B1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT1 reserved");
  case PCI_CHIP_HASWELL_SDV_B2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT2 reserved");
  case PCI_CHIP_HASWELL_SDV_B3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT3 reserved");
  case PCI_CHIP_HASWELL_SDV_E1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT1 reserved");
  case PCI_CHIP_HASWELL_SDV_E2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT2 reserved");
  case PCI_CHIP_HASWELL_SDV_E3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell"
                                                            " Software Development Vehicle device GT3 reserved");
  case PCI_CHIP_HASWELL_ULT_D1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT1 Desktop");
  case PCI_CHIP_HASWELL_ULT_D2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT2 Desktop");
  case PCI_CHIP_HASWELL_ULT_D3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT3 Desktop");
  case PCI_CHIP_HASWELL_ULT_S1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT1 Server");
  case PCI_CHIP_HASWELL_ULT_S2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT2 Server");
  case PCI_CHIP_HASWELL_ULT_S3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT3 Server");
  case PCI_CHIP_HASWELL_ULT_M1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT1 Mobile");
  case PCI_CHIP_HASWELL_ULT_M2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT2 Mobile");
  case PCI_CHIP_HASWELL_ULT_M3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT3 Mobile");
  case PCI_CHIP_HASWELL_ULT_B1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT1 reserved");
  case PCI_CHIP_HASWELL_ULT_B2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT2 reserved");
  case PCI_CHIP_HASWELL_ULT_B3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT3 reserved");
  case PCI_CHIP_HASWELL_ULT_E1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT1 reserved");
  case PCI_CHIP_HASWELL_ULT_E2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT2 reserved");
  case PCI_CHIP_HASWELL_ULT_E3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell Ultrabook GT3 reserved");

  /* CRW */
  case PCI_CHIP_HASWELL_CRW_D1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell CRW GT1 Desktop");
  case PCI_CHIP_HASWELL_CRW_D2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell CRW GT2 Desktop");
  case PCI_CHIP_HASWELL_CRW_D3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell CRW GT3 Desktop");
  case PCI_CHIP_HASWELL_CRW_S1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell CRW GT1 Server");
  case PCI_CHIP_HASWELL_CRW_S2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell CRW GT2 Server");
  case PCI_CHIP_HASWELL_CRW_S3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell CRW GT3 Server");
  case PCI_CHIP_HASWELL_CRW_M1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell CRW GT1 Mobile");
  case PCI_CHIP_HASWELL_CRW_M2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell CRW GT2 Mobile");
  case PCI_CHIP_HASWELL_CRW_M3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell CRW GT3 Mobile");
  case PCI_CHIP_HASWELL_CRW_B1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell CRW GT1 reserved");
  case PCI_CHIP_HASWELL_CRW_B2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell CRW GT2 reserved");
  case PCI_CHIP_HASWELL_CRW_B3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell CRW GT3 reserved");
  case PCI_CHIP_HASWELL_CRW_E1:
    DECL_INFO_STRING(has_break, intel_hsw_gt1_device, name, "Intel(R) HD Graphics Haswell CRW GT1 reserved");
  case PCI_CHIP_HASWELL_CRW_E2:
    DECL_INFO_STRING(has_break, intel_hsw_gt2_device, name, "Intel(R) HD Graphics Haswell CRW GT2 reserved");
  case PCI_CHIP_HASWELL_CRW_E3:
    DECL_INFO_STRING(has_break, intel_hsw_gt3_device, name, "Intel(R) HD Graphics Haswell CRW GT3 reserved");
  has_break:
    device->device_id = device_id;
    device->platform = platform;
    ret = device;
    cl_intel_platform_get_default_extension(ret);
    break;

  case PCI_CHIP_IVYBRIDGE_GT1:
    DECL_INFO_STRING(ivb_gt1_break, intel_ivb_gt1_device, name, "Intel(R) HD Graphics IvyBridge GT1");
  case PCI_CHIP_IVYBRIDGE_M_GT1:
    DECL_INFO_STRING(ivb_gt1_break, intel_ivb_gt1_device, name, "Intel(R) HD Graphics IvyBridge M GT1");
  case PCI_CHIP_IVYBRIDGE_S_GT1:
    DECL_INFO_STRING(ivb_gt1_break, intel_ivb_gt1_device, name, "Intel(R) HD Graphics IvyBridge S GT1");
  ivb_gt1_break:
    intel_ivb_gt1_device.device_id = device_id;
    intel_ivb_gt1_device.platform = platform;
    ret = &intel_ivb_gt1_device;
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_intel_motion_estimation_ext_id);
    break;

  case PCI_CHIP_IVYBRIDGE_GT2:
    DECL_INFO_STRING(ivb_gt2_break, intel_ivb_gt2_device, name, "Intel(R) HD Graphics IvyBridge GT2");
  case PCI_CHIP_IVYBRIDGE_M_GT2:
    DECL_INFO_STRING(ivb_gt2_break, intel_ivb_gt2_device, name, "Intel(R) HD Graphics IvyBridge M GT2");
  case PCI_CHIP_IVYBRIDGE_S_GT2:
    DECL_INFO_STRING(ivb_gt2_break, intel_ivb_gt2_device, name, "Intel(R) HD Graphics IvyBridge S GT2");
  ivb_gt2_break:
    intel_ivb_gt2_device.device_id = device_id;
    intel_ivb_gt2_device.platform = platform;
    ret = &intel_ivb_gt2_device;
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_intel_motion_estimation_ext_id);
    break;

  case PCI_CHIP_BAYTRAIL_T:
    DECL_INFO_STRING(baytrail_t_device_break, intel_baytrail_t_device, name, "Intel(R) HD Graphics Bay Trail-T");
  baytrail_t_device_break:
    intel_baytrail_t_device.device_id = device_id;
    intel_baytrail_t_device.platform = platform;
    ret = &intel_baytrail_t_device;
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_intel_motion_estimation_ext_id);
    break;

  case PCI_CHIP_BROADWLL_M_GT1:
    DECL_INFO_STRING(brw_gt1_break, intel_brw_gt1_device, name, "Intel(R) HD Graphics BroadWell Mobile GT1");
  case PCI_CHIP_BROADWLL_D_GT1:
    DECL_INFO_STRING(brw_gt1_break, intel_brw_gt1_device, name, "Intel(R) HD Graphics BroadWell U-Processor GT1");
  case PCI_CHIP_BROADWLL_S_GT1:
    DECL_INFO_STRING(brw_gt1_break, intel_brw_gt1_device, name, "Intel(R) HD Graphics BroadWell Server GT1");
  case PCI_CHIP_BROADWLL_W_GT1:
    DECL_INFO_STRING(brw_gt1_break, intel_brw_gt1_device, name, "Intel(R) HD Graphics BroadWell Workstation GT1");
  case PCI_CHIP_BROADWLL_U_GT1:
    DECL_INFO_STRING(brw_gt1_break, intel_brw_gt1_device, name, "Intel(R) HD Graphics BroadWell ULX GT1");
  brw_gt1_break:
    /* For Gen8 and later, half float is suppported and we will enable cl_khr_fp16. */
    intel_brw_gt1_device.device_id = device_id;
    intel_brw_gt1_device.platform = platform;
    ret = &intel_brw_gt1_device;
    cl_intel_platform_get_default_extension(ret);
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_BROADWLL_M_GT2:
    DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics 5600 BroadWell Mobile GT2");
  case PCI_CHIP_BROADWLL_D_GT2:
    DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics 5500 BroadWell U-Processor GT2");
  case PCI_CHIP_BROADWLL_S_GT2:
    DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell Server GT2");
  case PCI_CHIP_BROADWLL_W_GT2:
    DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell Workstation GT2");
  case PCI_CHIP_BROADWLL_U_GT2:
    DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics 5300 BroadWell ULX GT2");
  brw_gt2_break:
    intel_brw_gt2_device.device_id = device_id;
    intel_brw_gt2_device.platform = platform;
    ret = &intel_brw_gt2_device;
    cl_intel_platform_get_default_extension(ret);
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_BROADWLL_M_GT3:
    DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) Iris Pro Graphics 6200 BroadWell Mobile GT3");
  case PCI_CHIP_BROADWLL_D_GT3:
    DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics 6000 BroadWell U-Processor GT3");
  case PCI_CHIP_BROADWLL_UI_GT3:
    DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) Iris Graphics 6100 BroadWell U-Processor GT3");
  case PCI_CHIP_BROADWLL_S_GT3:
    DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) Iris Pro Graphics P6300 BroadWell Server GT3");
  case PCI_CHIP_BROADWLL_W_GT3:
    DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell Workstation GT3");
  case PCI_CHIP_BROADWLL_U_GT3:
    DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell ULX GT3");
  brw_gt3_break:
    intel_brw_gt3_device.device_id = device_id;
    intel_brw_gt3_device.platform = platform;
    ret = &intel_brw_gt3_device;
    cl_intel_platform_get_default_extension(ret);
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_CHV_0:
  case PCI_CHIP_CHV_1:
  case PCI_CHIP_CHV_2:
  case PCI_CHIP_CHV_3:
    DECL_INFO_STRING(chv_break, intel_chv_device, name, "Intel(R) HD Graphics Cherryview");
  chv_break:
    intel_chv_device.device_id = device_id;
    intel_chv_device.platform = platform;
    ret = &intel_chv_device;
    cl_intel_platform_get_default_extension(ret);
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_SKYLAKE_ULT_GT1:
    DECL_INFO_STRING(skl_gt1_break, intel_skl_gt1_device, name, "Intel(R) HD Graphics Skylake ULT GT1");
  case PCI_CHIP_SKYLAKE_ULX_GT1:
    DECL_INFO_STRING(skl_gt1_break, intel_skl_gt1_device, name, "Intel(R) HD Graphics Skylake ULX GT1");
  case PCI_CHIP_SKYLAKE_DT_GT1:
    DECL_INFO_STRING(skl_gt1_break, intel_skl_gt1_device, name, "Intel(R) HD Graphics Skylake Desktop GT1");
  case PCI_CHIP_SKYLAKE_HALO_GT1:
    DECL_INFO_STRING(skl_gt1_break, intel_skl_gt1_device, name, "Intel(R) HD Graphics Skylake Halo GT1");
  case PCI_CHIP_SKYLAKE_SRV_GT1:
    DECL_INFO_STRING(skl_gt1_break, intel_skl_gt1_device, name, "Intel(R) HD Graphics Skylake Server GT1");
  skl_gt1_break:
    intel_skl_gt1_device.device_id = device_id;
    intel_skl_gt1_device.platform = platform;
    ret = &intel_skl_gt1_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_SKYLAKE_ULT_GT2:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake ULT GT2");
  case PCI_CHIP_SKYLAKE_ULT_GT2F:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake ULT GT2F");
  case PCI_CHIP_SKYLAKE_ULX_GT2:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake ULX GT2");
  case PCI_CHIP_SKYLAKE_DT_GT2:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake Desktop GT2");
  case PCI_CHIP_SKYLAKE_HALO_GT2:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake Halo GT2");
  case PCI_CHIP_SKYLAKE_SRV_GT2:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake Server GT2");
  case PCI_CHIP_SKYLAKE_WKS_GT2:
    DECL_INFO_STRING(skl_gt2_break, intel_skl_gt2_device, name, "Intel(R) HD Graphics Skylake Workstation GT2");
  skl_gt2_break:
    intel_skl_gt2_device.device_id = device_id;
    intel_skl_gt2_device.platform = platform;
    ret = &intel_skl_gt2_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_SKYLAKE_ULT_GT3:
    DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake ULT GT3");
  case PCI_CHIP_SKYLAKE_HALO_GT3:
    DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Halo GT3");
  case PCI_CHIP_SKYLAKE_SRV_GT3:
    DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Server GT3");
  case PCI_CHIP_SKYLAKE_MEDIA_SRV_GT3:
    DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Media Server GT3");
  skl_gt3_break:
    intel_skl_gt3_device.device_id = device_id;
    intel_skl_gt3_device.platform = platform;
    ret = &intel_skl_gt3_device;
    cl_intel_platform_get_default_extension(ret);
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_SKYLAKE_DT_GT4:
    DECL_INFO_STRING(skl_gt4_break, intel_skl_gt4_device, name, "Intel(R) HD Graphics Skylake Desktop GT4");
  case PCI_CHIP_SKYLAKE_HALO_GT4:
    DECL_INFO_STRING(skl_gt4_break, intel_skl_gt4_device, name, "Intel(R) HD Graphics Skylake Halo GT4");
  case PCI_CHIP_SKYLAKE_SRV_GT4:
    DECL_INFO_STRING(skl_gt4_break, intel_skl_gt4_device, name, "Intel(R) HD Graphics Skylake Server GT4");
  case PCI_CHIP_SKYLAKE_WKS_GT4:
    DECL_INFO_STRING(skl_gt4_break, intel_skl_gt4_device, name, "Intel(R) HD Graphics Skylake Workstation GT4");
  skl_gt4_break:
    intel_skl_gt4_device.device_id = device_id;
    intel_skl_gt4_device.platform = platform;
    ret = &intel_skl_gt4_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_BROXTON_0:
    DECL_INFO_STRING(bxt18eu_break, intel_bxt18eu_device, name, "Intel(R) HD Graphics Broxton 0");
  case PCI_CHIP_BROXTON_2:
    DECL_INFO_STRING(bxt18eu_break, intel_bxt18eu_device, name, "Intel(R) HD Graphics Broxton 2");
  bxt18eu_break:
    intel_bxt18eu_device.device_id = device_id;
    intel_bxt18eu_device.platform = platform;
    ret = &intel_bxt18eu_device;
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_BROXTON_1:
    DECL_INFO_STRING(bxt12eu_break, intel_bxt12eu_device, name, "Intel(R) HD Graphics Broxton 1");
  case PCI_CHIP_BROXTON_3:
    DECL_INFO_STRING(bxt12eu_break, intel_bxt12eu_device, name, "Intel(R) HD Graphics Broxton 3");
  bxt12eu_break:
    intel_bxt12eu_device.device_id = device_id;
    intel_bxt12eu_device.platform = platform;
    ret = &intel_bxt12eu_device;
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_KABYLAKE_ULT_GT1:
    DECL_INFO_STRING(kbl_gt1_break, intel_kbl_gt1_device, name, "Intel(R) HD Graphics Kabylake ULT GT1");
  case PCI_CHIP_KABYLAKE_DT_GT1:
    DECL_INFO_STRING(kbl_gt1_break, intel_kbl_gt1_device, name, "Intel(R) HD Graphics Kabylake Desktop GT1");
  case PCI_CHIP_KABYLAKE_HALO_GT1:
    DECL_INFO_STRING(kbl_gt1_break, intel_kbl_gt1_device, name, "Intel(R) HD Graphics Kabylake Halo GT1");
  case PCI_CHIP_KABYLAKE_ULX_GT1:
    DECL_INFO_STRING(kbl_gt1_break, intel_kbl_gt1_device, name, "Intel(R) HD Graphics Kabylake ULX GT1");
  case PCI_CHIP_KABYLAKE_SRV_GT1:
    DECL_INFO_STRING(kbl_gt1_break, intel_kbl_gt1_device, name, "Intel(R) HD Graphics Kabylake Server GT1");
  kbl_gt1_break:
    intel_kbl_gt1_device.device_id = device_id;
    intel_kbl_gt1_device.platform = platform;
    ret = &intel_kbl_gt1_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_KABYLAKE_ULT_GT15:
    DECL_INFO_STRING(kbl_gt15_break, intel_kbl_gt15_device, name, "Intel(R) HD Graphics Kabylake ULT GT1.5");
  case PCI_CHIP_KABYLAKE_DT_GT15:
    DECL_INFO_STRING(kbl_gt15_break, intel_kbl_gt15_device, name, "Intel(R) HD Graphics Kabylake Desktop GT1.5");
  case PCI_CHIP_KABYLAKE_HALO_GT15:
    DECL_INFO_STRING(kbl_gt15_break, intel_kbl_gt15_device, name, "Intel(R) HD Graphics Kabylake Halo GT1.5");
  case PCI_CHIP_KABYLAKE_ULX_GT15:
    DECL_INFO_STRING(kbl_gt15_break, intel_kbl_gt15_device, name, "Intel(R) HD Graphics Kabylake ULX GT1.5");
  kbl_gt15_break:
    intel_kbl_gt15_device.device_id = device_id;
    intel_kbl_gt15_device.platform = platform;
    ret = &intel_kbl_gt15_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_KABYLAKE_ULT_GT2:
  case PCI_CHIP_KABYLAKE_ULT_GT2_1:
    DECL_INFO_STRING(kbl_gt2_break, intel_kbl_gt2_device, name, "Intel(R) HD Graphics Kabylake ULT GT2");
  case PCI_CHIP_KABYLAKE_DT_GT2:
    DECL_INFO_STRING(kbl_gt2_break, intel_kbl_gt2_device, name, "Intel(R) HD Graphics Kabylake Desktop GT2");
  case PCI_CHIP_KABYLAKE_HALO_GT2:
    DECL_INFO_STRING(kbl_gt2_break, intel_kbl_gt2_device, name, "Intel(R) HD Graphics Kabylake Halo GT2");
  case PCI_CHIP_KABYLAKE_ULX_GT2:
    DECL_INFO_STRING(kbl_gt2_break, intel_kbl_gt2_device, name, "Intel(R) HD Graphics Kabylake ULX GT2");
  case PCI_CHIP_KABYLAKE_SRV_GT2:
    DECL_INFO_STRING(kbl_gt2_break, intel_kbl_gt2_device, name, "Intel(R) HD Graphics Kabylake Server GT2");
  case PCI_CHIP_KABYLAKE_WKS_GT2:
    DECL_INFO_STRING(kbl_gt2_break, intel_kbl_gt2_device, name, "Intel(R) HD Graphics Kabylake Workstation GT2");
  kbl_gt2_break:
    intel_kbl_gt2_device.device_id = device_id;
    intel_kbl_gt2_device.platform = platform;
    ret = &intel_kbl_gt2_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_KABYLAKE_ULT_GT3:
  case PCI_CHIP_KABYLAKE_ULT_GT3_1:
  case PCI_CHIP_KABYLAKE_ULT_GT3_2:
    DECL_INFO_STRING(kbl_gt3_break, intel_kbl_gt3_device, name, "Intel(R) HD Graphics Kabylake ULT GT3");
  kbl_gt3_break:
    intel_kbl_gt3_device.device_id = device_id;
    intel_kbl_gt3_device.platform = platform;
    ret = &intel_kbl_gt3_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_KABYLAKE_HALO_GT4:
    DECL_INFO_STRING(kbl_gt4_break, intel_kbl_gt4_device, name, "Intel(R) HD Graphics Kabylake ULT GT4");
  kbl_gt4_break:
    intel_kbl_gt4_device.device_id = device_id;
    intel_kbl_gt4_device.platform = platform;
    ret = &intel_kbl_gt4_device;
#ifdef ENABLE_FP64
    cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
    cl_intel_platform_get_default_extension(ret);
    cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
    break;

  case PCI_CHIP_SANDYBRIDGE_BRIDGE:
  case PCI_CHIP_SANDYBRIDGE_GT1:
  case PCI_CHIP_SANDYBRIDGE_GT2:
  case PCI_CHIP_SANDYBRIDGE_GT2_PLUS:
  case PCI_CHIP_SANDYBRIDGE_BRIDGE_M:
  case PCI_CHIP_SANDYBRIDGE_M_GT1:
  case PCI_CHIP_SANDYBRIDGE_M_GT2:
  case PCI_CHIP_SANDYBRIDGE_M_GT2_PLUS:
  case PCI_CHIP_SANDYBRIDGE_BRIDGE_S:
  case PCI_CHIP_SANDYBRIDGE_S_GT:
    // Intel(R) HD Graphics SandyBridge not supported yet
    ret = NULL;
    break;
  default:
    CL_LOG_WARNING("cl_get_gt_device(): error, unknown device: %x", device_id);
  }

  if (ret == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(ret, CL_OBJECT_DEVICE_MAGIC);

  /* Apply any driver-dependent updates to the device info */
  intel_update_device_info(ret);

#define toMB(size) (size) & (UINT64_MAX << 20)
  /* Get the global_mem_size and max_mem_alloc size from
   * driver, system ram and hardware*/
  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    uint64_t totalgpumem = ret->global_mem_size;
    uint64_t maxallocmem = ret->max_mem_alloc_size;
    uint64_t totalram = info.totalram * info.mem_unit;
    /* In case to keep system stable we just use half
     * of the raw as global mem */
    ret->global_mem_size = toMB((totalram / 2 > totalgpumem) ? totalgpumem : totalram / 2);
    /* The hardware has some limit about the alloc size
     * and the excution of kernel need some global mem
     * so we now make sure single mem does not use much
     * than 3/4 global mem*/
    ret->max_mem_alloc_size = toMB((ret->global_mem_size * 3 / 4 > maxallocmem) ? maxallocmem : ret->global_mem_size * 3 / 4);
  }

  device = ret;
  return ret;
}

typedef enum cl_self_test_res {
  SELF_TEST_PASS = 0,
  SELF_TEST_SLM_FAIL = 1,
  SELF_TEST_ATOMIC_FAIL = 2,
  SELF_TEST_OTHER_FAIL = 3,
} cl_self_test_res;

/* Runs a small kernel to check that the device works; returns
 * SELF_TEST_PASS: for success.
 * SELF_TEST_SLM_FAIL: for SLM results mismatch;
 * SELF_TEST_ATOMIC_FAIL: for hsw enqueue  kernel failure to not enable atomics in L3.
 * SELF_TEST_OTHER_FAIL: other fail like runtime API fail.*/
static cl_self_test_res
cl_gen_self_test(cl_device_id device)
{
  cl_int status;
  cl_context ctx;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;
  cl_mem buffer;
  cl_event kernel_finished;
  size_t n = 3;
  cl_int test_data[3] = {3, 7, 5};
  const char *kernel_source = "__kernel void self_test(__global int *buf) {"
                              "  __local int tmp[3];"
                              "  tmp[get_local_id(0)] = buf[get_local_id(0)];"
                              "  barrier(CLK_LOCAL_MEM_FENCE);"
                              "  buf[get_global_id(0)] = tmp[2 - get_local_id(0)] + buf[get_global_id(0)];"
                              "}"; // using __local to catch the "no SLM on Haswell" problem
  static int tested = 0;
  static cl_self_test_res ret = SELF_TEST_OTHER_FAIL;
  if (tested != 0)
    return ret;
  tested = 1;
  ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
  if (!ctx)
    return ret;

  if (status == CL_SUCCESS) {
    queue = clCreateCommandQueueWithProperties(ctx, device, 0, &status);
    if (status == CL_SUCCESS) {
      program = clCreateProgramWithSource(ctx, 1, &kernel_source, NULL, &status);
      if (status == CL_SUCCESS) {
        status = clBuildProgram(program, 1, &device, "", NULL, NULL);
        if (status == CL_SUCCESS) {
          kernel = clCreateKernel(program, "self_test", &status);
          if (status == CL_SUCCESS) {
            buffer = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, n * 4, test_data, &status);
            if (status == CL_SUCCESS) {
              status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
              if (status == CL_SUCCESS) {
                status = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &n, &n, 0, NULL, &kernel_finished);
                if (status == CL_SUCCESS) {
                  status = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, n * 4, test_data, 1, &kernel_finished, NULL);
                  if (status == CL_SUCCESS) {
                    if (test_data[0] == 8 && test_data[1] == 14 && test_data[2] == 8) {
                      ret = SELF_TEST_PASS;
                    } else {
                      ret = SELF_TEST_SLM_FAIL;
                      CL_LOG_WARNING("Beignet: self-test failed: (3, 7, 5) + (5, 7, 3) returned (%i, %i, %i)\n"
                                     "See README.md or http://www.freedesktop.org/wiki/Software/Beignet/",
                                     test_data[0], test_data[1], test_data[2]);
                    }
                  }
                } else {
                  ret = SELF_TEST_ATOMIC_FAIL;
                  // Atomic fail need to test SLM again with atomic in L3 feature disabled.
                  tested = 0;
                }
                clReleaseEvent(kernel_finished);
              }
            }
            clReleaseMemObject(buffer);
          }
          clReleaseKernel(kernel);
        }
      }
      clReleaseProgram(program);
    }
    clReleaseCommandQueue(queue);
  }
  clReleaseContext(ctx);
  return ret;
}

static struct _cl_device_id_gen device_gen;
static cl_device_id __gen_device = NULL;

LOCAL cl_device_id
cl_device_get_id_gen(cl_platform_id platform)
{
  static int inited = 0;
  cl_device_id dev = NULL;

  if (__gen_device)
    return __gen_device;

  if (inited)
    return NULL;

  inited = 1;
  dev = get_gt_device(platform);
  if (dev == NULL)
    return NULL;

  memset(&device_gen.base, 0, sizeof(_cl_device_id_gen));
  memcpy(&device_gen.base, dev, sizeof(_cl_device_id));
  __gen_device = &device_gen.base;

  if (cl_compiler_load_gen(__gen_device) == CL_FALSE) {
    __gen_device->profile = "EMBEDDED_PROFILE";
    __gen_device->profile_sz = strlen(__gen_device->profile) + 1;
  }

  /* Setup all the device api */
  __gen_device->api = __gen_device_api;

  if (cl_compiler_check_available(__gen_device) != CL_SUCCESS)
    return __gen_device;

  /* Run a self test for more info, some device may fail the atomic and slm */
  cl_self_test_res ret = cl_gen_self_test(__gen_device);
  if (ret == SELF_TEST_ATOMIC_FAIL) {
    __gen_device->atomic_test_result = 0;
    ret = cl_gen_self_test(__gen_device); // Run it again to test slm
    assert(ret != SELF_TEST_ATOMIC_FAIL);
    CL_LOG_WARNING("disable atomic in L3 feature.");
  }

  if (ret == SELF_TEST_SLM_FAIL) { // Can not use slm is a serious bug
    int disable_self_test = 0;
    // can't use BVAR (backend/src/sys/cvar.hpp) here as it's C++
    const char *env = getenv("OCL_IGNORE_SELF_TEST");
    if (env != NULL) {
      sscanf(env, "%i", &disable_self_test);
    }
    if (disable_self_test) {
      CL_LOG_WARNING("overriding self-test failure");
    } else {
      CL_LOG_WARNING("Disabling non-working device");
      __gen_device = NULL;
    }
  }

  if (__gen_device == NULL)
    return NULL;

  __gen_device->built_in_kernels = cl_internal_built_in_kernel_str_kernels;
  __gen_device->built_in_kernels_binary = (char *)&cl_internal_built_in_kernel_str;
  __gen_device->built_in_kernels_binary_sz = cl_internal_built_in_kernel_str_size;

  return __gen_device;
}

LOCAL void
cl_device_gen_cleanup(void)
{
  int i;
  cl_device_id_gen gen_dev = NULL;

  if (__gen_device == NULL)
    return;

  gen_dev = (cl_device_id_gen)__gen_device;

  for (i = 0; i < CL_INTERNAL_KERNEL_MAX; i++) {
    if (gen_dev->internal_kernels[i]) {
      cl_kernel_delete(gen_dev->internal_kernels[i]);
      gen_dev->internal_kernels[i] = NULL;
    }

    if (gen_dev->internal_program[i]) {
      cl_program_delete(gen_dev->internal_program[i]);
      gen_dev->internal_program[i] = NULL;
    }
  }
}

LOCAL cl_int
cl_device_get_version_gen(cl_device_id device, cl_int *ver)
{
  if (device != __gen_device)
    return CL_INVALID_DEVICE;

  if (IS_GEN7(device->device_id)) {
    *ver = 7;
  } else if (IS_GEN75(device->device_id)) {
    *ver = 75;
  } else if (IS_GEN8(device->device_id)) {
    *ver = 8;
  } else if (IS_GEN9(device->device_id)) {
    *ver = 9;
  } else {
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}
