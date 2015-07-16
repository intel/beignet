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
#include "cl_thread.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"
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
  INIT_ICD(dispatch)
  .max_compute_unit = 16,
  .max_thread_per_unit = 8,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen7_device.h"
};

static struct _cl_device_id intel_ivb_gt1_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 6,
  .max_thread_per_unit = 6,
  .sub_slice_count = 1,
  .max_work_item_sizes = {256, 256, 256},
  .max_work_group_size = 256,
  .max_clock_frequency = 1000,
#include "cl_gen7_device.h"
};

static struct _cl_device_id intel_baytrail_t_device = {
  INIT_ICD(dispatch)
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
  INIT_ICD(dispatch)
  .max_compute_unit = 10,
  .max_thread_per_unit = 7,
  .sub_slice_count = 1,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_hsw_gt2_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 20,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_hsw_gt3_device = {
  INIT_ICD(dispatch)
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
  INIT_ICD(dispatch)
  .max_compute_unit = 12,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_brw_gt2_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_brw_gt3_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

//Cherryview has the same pciid, must get the max_compute_unit and max_thread_per_unit from drm
static struct _cl_device_id intel_chv_device = {
  INIT_ICD(dispatch)
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
  INIT_ICD(dispatch)
  .max_compute_unit = 6,
  .max_thread_per_unit = 7,
  .sub_slice_count = 2,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_skl_gt2_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 24,
  .max_thread_per_unit = 7,
  .sub_slice_count = 3,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_skl_gt3_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 48,
  .max_thread_per_unit = 7,
  .sub_slice_count = 6,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

static struct _cl_device_id intel_skl_gt4_device = {
  INIT_ICD(dispatch)
  .max_compute_unit = 72,
  .max_thread_per_unit = 7,
  .sub_slice_count = 9,
  .max_work_item_sizes = {512, 512, 512},
  .max_work_group_size = 512,
  .max_clock_frequency = 1000,
#include "cl_gen75_device.h"
};

LOCAL cl_device_id
cl_get_gt_device(void)
{
  cl_device_id ret = NULL;
  const int device_id = cl_driver_get_device_id();
  cl_device_id device = NULL;

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
      device->vendor_id = device_id;
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
      intel_ivb_gt1_device.vendor_id = device_id;
      intel_ivb_gt1_device.platform = cl_get_platform_default();
      ret = &intel_ivb_gt1_device;
      cl_intel_platform_get_default_extension(ret);
      break;

    case PCI_CHIP_IVYBRIDGE_GT2:
      DECL_INFO_STRING(ivb_gt2_break, intel_ivb_gt2_device, name, "Intel(R) HD Graphics IvyBridge GT2");
    case PCI_CHIP_IVYBRIDGE_M_GT2:
      DECL_INFO_STRING(ivb_gt2_break, intel_ivb_gt2_device, name, "Intel(R) HD Graphics IvyBridge M GT2");
    case PCI_CHIP_IVYBRIDGE_S_GT2:
      DECL_INFO_STRING(ivb_gt2_break, intel_ivb_gt2_device, name, "Intel(R) HD Graphics IvyBridge S GT2");
ivb_gt2_break:
      intel_ivb_gt2_device.vendor_id = device_id;
      intel_ivb_gt2_device.platform = cl_get_platform_default();
      ret = &intel_ivb_gt2_device;
      cl_intel_platform_get_default_extension(ret);
      break;

    case PCI_CHIP_BAYTRAIL_T:
      DECL_INFO_STRING(baytrail_t_device_break, intel_baytrail_t_device, name, "Intel(R) HD Graphics Bay Trail-T");
baytrail_t_device_break:
      intel_baytrail_t_device.vendor_id = device_id;
      intel_baytrail_t_device.platform = cl_get_platform_default();
      ret = &intel_baytrail_t_device;
      cl_intel_platform_get_default_extension(ret);
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
      intel_brw_gt1_device.vendor_id = device_id;
      intel_brw_gt1_device.platform = cl_get_platform_default();
      ret = &intel_brw_gt1_device;
      cl_intel_platform_enable_fp16_extension(ret);
      break;

    case PCI_CHIP_BROADWLL_M_GT2:
      DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell Mobile GT2");
    case PCI_CHIP_BROADWLL_D_GT2:
      DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell U-Processor GT2");
    case PCI_CHIP_BROADWLL_S_GT2:
      DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell Server GT2");
    case PCI_CHIP_BROADWLL_W_GT2:
      DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell Workstation GT2");
    case PCI_CHIP_BROADWLL_U_GT2:
      DECL_INFO_STRING(brw_gt2_break, intel_brw_gt2_device, name, "Intel(R) HD Graphics BroadWell ULX GT2");
brw_gt2_break:
      intel_brw_gt2_device.vendor_id = device_id;
      intel_brw_gt2_device.platform = cl_get_platform_default();
      ret = &intel_brw_gt2_device;
      cl_intel_platform_enable_fp16_extension(ret);
      break;

    case PCI_CHIP_BROADWLL_M_GT3:
      DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell Mobile GT3");
    case PCI_CHIP_BROADWLL_D_GT3:
      DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell U-Processor GT3");
    case PCI_CHIP_BROADWLL_S_GT3:
      DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell Server GT3");
    case PCI_CHIP_BROADWLL_W_GT3:
      DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell Workstation GT3");
    case PCI_CHIP_BROADWLL_U_GT3:
      DECL_INFO_STRING(brw_gt3_break, intel_brw_gt3_device, name, "Intel(R) HD Graphics BroadWell ULX GT3");
brw_gt3_break:
      intel_brw_gt3_device.vendor_id = device_id;
      intel_brw_gt3_device.platform = cl_get_platform_default();
      ret = &intel_brw_gt3_device;
      cl_intel_platform_enable_fp16_extension(ret);
      break;

    case PCI_CHIP_CHV_0:
    case PCI_CHIP_CHV_1:
    case PCI_CHIP_CHV_2:
    case PCI_CHIP_CHV_3:
      DECL_INFO_STRING(chv_break, intel_chv_device, name, "Intel(R) HD Graphics Cherryview");
chv_break:
      intel_chv_device.vendor_id = device_id;
      intel_chv_device.platform = cl_get_platform_default();
      ret = &intel_chv_device;
      cl_intel_platform_enable_fp16_extension(ret);
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
      intel_skl_gt1_device.vendor_id = device_id;
      intel_skl_gt1_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt1_device;
      cl_intel_platform_enable_fp16_extension(ret);
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
skl_gt2_break:
      intel_skl_gt2_device.vendor_id = device_id;
      intel_skl_gt2_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt2_device;
      cl_intel_platform_enable_fp16_extension(ret);
      break;

    case PCI_CHIP_SKYLAKE_ULT_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake ULT GT3");
    case PCI_CHIP_SKYLAKE_HALO_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Halo GT3");
    case PCI_CHIP_SKYLAKE_SRV_GT3:
      DECL_INFO_STRING(skl_gt3_break, intel_skl_gt3_device, name, "Intel(R) HD Graphics Skylake Server GT3");
skl_gt3_break:
      intel_skl_gt3_device.vendor_id = device_id;
      intel_skl_gt3_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt3_device;
      cl_intel_platform_enable_fp16_extension(ret);
      break;

    case PCI_CHIP_SKYLAKE_HALO_GT4:
      DECL_INFO_STRING(skl_gt4_break, intel_skl_gt4_device, name, "Intel(R) HD Graphics Skylake Halo GT4");
    case PCI_CHIP_SKYLAKE_SRV_GT4:
      DECL_INFO_STRING(skl_gt4_break, intel_skl_gt4_device, name, "Intel(R) HD Graphics Skylake Server GT4");
skl_gt4_break:
      intel_skl_gt4_device.vendor_id = device_id;
      intel_skl_gt4_device.platform = cl_get_platform_default();
      ret = &intel_skl_gt4_device;
      cl_intel_platform_enable_fp16_extension(ret);
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

  if (!CompilerSupported()) {
    ret->compiler_available = CL_FALSE;
    //ret->linker_available = CL_FALSE;
    ret->profile = "EMBEDDED_PROFILE";
    ret->profile_sz = strlen(ret->profile) + 1;
  }

  /* Apply any driver-dependent updates to the device info */
  cl_driver_update_device_info(ret);

  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    uint64_t two_gb = 2 * 1024 * 1024 * 1024ul; 
    uint64_t totalram = info.totalram * info.mem_unit;
    ret->global_mem_size = (totalram > two_gb) ? 
                            two_gb : info.totalram;
    ret->max_mem_alloc_size = ret->global_mem_size / 2;
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
  cl_driver_set_atomic_flag(ctx->drv, atomic_in_l3_flag);
  if (status == CL_SUCCESS) {
    queue = clCreateCommandQueue(ctx, device, 0, &status);
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
  device = cl_get_gt_device();
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

#define DECL_FIELD(CASE,FIELD)                                      \
  case JOIN(CL_DEVICE_,CASE):                                       \
    if (param_value_size_ret) {                                     \
      *param_value_size_ret = sizeof device->FIELD;                 \
      if (!param_value)                                             \
        return CL_SUCCESS;                                          \
    }                                                               \
    if (param_value_size < sizeof device->FIELD)                    \
      return CL_INVALID_VALUE;                                      \
    memcpy(param_value, &device->FIELD, sizeof device->FIELD);      \
    return CL_SUCCESS;

#define DECL_STRING_FIELD(CASE,FIELD)                               \
  case JOIN(CL_DEVICE_,CASE):                                       \
    if (param_value_size_ret) {                                     \
      *param_value_size_ret = device->JOIN(FIELD,_sz);              \
      if (!param_value)                                             \
        return CL_SUCCESS;                                          \
    }                                                               \
    if (param_value_size < device->JOIN(FIELD,_sz))                 \
      return CL_INVALID_VALUE;                                      \
    memcpy(param_value, device->FIELD, device->JOIN(FIELD,_sz));    \
    return CL_SUCCESS;

LOCAL cl_int
cl_get_device_info(cl_device_id     device,
                   cl_device_info   param_name,
                   size_t           param_value_size,
                   void *           param_value,
                   size_t *         param_value_size_ret)
{
  if (UNLIKELY(device != &intel_ivb_gt1_device &&
               device != &intel_ivb_gt2_device &&
               device != &intel_baytrail_t_device &&
               device != &intel_hsw_gt1_device &&
               device != &intel_hsw_gt2_device &&
               device != &intel_hsw_gt3_device &&
               device != &intel_brw_gt1_device &&
               device != &intel_brw_gt2_device &&
               device != &intel_brw_gt3_device &&
               device != &intel_chv_device &&
               device != &intel_skl_gt1_device &&
               device != &intel_skl_gt2_device &&
               device != &intel_skl_gt3_device &&
               device != &intel_skl_gt4_device
               ))
    return CL_INVALID_DEVICE;

  /* Find the correct parameter */
  switch (param_name) {
    DECL_FIELD(TYPE, device_type)
    DECL_FIELD(VENDOR_ID, vendor_id)
    DECL_FIELD(MAX_COMPUTE_UNITS, max_compute_unit)
    DECL_FIELD(MAX_WORK_ITEM_DIMENSIONS, max_work_item_dimensions)
    DECL_FIELD(MAX_WORK_ITEM_SIZES, max_work_item_sizes)
    DECL_FIELD(MAX_WORK_GROUP_SIZE, max_work_group_size)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_CHAR, preferred_vector_width_char)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_SHORT, preferred_vector_width_short)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_INT, preferred_vector_width_int)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_LONG, preferred_vector_width_long)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_FLOAT, preferred_vector_width_float)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_DOUBLE, preferred_vector_width_double)
    DECL_FIELD(PREFERRED_VECTOR_WIDTH_HALF, preferred_vector_width_half)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_CHAR, native_vector_width_char)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_SHORT, native_vector_width_short)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_INT, native_vector_width_int)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_LONG, native_vector_width_long)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_FLOAT, native_vector_width_float)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_DOUBLE, native_vector_width_double)
    DECL_FIELD(NATIVE_VECTOR_WIDTH_HALF, native_vector_width_half)
    DECL_FIELD(MAX_CLOCK_FREQUENCY, max_clock_frequency)
    DECL_FIELD(ADDRESS_BITS, address_bits)
    DECL_FIELD(MAX_MEM_ALLOC_SIZE, max_mem_alloc_size)
    DECL_FIELD(IMAGE_SUPPORT, image_support)
    DECL_FIELD(MAX_READ_IMAGE_ARGS, max_read_image_args)
    DECL_FIELD(MAX_WRITE_IMAGE_ARGS, max_write_image_args)
    DECL_FIELD(IMAGE_MAX_ARRAY_SIZE, image_max_array_size)
    DECL_FIELD(IMAGE2D_MAX_WIDTH, image2d_max_width)
    DECL_FIELD(IMAGE2D_MAX_HEIGHT, image2d_max_height)
    DECL_FIELD(IMAGE3D_MAX_WIDTH, image3d_max_width)
    DECL_FIELD(IMAGE3D_MAX_HEIGHT, image3d_max_height)
    DECL_FIELD(IMAGE3D_MAX_DEPTH, image3d_max_depth)
    DECL_FIELD(MAX_SAMPLERS, max_samplers)
    DECL_FIELD(MAX_PARAMETER_SIZE, max_parameter_size)
    DECL_FIELD(MEM_BASE_ADDR_ALIGN, mem_base_addr_align)
    DECL_FIELD(MIN_DATA_TYPE_ALIGN_SIZE, min_data_type_align_size)
    DECL_FIELD(SINGLE_FP_CONFIG, single_fp_config)
    DECL_FIELD(HALF_FP_CONFIG, half_fp_config)
    DECL_FIELD(DOUBLE_FP_CONFIG, double_fp_config)
    DECL_FIELD(GLOBAL_MEM_CACHE_TYPE, global_mem_cache_type)
    DECL_FIELD(GLOBAL_MEM_CACHELINE_SIZE, global_mem_cache_line_size)
    DECL_FIELD(GLOBAL_MEM_CACHE_SIZE, global_mem_cache_size)
    DECL_FIELD(GLOBAL_MEM_SIZE, global_mem_size)
    DECL_FIELD(MAX_CONSTANT_BUFFER_SIZE, max_constant_buffer_size)
    DECL_FIELD(IMAGE_MAX_BUFFER_SIZE, image_mem_size)
    DECL_FIELD(MAX_CONSTANT_ARGS, max_constant_args)
    DECL_FIELD(LOCAL_MEM_TYPE, local_mem_type)
    DECL_FIELD(LOCAL_MEM_SIZE, local_mem_size)
    DECL_FIELD(ERROR_CORRECTION_SUPPORT, error_correction_support)
    DECL_FIELD(HOST_UNIFIED_MEMORY, host_unified_memory)
    DECL_FIELD(PROFILING_TIMER_RESOLUTION, profiling_timer_resolution)
    DECL_FIELD(ENDIAN_LITTLE, endian_little)
    DECL_FIELD(AVAILABLE, available)
    DECL_FIELD(COMPILER_AVAILABLE, compiler_available)
    DECL_FIELD(LINKER_AVAILABLE, linker_available)
    DECL_FIELD(EXECUTION_CAPABILITIES, execution_capabilities)
    DECL_FIELD(QUEUE_PROPERTIES, queue_properties)
    DECL_FIELD(PLATFORM, platform)
    DECL_FIELD(PRINTF_BUFFER_SIZE, printf_buffer_size)
    DECL_FIELD(PREFERRED_INTEROP_USER_SYNC, interop_user_sync)
    DECL_STRING_FIELD(NAME, name)
    DECL_STRING_FIELD(VENDOR, vendor)
    DECL_STRING_FIELD(VERSION, version)
    DECL_STRING_FIELD(PROFILE, profile)
    DECL_STRING_FIELD(OPENCL_C_VERSION, opencl_c_version)
    DECL_STRING_FIELD(EXTENSIONS, extensions);
    DECL_STRING_FIELD(BUILT_IN_KERNELS, built_in_kernels)
    DECL_FIELD(PARENT_DEVICE, parent_device)
    DECL_FIELD(PARTITION_MAX_SUB_DEVICES, partition_max_sub_device)
    DECL_FIELD(PARTITION_PROPERTIES, partition_property)
    DECL_FIELD(PARTITION_AFFINITY_DOMAIN, affinity_domain)
    DECL_FIELD(PARTITION_TYPE, partition_type)
    DECL_FIELD(REFERENCE_COUNT, device_reference_count)

    case CL_DRIVER_VERSION:
      if (param_value_size_ret) {
        *param_value_size_ret = device->driver_version_sz;
        if (!param_value)
          return CL_SUCCESS;
      }
      if (param_value_size < device->driver_version_sz)
        return CL_INVALID_VALUE;
      memcpy(param_value, device->driver_version, device->driver_version_sz);
      return CL_SUCCESS;

    default: return CL_INVALID_VALUE;
  };
}

LOCAL cl_int
cl_device_get_version(cl_device_id device, cl_int *ver)
{
  if (UNLIKELY(device != &intel_ivb_gt1_device &&
               device != &intel_ivb_gt2_device &&
               device != &intel_baytrail_t_device &&
               device != &intel_hsw_gt1_device &&
               device != &intel_hsw_gt2_device &&
               device != &intel_hsw_gt3_device &&
               device != &intel_brw_gt1_device &&
               device != &intel_brw_gt2_device &&
               device != &intel_brw_gt3_device &&
               device != &intel_chv_device &&
               device != &intel_skl_gt1_device &&
               device != &intel_skl_gt2_device &&
               device != &intel_skl_gt3_device &&
               device != &intel_skl_gt4_device))
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
        || device == &intel_skl_gt3_device || device == &intel_skl_gt4_device) {
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
    if (!strstr(device->built_in_kernels, n)){
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
  int vendor_id = kernel->program->ctx->device->vendor_id;
  if (!interp_kernel_use_slm(kernel->opaque)) {
    if (!IS_BAYTRAIL_T(vendor_id) || simd_width == 16)
      work_group_size = simd_width * 64;
    else
      work_group_size = kernel->program->ctx->device->max_compute_unit *
                        kernel->program->ctx->device->max_thread_per_unit * simd_width;
  } else {
    thread_cnt = kernel->program->ctx->device->max_compute_unit *
                 kernel->program->ctx->device->max_thread_per_unit / kernel->program->ctx->device->sub_slice_count;
    if(thread_cnt > 64)
      thread_cnt = 64;
    work_group_size = thread_cnt * simd_width;
  }
  if(work_group_size > kernel->program->ctx->device->max_work_group_size)
    work_group_size = kernel->program->ctx->device->max_work_group_size;
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
  if (UNLIKELY(device != &intel_ivb_gt1_device &&
               device != &intel_ivb_gt2_device &&
               device != &intel_baytrail_t_device &&
               device != &intel_hsw_gt1_device &&
               device != &intel_hsw_gt2_device &&
               device != &intel_hsw_gt3_device &&
               device != &intel_brw_gt1_device &&
               device != &intel_brw_gt2_device &&
               device != &intel_brw_gt3_device &&
               device != &intel_chv_device &&
               device != &intel_skl_gt1_device &&
               device != &intel_skl_gt2_device &&
               device != &intel_skl_gt3_device &&
               device != &intel_skl_gt4_device))
    return CL_INVALID_DEVICE;

  CHECK_KERNEL(kernel);
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
    DECL_FIELD(PREFERRED_WORK_GROUP_SIZE_MULTIPLE, device->preferred_wg_sz_mul)
    case CL_KERNEL_LOCAL_MEM_SIZE:
    {
      size_t local_mem_sz =  interp_kernel_get_slm_size(kernel->opaque) + kernel->local_mem_sz;
      _DECL_FIELD(local_mem_sz)
    }
    DECL_FIELD(COMPILE_WORK_GROUP_SIZE, kernel->compile_wg_sz)
    DECL_FIELD(PRIVATE_MEM_SIZE, kernel->stack_size)
    case CL_KERNEL_GLOBAL_WORK_SIZE:
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
    default:
      return CL_INVALID_VALUE;
  };

error:
  return err;
}

