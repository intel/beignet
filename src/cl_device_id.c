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
 * Author: Benjamin Segovia <benjamin.segovia@intel.com>
 */

#include "cl_platform_id.h"
#include "cl_device_id.h"
#include "cl_internals.h"
#include "cl_utils.h"
#include "cl_driver.h"
#include "cl_device_data.h"
#include "cl_khr_icd.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"
#include "CL/cl_intel.h"
#include "cl_gbe_loader.h"
#include "cl_alloc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

#ifndef CL_VERSION_1_2
#define CL_DEVICE_BUILT_IN_KERNELS 0x103F
#endif

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
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_skl_gt2_device = {
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_skl_gt3_device = {
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_skl_gt4_device = {
  .max_compute_unit = 72,
  .max_thread_per_unit = 7,
  .sub_slice_count = 9,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
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
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt15_device = {
  .max_compute_unit = 18,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt2_device = {
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt3_device = {
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_kbl_gt4_device = {
  .max_compute_unit = 72,
  .max_thread_per_unit = 7,
  .sub_slice_count = 9,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_glk18eu_device = {
  .max_compute_unit = 18,
  .max_thread_per_unit = 6,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

static struct _cl_device_id intel_glk12eu_device = {
  .max_compute_unit = 12,
  .max_thread_per_unit = 6,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen9_device.h"
};

LOCAL cl_device_id
cl_get_gt_device(cl_device_type device_type)
{
  cl_device_id ret = NULL;
  const int device_id = cl_driver_get_device_id();
  cl_device_id device = NULL;

  //cl_get_gt_device only return GPU type device.
  if (((CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT) & device_type) == 0)
    return NULL;

#define DECL_INFO_STRING(BREAK, STRUCT, FIELD, STRING) \
    STRUCT.FIELD = STRING; \
    STRUCT.JOIN(FIELD,_sz) = sizeof(STRING); \
    device = &STRUCT; \
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
      device->platform = cl_get_platform_default();
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
      intel_ivb_gt1_device.platform = cl_get_platform_default();
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
      intel_ivb_gt2_device.platform = cl_get_platform_default();
      ret = &intel_ivb_gt2_device;
      cl_intel_platform_get_default_extension(ret);
      cl_intel_platform_enable_extension(ret, cl_intel_motion_estimation_ext_id);
      break;

    case PCI_CHIP_BAYTRAIL_T:
      DECL_INFO_STRING(baytrail_t_device_break, intel_baytrail_t_device, name, "Intel(R) HD Graphics Bay Trail-T");
baytrail_t_device_break:
      intel_baytrail_t_device.device_id = device_id;
      intel_baytrail_t_device.platform = cl_get_platform_default();
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
      intel_brw_gt1_device.platform = cl_get_platform_default();
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
      intel_brw_gt2_device.platform = cl_get_platform_default();
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
      intel_brw_gt3_device.platform = cl_get_platform_default();
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
      intel_chv_device.platform = cl_get_platform_default();
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
      intel_skl_gt1_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt1_device;
#ifdef ENABLE_FP64
      cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
      cl_intel_platform_get_default_extension(ret);
      cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
      cl_intel_platform_enable_extension(ret, cl_intel_device_side_avc_motion_estimation_ext_id);
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
      intel_skl_gt2_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt2_device;
#ifdef ENABLE_FP64
      cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
      cl_intel_platform_get_default_extension(ret);
      cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
      cl_intel_platform_enable_extension(ret, cl_intel_device_side_avc_motion_estimation_ext_id);
      break;

    case PCI_CHIP_SKYLAKE_ULT_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake ULT GT3");
    case PCI_CHIP_SKYLAKE_ULT_GT3E1:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake ULT GT3E");
    case PCI_CHIP_SKYLAKE_ULT_GT3E2:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake ULT GT3E");
    case PCI_CHIP_SKYLAKE_HALO_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Halo GT3");
    case PCI_CHIP_SKYLAKE_SRV_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Server GT3");
    case PCI_CHIP_SKYLAKE_MEDIA_SRV_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Media Server GT3");
skl_gt3_break:
      intel_skl_gt3_device.device_id = device_id;
      intel_skl_gt3_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt3_device;
      cl_intel_platform_get_default_extension(ret);
#ifdef ENABLE_FP64
      cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
      cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
      cl_intel_platform_enable_extension(ret, cl_intel_device_side_avc_motion_estimation_ext_id);
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
      intel_skl_gt4_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt4_device;
#ifdef ENABLE_FP64
      cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
      cl_intel_platform_get_default_extension(ret);
      cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
      cl_intel_platform_enable_extension(ret, cl_intel_device_side_avc_motion_estimation_ext_id);
      break;

    case PCI_CHIP_BROXTON_0:
      DECL_INFO_STRING(bxt18eu_break, intel_bxt18eu_device, name, "Intel(R) HD Graphics Broxton 0");
    case PCI_CHIP_BROXTON_2:
      DECL_INFO_STRING(bxt18eu_break, intel_bxt18eu_device, name, "Intel(R) HD Graphics Broxton 2");
bxt18eu_break:
      intel_bxt18eu_device.device_id = device_id;
      intel_bxt18eu_device.platform = cl_get_platform_default();
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
      intel_bxt12eu_device.platform = cl_get_platform_default();
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
      intel_kbl_gt1_device.platform = cl_get_platform_default();
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
      intel_kbl_gt15_device.platform = cl_get_platform_default();
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
      intel_kbl_gt2_device.platform = cl_get_platform_default();
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
      intel_kbl_gt3_device.platform = cl_get_platform_default();
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
      intel_kbl_gt4_device.platform = cl_get_platform_default();
      ret = &intel_kbl_gt4_device;
#ifdef ENABLE_FP64
      cl_intel_platform_enable_extension(ret, cl_khr_fp64_ext_id);
#endif
      cl_intel_platform_get_default_extension(ret);
      cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
      break;

    case PCI_CHIP_GLK_3x6:
      DECL_INFO_STRING(glk18eu_break, intel_bxt18eu_device, name, "Intel(R) HD Graphics Geminilake(3x6)");
glk18eu_break:
      intel_glk18eu_device.device_id = device_id;
      intel_glk18eu_device.platform = cl_get_platform_default();
      ret = &intel_glk18eu_device;
      cl_intel_platform_get_default_extension(ret);
      cl_intel_platform_enable_extension(ret, cl_khr_fp16_ext_id);
      break;

    case PCI_CHIP_GLK_2x6:
      DECL_INFO_STRING(glk12eu_break, intel_bxt12eu_device, name, "Intel(R) HD Graphics Geminilake(2x6)");
glk12eu_break:
      intel_glk12eu_device.device_id = device_id;
      intel_glk12eu_device.platform = cl_get_platform_default();
      ret = &intel_glk12eu_device;
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
      printf("cl_get_gt_device(): error, unknown device: %x\n", device_id);
  }

  if (ret == NULL)
    return NULL;

  CL_OBJECT_INIT_BASE(ret, CL_OBJECT_DEVICE_MAGIC);
  if (!CompilerSupported()) {
    ret->compiler_available = CL_FALSE;
    //ret->linker_available = CL_FALSE;
    ret->profile = "EMBEDDED_PROFILE";
    ret->profile_sz = strlen(ret->profile) + 1;
  }

  /* Apply any driver-dependent updates to the device info */
  cl_driver_update_device_info(ret);

  #define toMB(size) (size)&(UINT64_MAX<<20)
  /* Get the global_mem_size and max_mem_alloc size from
   * driver, system ram and hardware*/
  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    uint64_t totalgpumem = ret->global_mem_size;
	uint64_t maxallocmem = ret->max_mem_alloc_size;
    uint64_t totalram = info.totalram * info.mem_unit;
	/* In case to keep system stable we just use half
	 * of the raw as global mem */
    ret->global_mem_size = toMB((totalram / 2 > totalgpumem) ?
                            totalgpumem: totalram / 2);
	/* The hardware has some limit about the alloc size
	 * and the excution of kernel need some global mem
	 * so we now make sure single mem does not use much
	 * than 3/4 global mem*/
    ret->max_mem_alloc_size = toMB((ret->global_mem_size * 3 / 4 > maxallocmem) ?
                              maxallocmem: ret->global_mem_size * 3 / 4);
  }

  return ret;
}

/* Runs a small kernel to check that the device works; returns
 * SELF_TEST_PASS: for success.
 * SELF_TEST_SLM_FAIL: for SLM results mismatch;
 * SELF_TEST_ATOMIC_FAIL: for hsw enqueue  kernel failure to not enable atomics in L3.
 * SELF_TEST_OTHER_FAIL: other fail like runtime API fail.*/
LOCAL cl_self_test_res
cl_self_test(cl_device_id device, cl_self_test_res atomic_in_l3_flag)
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
  const char* kernel_source = "__kernel void self_test(__global int *buf) {"
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
  if(!ctx)
    return ret;
  cl_driver_set_atomic_flag(ctx->drv, atomic_in_l3_flag);
  if (status == CL_SUCCESS) {
    queue = clCreateCommandQueueWithProperties(ctx, device, 0, &status);
    if (status == CL_SUCCESS) {
      program = clCreateProgramWithSource(ctx, 1, &kernel_source, NULL, &status);
      if (status == CL_SUCCESS) {
        status = clBuildProgram(program, 1, &device, "", NULL, NULL);
        if (status == CL_SUCCESS) {
          kernel = clCreateKernel(program, "self_test", &status);
          if (status == CL_SUCCESS) {
            buffer = clCreateBuffer(ctx, CL_MEM_COPY_HOST_PTR, n*4, test_data, &status);
            if (status == CL_SUCCESS) {
              status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
              if (status == CL_SUCCESS) {
                status = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &n, &n, 0, NULL, &kernel_finished);
                if (status == CL_SUCCESS) {
                  status = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, n*4, test_data, 1, &kernel_finished, NULL);
                  if (status == CL_SUCCESS) {
                    if (test_data[0] == 8 && test_data[1] == 14 && test_data[2] == 8){
                      ret = SELF_TEST_PASS;
                    } else {
                      ret = SELF_TEST_SLM_FAIL;
                      printf("Beignet: self-test failed: (3, 7, 5) + (5, 7, 3) returned (%i, %i, %i)\n"
                             "See README.md or http://www.freedesktop.org/wiki/Software/Beignet/\n",
                             test_data[0], test_data[1], test_data[2]);

                    }
                  }
                } else{
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

LOCAL cl_int
cl_get_device_ids(cl_platform_id    platform,
                  cl_device_type    device_type,
                  cl_uint           num_entries,
                  cl_device_id *    devices,
                  cl_uint *         num_devices)
{
  cl_device_id device;

  /* Do we have a usable device? */
  device = cl_get_gt_device(device_type);
  if (device) {
    cl_self_test_res ret = cl_self_test(device, SELF_TEST_PASS);
    if (ret == SELF_TEST_ATOMIC_FAIL) {
      device->atomic_test_result = ret;
      ret = cl_self_test(device, ret);
      printf("Beignet: warning - disable atomic in L3 feature.\n");
    }

    if(ret == SELF_TEST_SLM_FAIL) {
      int disable_self_test = 0;
      // can't use BVAR (backend/src/sys/cvar.hpp) here as it's C++
      const char *env = getenv("OCL_IGNORE_SELF_TEST");
      if (env != NULL) {
        sscanf(env, "%i", &disable_self_test);
      }
      if (disable_self_test) {
        printf("Beignet: Warning - overriding self-test failure\n");
      } else {
        printf("Beignet: disabling non-working device\n");
        device = 0;
      }
    }
  }
  if (!device) {
    if (num_devices)
      *num_devices = 0;
    if (devices)
      *devices = 0;
    return CL_DEVICE_NOT_FOUND;
  } else {
    if (num_devices)
      *num_devices = 1;
    if (devices) {
      *devices = device;
    }
    return CL_SUCCESS;
  }
}

LOCAL cl_bool is_gen_device(cl_device_id device) {
  return device == &intel_ivb_gt1_device ||
         device == &intel_ivb_gt2_device ||
         device == &intel_baytrail_t_device ||
         device == &intel_hsw_gt1_device ||
         device == &intel_hsw_gt2_device ||
         device == &intel_hsw_gt3_device ||
         device == &intel_brw_gt1_device ||
         device == &intel_brw_gt2_device ||
         device == &intel_brw_gt3_device ||
         device == &intel_chv_device ||
         device == &intel_skl_gt1_device ||
         device == &intel_skl_gt2_device ||
         device == &intel_skl_gt3_device ||
         device == &intel_skl_gt4_device ||
         device == &intel_bxt18eu_device ||
         device == &intel_bxt12eu_device ||
         device == &intel_kbl_gt1_device ||
         device == &intel_kbl_gt15_device ||
         device == &intel_kbl_gt2_device ||
         device == &intel_kbl_gt3_device ||
         device == &intel_kbl_gt4_device ||
         device == &intel_glk18eu_device ||
         device == &intel_glk12eu_device;
}

LOCAL cl_int
cl_get_device_info(cl_device_id     device,
                   cl_device_info   param_name,
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret)
{
  const void *src_ptr = NULL;
  size_t src_size = 0;
  cl_int dev_ref;

  // We now just support gen devices.
  if (UNLIKELY(is_gen_device(device) == CL_FALSE))
    return CL_INVALID_DEVICE;

  /* Find the correct parameter */
  switch (param_name) {
    case CL_DEVICE_TYPE:
      src_ptr = &device->device_type;
      src_size = sizeof(device->device_type);
      break;
    case CL_DEVICE_VENDOR_ID:
      src_ptr = &device->vendor_id;
      src_size = sizeof(device->vendor_id);
      break;
    case CL_DEVICE_MAX_COMPUTE_UNITS:
      src_ptr = &device->max_compute_unit;
      src_size = sizeof(device->max_compute_unit);
      break;
    case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
      src_ptr = &device->max_work_item_dimensions;
      src_size = sizeof(device->max_work_item_dimensions);
      break;
    case CL_DEVICE_MAX_WORK_ITEM_SIZES:
      src_ptr = &device->max_work_item_sizes;
      src_size = sizeof(device->max_work_item_sizes);
      break;
    case CL_DEVICE_MAX_WORK_GROUP_SIZE:
      src_ptr = &device->max_work_group_size;
      src_size = sizeof(device->max_work_group_size);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
      src_ptr = &device->preferred_vector_width_char;
      src_size = sizeof(device->preferred_vector_width_char);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
      src_ptr = &device->preferred_vector_width_short;
      src_size = sizeof(device->preferred_vector_width_short);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
      src_ptr = &device->preferred_vector_width_int;
      src_size = sizeof(device->preferred_vector_width_int);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
      src_ptr = &device->preferred_vector_width_long;
      src_size = sizeof(device->preferred_vector_width_long);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
      src_ptr = &device->preferred_vector_width_float;
      src_size = sizeof(device->preferred_vector_width_float);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
      src_ptr = &device->preferred_vector_width_double;
      src_size = sizeof(device->preferred_vector_width_double);
      break;
    case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
      src_ptr = &device->preferred_vector_width_half;
      src_size = sizeof(device->preferred_vector_width_half);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
      src_ptr = &device->native_vector_width_char;
      src_size = sizeof(device->native_vector_width_char);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
      src_ptr = &device->native_vector_width_short;
      src_size = sizeof(device->native_vector_width_short);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
      src_ptr = &device->native_vector_width_int;
      src_size = sizeof(device->native_vector_width_int);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
      src_ptr = &device->native_vector_width_long;
      src_size = sizeof(device->native_vector_width_long);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
      src_ptr = &device->native_vector_width_float;
      src_size = sizeof(device->native_vector_width_float);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
      src_ptr = &device->native_vector_width_double;
      src_size = sizeof(device->native_vector_width_double);
      break;
    case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
      src_ptr = &device->native_vector_width_half;
      src_size = sizeof(device->native_vector_width_half);
      break;
    case CL_DEVICE_MAX_CLOCK_FREQUENCY:
      src_ptr = &device->max_clock_frequency;
      src_size = sizeof(device->max_clock_frequency);
      break;
    case CL_DEVICE_ADDRESS_BITS:
      src_ptr = &device->address_bits;
      src_size = sizeof(device->address_bits);
      break;
    case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
      src_ptr = &device->max_mem_alloc_size;
      src_size = sizeof(device->max_mem_alloc_size);
      break;
    case CL_DEVICE_IMAGE_SUPPORT:
      src_ptr = &device->image_support;
      src_size = sizeof(device->image_support);
      break;
    case CL_DEVICE_MAX_READ_IMAGE_ARGS:
      src_ptr = &device->max_read_image_args;
      src_size = sizeof(device->max_read_image_args);
      break;
    case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
      src_ptr = &device->max_write_image_args;
      src_size = sizeof(device->max_write_image_args);
      break;
    case CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS:
      src_ptr = &device->max_read_write_image_args;
      src_size = sizeof(device->max_read_write_image_args);
      break;
    case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
      src_ptr = &device->image_max_array_size;
      src_size = sizeof(device->image_max_array_size);
      break;
    case CL_DEVICE_IMAGE2D_MAX_WIDTH:
    case CL_DEVICE_PLANAR_YUV_MAX_WIDTH_INTEL:
      src_ptr = &device->image2d_max_width;
      src_size = sizeof(device->image2d_max_width);
      break;
    case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
    case CL_DEVICE_PLANAR_YUV_MAX_HEIGHT_INTEL:
      src_ptr = &device->image2d_max_height;
      src_size = sizeof(device->image2d_max_height);
      break;
    case CL_DEVICE_IMAGE3D_MAX_WIDTH:
      src_ptr = &device->image3d_max_width;
      src_size = sizeof(device->image3d_max_width);
      break;
    case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
      src_ptr = &device->image3d_max_height;
      src_size = sizeof(device->image3d_max_height);
      break;
    case CL_DEVICE_IMAGE3D_MAX_DEPTH:
      src_ptr = &device->image3d_max_depth;
      src_size = sizeof(device->image3d_max_depth);
      break;
    case CL_DEVICE_MAX_SAMPLERS:
      src_ptr = &device->max_samplers;
      src_size = sizeof(device->max_samplers);
      break;
    case CL_DEVICE_MAX_PARAMETER_SIZE:
      src_ptr = &device->max_parameter_size;
      src_size = sizeof(device->max_parameter_size);
      break;
    case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
      src_ptr = &device->mem_base_addr_align;
      src_size = sizeof(device->mem_base_addr_align);
      break;
    case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
      src_ptr = &device->min_data_type_align_size;
      src_size = sizeof(device->min_data_type_align_size);
      break;
    case CL_DEVICE_MAX_PIPE_ARGS:
      src_ptr = &device->max_pipe_args;
      src_size = sizeof(device->max_pipe_args);
      break;
    case CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS:
      src_ptr = &device->pipe_max_active_reservations;
      src_size = sizeof(device->pipe_max_active_reservations);
      break;
    case CL_DEVICE_PIPE_MAX_PACKET_SIZE:
      src_ptr = &device->pipe_max_packet_siz;
      src_size = sizeof(device->pipe_max_packet_siz);
      break;
    case CL_DEVICE_SINGLE_FP_CONFIG:
      src_ptr = &device->single_fp_config;
      src_size = sizeof(device->single_fp_config);
      break;
    case CL_DEVICE_HALF_FP_CONFIG:
      src_ptr = &device->half_fp_config;
      src_size = sizeof(device->half_fp_config);
      break;
    case CL_DEVICE_DOUBLE_FP_CONFIG:
      src_ptr = &device->double_fp_config;
      src_size = sizeof(device->double_fp_config);
      break;
    case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
      src_ptr = &device->global_mem_cache_type;
      src_size = sizeof(device->global_mem_cache_type);
      break;
    case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
      src_ptr = &device->global_mem_cache_line_size;
      src_size = sizeof(device->global_mem_cache_line_size);
      break;
    case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
      src_ptr = &device->global_mem_cache_size;
      src_size = sizeof(device->global_mem_cache_size);
      break;
    case CL_DEVICE_GLOBAL_MEM_SIZE:
      src_ptr = &device->global_mem_size;
      src_size = sizeof(device->global_mem_size);
      break;
    case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
      src_ptr = &device->max_constant_buffer_size;
      src_size = sizeof(device->max_constant_buffer_size);
      break;
    case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
      src_ptr = &device->image_mem_size;
      src_size = sizeof(device->image_mem_size);
      break;
    case CL_DEVICE_MAX_CONSTANT_ARGS:
      src_ptr = &device->max_constant_args;
      src_size = sizeof(device->max_constant_args);
      break;
    case CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE:
      src_ptr = &device->max_global_variable_size;
      src_size = sizeof(device->max_global_variable_size);
      break;
    case CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE:
      src_ptr = &device->global_variable_preferred_total_size;
      src_size = sizeof(device->global_variable_preferred_total_size);
      break;
    case CL_DEVICE_LOCAL_MEM_TYPE:
      src_ptr = &device->local_mem_type;
      src_size = sizeof(device->local_mem_type);
      break;
    case CL_DEVICE_LOCAL_MEM_SIZE:
      src_ptr = &device->local_mem_size;
      src_size = sizeof(device->local_mem_size);
      break;
    case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
      src_ptr = &device->error_correction_support;
      src_size = sizeof(device->error_correction_support);
      break;
    case CL_DEVICE_HOST_UNIFIED_MEMORY:
      src_ptr = &device->host_unified_memory;
      src_size = sizeof(device->host_unified_memory);
      break;
    case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
      src_ptr = &device->profiling_timer_resolution;
      src_size = sizeof(device->profiling_timer_resolution);
      break;
    case CL_DEVICE_ENDIAN_LITTLE:
      src_ptr = &device->endian_little;
      src_size = sizeof(device->endian_little);
      break;
    case CL_DEVICE_AVAILABLE:
      src_ptr = &device->available;
      src_size = sizeof(device->available);
      break;
    case CL_DEVICE_COMPILER_AVAILABLE:
      src_ptr = &device->compiler_available;
      src_size = sizeof(device->compiler_available);
      break;
    case CL_DEVICE_LINKER_AVAILABLE:
      src_ptr = &device->linker_available;
      src_size = sizeof(device->linker_available);
      break;
    case CL_DEVICE_EXECUTION_CAPABILITIES:
      src_ptr = &device->execution_capabilities;
      src_size = sizeof(device->execution_capabilities);
      break;
    case CL_DEVICE_QUEUE_PROPERTIES:
      src_ptr = &device->queue_properties;
      src_size = sizeof(device->queue_properties);
      break;
    case CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES:
      src_ptr = &device->queue_on_device_properties;
      src_size = sizeof(device->queue_on_device_properties);
      break;
    case CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE:
      src_ptr = &device->queue_on_device_preferred_size;
      src_size = sizeof(device->queue_on_device_preferred_size);
      break;
    case CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE:
      src_ptr = &device->queue_on_device_max_size;
      src_size = sizeof(device->queue_on_device_max_size);
      break;
    case CL_DEVICE_MAX_ON_DEVICE_QUEUES:
      src_ptr = &device->max_on_device_queues;
      src_size = sizeof(device->max_on_device_queues);
      break;
    case CL_DEVICE_MAX_ON_DEVICE_EVENTS:
      src_ptr = &device->max_on_device_events;
      src_size = sizeof(device->max_on_device_events);
      break;
    case CL_DEVICE_PLATFORM:
      src_ptr = &device->platform;
      src_size = sizeof(device->platform);
      break;
    case CL_DEVICE_PRINTF_BUFFER_SIZE:
      src_ptr = &device->printf_buffer_size;
      src_size = sizeof(device->printf_buffer_size);
      break;
    case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
      src_ptr = &device->interop_user_sync;
      src_size = sizeof(device->interop_user_sync);
      break;
    case CL_DEVICE_NAME:
      src_ptr = device->name;
      src_size = device->name_sz;
      break;
    case CL_DEVICE_VENDOR:
      src_ptr = device->vendor;
      src_size = device->vendor_sz;
      break;
    case CL_DEVICE_VERSION:
      src_ptr = device->version;
      src_size = device->version_sz;
      break;
    case CL_DEVICE_PROFILE:
      src_ptr = device->profile;
      src_size = device->profile_sz;
      break;
    case CL_DEVICE_OPENCL_C_VERSION:
      src_ptr = device->opencl_c_version;
      src_size = device->opencl_c_version_sz;
      break;
    case CL_DEVICE_SPIR_VERSIONS:
      src_ptr = device->spir_versions;
      src_size = device->spir_versions_sz;
      break;
    case CL_DEVICE_EXTENSIONS:
      src_ptr = device->extensions;
      src_size = device->extensions_sz;
      break;
    case CL_DEVICE_BUILT_IN_KERNELS:
      src_ptr = device->built_in_kernels;
      src_size = device->built_in_kernels_sz;
      break;
    case CL_DEVICE_PARENT_DEVICE:
      src_ptr = &device->parent_device;
      src_size = sizeof(device->parent_device);
      break;
    case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
      src_ptr = &device->partition_max_sub_device;
      src_size = sizeof(device->partition_max_sub_device);
      break;
    case CL_DEVICE_PARTITION_PROPERTIES:
      src_ptr = &device->partition_property;
      src_size = sizeof(device->partition_property);
      break;
    case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
      src_ptr = &device->affinity_domain;
      src_size = sizeof(device->affinity_domain);
      break;
    case CL_DEVICE_PARTITION_TYPE:
      src_ptr = &device->partition_type;
      src_size = sizeof(device->partition_type);
      break;
    case CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT:
      src_ptr = &device->preferred_platform_atomic_alignment;
      src_size = sizeof(device->preferred_platform_atomic_alignment);
      break;
    case CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT:
      src_ptr = &device->preferred_global_atomic_alignment;
      src_size = sizeof(device->preferred_global_atomic_alignment);
      break;
    case CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT:
      src_ptr = &device->preferred_local_atomic_alignment;
      src_size = sizeof(device->preferred_local_atomic_alignment);
      break;
    case CL_DEVICE_IMAGE_PITCH_ALIGNMENT:
      src_ptr = &device->image_pitch_alignment;
      src_size = sizeof(device->image_pitch_alignment);
      break;
    case CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT:
      src_ptr = &device->image_base_address_alignment;
      src_size = sizeof(device->image_base_address_alignment);
      break;
    case CL_DEVICE_SVM_CAPABILITIES:
      src_ptr = &device->svm_capabilities;
      src_size = sizeof(device->svm_capabilities);
      break;
    case CL_DEVICE_REFERENCE_COUNT:
      {
        dev_ref = CL_OBJECT_GET_REF(device);
        src_ptr = &dev_ref;
        src_size = sizeof(cl_int);
        break;
      }
    case CL_DRIVER_VERSION:
      src_ptr = device->driver_version;
      src_size = device->driver_version_sz;
      break;
    case CL_DEVICE_SUB_GROUP_SIZES_INTEL:
      src_ptr = device->sub_group_sizes;
      src_size = device->sub_group_sizes_sz;
      break;

    default:
      return CL_INVALID_VALUE;
  }

  return cl_get_info_helper(src_ptr, src_size,
                            param_value, param_value_size, param_value_size_ret);
}

LOCAL cl_int
cl_device_get_version(cl_device_id device, cl_int *ver)
{
  if (UNLIKELY(is_gen_device(device) == CL_FALSE))
    return CL_INVALID_DEVICE;
  if (ver == NULL)
    return CL_SUCCESS;
  if (device == &intel_ivb_gt1_device || 
      device == &intel_ivb_gt2_device ||
      device == &intel_baytrail_t_device) {
    *ver = 7;
  } else if (device == &intel_hsw_gt1_device || device == &intel_hsw_gt2_device
        || device == &intel_hsw_gt3_device) {
    *ver = 75;
  } else if (device == &intel_brw_gt1_device || device == &intel_brw_gt2_device
        || device == &intel_brw_gt3_device || device == &intel_chv_device) {
    *ver = 8;
  } else if (device == &intel_skl_gt1_device || device == &intel_skl_gt2_device
        || device == &intel_skl_gt3_device || device == &intel_skl_gt4_device
        || device == &intel_bxt18eu_device || device == &intel_bxt12eu_device || device == &intel_kbl_gt1_device
        || device == &intel_kbl_gt2_device || device == &intel_kbl_gt3_device
        || device == &intel_kbl_gt4_device || device == &intel_kbl_gt15_device
        || device == &intel_glk18eu_device || device == &intel_glk12eu_device) {
    *ver = 9;
  } else
    return CL_INVALID_VALUE;

  return CL_SUCCESS;
}
#undef DECL_FIELD

#define _DECL_FIELD(FIELD)                                 \
      if (param_value && param_value_size < sizeof(FIELD)) \
        return CL_INVALID_VALUE;                           \
      if (param_value_size_ret != NULL)                    \
        *param_value_size_ret = sizeof(FIELD);             \
      if (param_value)                                     \
        memcpy(param_value, &FIELD, sizeof(FIELD));        \
        return CL_SUCCESS;

#define DECL_FIELD(CASE,FIELD)                             \
  case JOIN(CL_KERNEL_,CASE):                              \
  _DECL_FIELD(FIELD)

#include "cl_kernel.h"
#include "cl_program.h"
static int
cl_check_builtin_kernel_dimension(cl_kernel kernel, cl_device_id device)
{
  const char * n = cl_kernel_get_name(kernel);
  const char * builtin_kernels_2d = "__cl_copy_image_2d_to_2d;__cl_copy_image_2d_to_buffer;__cl_copy_buffer_to_image_2d;__cl_fill_image_2d;__cl_fill_image_2d_array;";
  const char * builtin_kernels_3d = "__cl_copy_image_3d_to_2d;__cl_copy_image_2d_to_3d;__cl_copy_image_3d_to_3d;__cl_copy_image_3d_to_buffer;__cl_copy_buffer_to_image_3d;__cl_fill_image_3d";
    if (n == NULL || !strstr(device->built_in_kernels, n)){
      return 0;
    }else if(strstr(builtin_kernels_2d, n)){
      return 2;
    }else if(strstr(builtin_kernels_3d, n)){
      return 3;
    }else
      return 1;

}

LOCAL size_t
cl_get_kernel_max_wg_sz(cl_kernel kernel)
{
  size_t work_group_size, thread_cnt;
  int simd_width = interp_kernel_get_simd_width(kernel->opaque);
  int device_id = kernel->program->ctx->devices[0]->device_id;
  if (!interp_kernel_use_slm(kernel->opaque)) {
    if (!IS_BAYTRAIL_T(device_id) || simd_width == 16)
      work_group_size = simd_width * 64;
    else
      work_group_size = kernel->program->ctx->devices[0]->max_compute_unit *
                        kernel->program->ctx->devices[0]->max_thread_per_unit * simd_width;
  } else {
    thread_cnt = kernel->program->ctx->devices[0]->max_compute_unit *
                 kernel->program->ctx->devices[0]->max_thread_per_unit / kernel->program->ctx->devices[0]->sub_slice_count;
    if(thread_cnt > 64)
      thread_cnt = 64;
    work_group_size = thread_cnt * simd_width;
  }
  if(work_group_size > kernel->program->ctx->devices[0]->max_work_group_size)
    work_group_size = kernel->program->ctx->devices[0]->max_work_group_size;
  return work_group_size;
}

LOCAL cl_int
cl_get_kernel_workgroup_info(cl_kernel kernel,
                             cl_device_id device,
                             cl_kernel_work_group_info param_name,
                             size_t param_value_size,
                             void* param_value,
                             size_t* param_value_size_ret)
{
  int err = CL_SUCCESS;
  int dimension = 0;
  CHECK_KERNEL(kernel);
  if (device == NULL)
    device = kernel->program->ctx->devices[0];
  if (UNLIKELY(is_gen_device(device) == CL_FALSE))
    return CL_INVALID_DEVICE;

  switch (param_name) {
    case CL_KERNEL_WORK_GROUP_SIZE:
    {
      if (param_value && param_value_size < sizeof(size_t))
        return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(size_t);
      if (param_value) {
        size_t work_group_size = cl_get_kernel_max_wg_sz(kernel);
        *(size_t*)param_value = work_group_size;
        return CL_SUCCESS;
      }
    }
    case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
    {
      if (param_value && param_value_size < sizeof(size_t))
        return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(size_t);
      if (param_value)
        *(size_t*)param_value = interp_kernel_get_simd_width(kernel->opaque);
      return CL_SUCCESS;
    }
    case CL_KERNEL_LOCAL_MEM_SIZE:
    {
      size_t local_mem_sz =  interp_kernel_get_slm_size(kernel->opaque) + kernel->local_mem_sz;
      _DECL_FIELD(local_mem_sz)
    }
    DECL_FIELD(COMPILE_WORK_GROUP_SIZE, kernel->compile_wg_sz)
    DECL_FIELD(PRIVATE_MEM_SIZE, kernel->stack_size)
    case CL_KERNEL_GLOBAL_WORK_SIZE:
    {
      dimension = cl_check_builtin_kernel_dimension(kernel, device);
      if ( !dimension ) return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(device->max_1d_global_work_sizes);
      if (param_value) {
        if (dimension == 1) {
          memcpy(param_value, device->max_1d_global_work_sizes, sizeof(device->max_1d_global_work_sizes));
        }else if(dimension == 2){
          memcpy(param_value, device->max_2d_global_work_sizes, sizeof(device->max_2d_global_work_sizes));
        }else if(dimension == 3){
          memcpy(param_value, device->max_3d_global_work_sizes, sizeof(device->max_3d_global_work_sizes));
        }else
          return CL_INVALID_VALUE;

        return CL_SUCCESS;
      }
      return CL_SUCCESS;
    }
    case CL_KERNEL_SPILL_MEM_SIZE_INTEL:
    {
      if (param_value && param_value_size < sizeof(cl_ulong))
        return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(cl_ulong);
      if (param_value)
        *(cl_ulong*)param_value = (cl_ulong)interp_kernel_get_scratch_size(kernel->opaque);
      return CL_SUCCESS;
    }

    default:
      return CL_INVALID_VALUE;
  };

error:
  return err;
}

LOCAL cl_int
cl_get_kernel_subgroup_info(cl_kernel kernel,
                            cl_device_id device,
                            cl_kernel_work_group_info param_name,
                            size_t input_value_size,
                            const void* input_value,
                            size_t param_value_size,
                            void* param_value,
                            size_t* param_value_size_ret)
{
  int err = CL_SUCCESS;
  if(device != NULL)
    if (kernel->program->ctx->devices[0] != device)
      return CL_INVALID_DEVICE;

  CHECK_KERNEL(kernel);
  switch (param_name) {
    case CL_KERNEL_MAX_SUB_GROUP_SIZE_FOR_NDRANGE_KHR:
    {
      int i, dim = 0;
      size_t local_sz = 1;
      if (param_value && param_value_size < sizeof(size_t))
        return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(size_t);
      switch (input_value_size)
      {
        case sizeof(size_t)*1:
        case sizeof(size_t)*2:
        case sizeof(size_t)*3:
          dim = input_value_size/sizeof(size_t);
          break;
        default: return CL_INVALID_VALUE;
      }
      if (input_value == NULL )
        return CL_INVALID_VALUE;
      for(i = 0; i < dim; i++)
        local_sz *= ((size_t*)input_value)[i];
      if (param_value) {
        size_t simd_sz = cl_kernel_get_simd_width(kernel);
        size_t sub_group_size = local_sz >= simd_sz? simd_sz : local_sz;
        *(size_t*)param_value = sub_group_size;
        return CL_SUCCESS;
      }
      break;
    }
    case CL_KERNEL_SUB_GROUP_COUNT_FOR_NDRANGE_KHR:
    {
      int i, dim = 0;
      size_t local_sz = 1;
      if (param_value && param_value_size < sizeof(size_t))
        return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(size_t);
      switch (input_value_size)
      {
        case sizeof(size_t)*1:
        case sizeof(size_t)*2:
        case sizeof(size_t)*3:
          dim = input_value_size/sizeof(size_t);
          break;
        default: return CL_INVALID_VALUE;
      }
      if (input_value == NULL )
        return CL_INVALID_VALUE;
      for(i = 0; i < dim; i++)
        local_sz *= ((size_t*)input_value)[i];
      if (param_value) {
        size_t simd_sz = cl_kernel_get_simd_width(kernel);
        size_t sub_group_num = (local_sz + simd_sz - 1) / simd_sz;
        *(size_t*)param_value = sub_group_num;
        return CL_SUCCESS;
      }
      break;
    }
    case CL_KERNEL_COMPILE_SUB_GROUP_SIZE_INTEL:
    {
      if (param_value && param_value_size < sizeof(size_t))
        return CL_INVALID_VALUE;
      if (param_value_size_ret != NULL)
        *param_value_size_ret = sizeof(size_t);
      if (param_value)
        *(size_t*)param_value = interp_kernel_get_simd_width(kernel->opaque);
      return CL_SUCCESS;
    }
    default:
      return CL_INVALID_VALUE;
  };

error:
  return err;
}

LOCAL cl_int
cl_devices_list_check(cl_uint num_devices, const cl_device_id *devices)
{
  cl_uint i;

  if (devices == NULL)
    return CL_INVALID_DEVICE;

  assert(num_devices > 0);
  for (i = 0; i < num_devices; i++) {
    if (!CL_OBJECT_IS_DEVICE(devices[i])) {
      return CL_INVALID_DEVICE;
    }

    if (devices[i]->available == CL_FALSE) {
      return CL_DEVICE_NOT_AVAILABLE;
    }

    // We now just support one platform.
    if (devices[i]->platform != cl_get_platform_default()) {
      return CL_INVALID_DEVICE;
    }

    // TODO: We now just support Gen Device.
    if (devices[i] != cl_get_gt_device(devices[i]->device_type)) {
      return CL_INVALID_DEVICE;
    }
  }

  return CL_SUCCESS;
}

LOCAL cl_int
cl_devices_list_include_check(cl_uint num_devices, const cl_device_id *devices,
                              cl_uint num_to_check, const cl_device_id *devices_to_check)
{
  cl_uint i, j;

  for (i = 0; i < num_to_check; i++) {
    for (j = 0; j < num_devices; j++) {
      if (devices_to_check[i] == devices[j])
        break;
    }

    if (j == num_devices)
      return CL_INVALID_DEVICE;
  }

  return CL_SUCCESS;
}
