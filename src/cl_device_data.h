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

#ifndef __CL_DEVICE_DATA_H__
#define __CL_DEVICE_DATA_H__

#define INVALID_CHIP_ID -1 //returned by intel_get_device_id if no device found
#define INTEL_VENDOR_ID                 0x8086   // Vendor ID for Intel

#define PCI_CHIP_GM45_GM                0x2A42
#define PCI_CHIP_IGD_E_G                0x2E02
#define PCI_CHIP_Q45_G                  0x2E12
#define PCI_CHIP_G45_G                  0x2E22
#define PCI_CHIP_G41_G                  0x2E32

#define PCI_CHIP_IGDNG_D_G              0x0042
#define PCI_CHIP_IGDNG_M_G              0x0046

#define IS_G45(devid)           (devid == PCI_CHIP_IGD_E_G || \
    devid == PCI_CHIP_Q45_G || \
    devid == PCI_CHIP_G45_G || \
    devid == PCI_CHIP_G41_G)
#define IS_GM45(devid)          (devid == PCI_CHIP_GM45_GM)
#define IS_G4X(devid)    (IS_G45(devid) || IS_GM45(devid))

#define IS_IGDNG_D(devid)       (devid == PCI_CHIP_IGDNG_D_G)
#define IS_IGDNG_M(devid)       (devid == PCI_CHIP_IGDNG_M_G)
#define IS_IGDNG(devid)         (IS_IGDNG_D(devid) || IS_IGDNG_M(devid))

#ifndef PCI_CHIP_SANDYBRIDGE_BRIDGE
#define PCI_CHIP_SANDYBRIDGE_BRIDGE      0x0100  /* Desktop */
#define PCI_CHIP_SANDYBRIDGE_GT1         0x0102
#define PCI_CHIP_SANDYBRIDGE_GT2         0x0112
#define PCI_CHIP_SANDYBRIDGE_GT2_PLUS    0x0122
#define PCI_CHIP_SANDYBRIDGE_BRIDGE_M    0x0104  /* Mobile */
#define PCI_CHIP_SANDYBRIDGE_M_GT1       0x0106
#define PCI_CHIP_SANDYBRIDGE_M_GT2       0x0116
#define PCI_CHIP_SANDYBRIDGE_M_GT2_PLUS  0x0126
#define PCI_CHIP_SANDYBRIDGE_BRIDGE_S  0x0108  /* Server */
#define PCI_CHIP_SANDYBRIDGE_S_GT  0x010A
#endif

#define IS_GEN6(devid)                          \
   (devid == PCI_CHIP_SANDYBRIDGE_GT1 ||        \
    devid == PCI_CHIP_SANDYBRIDGE_GT2 ||        \
    devid == PCI_CHIP_SANDYBRIDGE_GT2_PLUS ||   \
    devid == PCI_CHIP_SANDYBRIDGE_M_GT1 ||      \
    devid == PCI_CHIP_SANDYBRIDGE_M_GT2 ||      \
    devid == PCI_CHIP_SANDYBRIDGE_M_GT2_PLUS || \
    devid == PCI_CHIP_SANDYBRIDGE_S_GT)

#define PCI_CHIP_IVYBRIDGE_GT1          0x0152  /* Desktop */
#define PCI_CHIP_IVYBRIDGE_GT2          0x0162
#define PCI_CHIP_IVYBRIDGE_M_GT1        0x0156  /* Mobile */
#define PCI_CHIP_IVYBRIDGE_M_GT2        0x0166
#define PCI_CHIP_IVYBRIDGE_S_GT1        0x015a  /* Server */
#define PCI_CHIP_IVYBRIDGE_S_GT2        0x016a

#define PCI_CHIP_BAYTRAIL_T 0x0F31

#define IS_IVB_GT1(devid)               \
  (devid == PCI_CHIP_IVYBRIDGE_GT1 ||   \
   devid == PCI_CHIP_IVYBRIDGE_M_GT1 || \
   devid == PCI_CHIP_IVYBRIDGE_S_GT1)

#define IS_IVB_GT2(devid)               \
  (devid == PCI_CHIP_IVYBRIDGE_GT2 ||   \
   devid == PCI_CHIP_IVYBRIDGE_M_GT2 || \
   devid == PCI_CHIP_IVYBRIDGE_S_GT2)

#define IS_BAYTRAIL_T(devid)              \
  (devid == PCI_CHIP_BAYTRAIL_T)

#define IS_IVYBRIDGE(devid) (IS_IVB_GT1(devid) || IS_IVB_GT2(devid) || IS_BAYTRAIL_T(devid))
#define IS_GEN7(devid)      IS_IVYBRIDGE(devid)


#define PCI_CHIP_HASWELL_D1          0x0402 /* GT1 desktop */
#define PCI_CHIP_HASWELL_D2          0x0412 /* GT2 desktop */
#define PCI_CHIP_HASWELL_D3          0x0422 /* GT3 desktop */
#define PCI_CHIP_HASWELL_S1          0x040a /* GT1 server */
#define PCI_CHIP_HASWELL_S2          0x041a /* GT2 server */
#define PCI_CHIP_HASWELL_S3          0x042a /* GT3 server */
#define PCI_CHIP_HASWELL_M1          0x0406 /* GT1 mobile */
#define PCI_CHIP_HASWELL_M2          0x0416 /* GT2 mobile */
#define PCI_CHIP_HASWELL_M3          0x0426 /* GT3 mobile */
#define PCI_CHIP_HASWELL_B1          0x040B /* Haswell GT1 */
#define PCI_CHIP_HASWELL_B2          0x041B /* Haswell GT2 */
#define PCI_CHIP_HASWELL_B3          0x042B /* Haswell GT3 */
#define PCI_CHIP_HASWELL_E1          0x040E /* Haswell GT1 */
#define PCI_CHIP_HASWELL_E2          0x041E /* Haswell GT2 */
#define PCI_CHIP_HASWELL_E3          0x042E /* Haswell GT3 */

/* Software Development Vehicle devices. */
#define PCI_CHIP_HASWELL_SDV_D1      0x0C02 /* SDV GT1 desktop */
#define PCI_CHIP_HASWELL_SDV_D2      0x0C12 /* SDV GT2 desktop */
#define PCI_CHIP_HASWELL_SDV_D3      0x0C22 /* SDV GT3 desktop */
#define PCI_CHIP_HASWELL_SDV_S1      0x0C0A /* SDV GT1 server */
#define PCI_CHIP_HASWELL_SDV_S2      0x0C1A /* SDV GT2 server */
#define PCI_CHIP_HASWELL_SDV_S3      0x0C2A /* SDV GT3 server */
#define PCI_CHIP_HASWELL_SDV_M1      0x0C06 /* SDV GT1 mobile */
#define PCI_CHIP_HASWELL_SDV_M2      0x0C16 /* SDV GT2 mobile */
#define PCI_CHIP_HASWELL_SDV_M3      0x0C26 /* SDV GT3 mobile */
#define PCI_CHIP_HASWELL_SDV_B1      0x0C0B /* SDV GT1 */
#define PCI_CHIP_HASWELL_SDV_B2      0x0C1B /* SDV GT2 */
#define PCI_CHIP_HASWELL_SDV_B3      0x0C2B /* SDV GT3 */
#define PCI_CHIP_HASWELL_SDV_E1      0x0C0E /* SDV GT1 */
#define PCI_CHIP_HASWELL_SDV_E2      0x0C1E /* SDV GT2 */
#define PCI_CHIP_HASWELL_SDV_E3      0x0C2E /* SDV GT3 */
/* Ultrabooks */
#define PCI_CHIP_HASWELL_ULT_D1      0x0A02 /* ULT GT1 desktop */
#define PCI_CHIP_HASWELL_ULT_D2      0x0A12 /* ULT GT2 desktop */
#define PCI_CHIP_HASWELL_ULT_D3      0x0A22 /* ULT GT3 desktop */
#define PCI_CHIP_HASWELL_ULT_S1      0x0A0A /* ULT GT1 server */
#define PCI_CHIP_HASWELL_ULT_S2      0x0A1A /* ULT GT2 server */
#define PCI_CHIP_HASWELL_ULT_S3      0x0A2A /* ULT GT3 server */
#define PCI_CHIP_HASWELL_ULT_M1      0x0A06 /* ULT GT1 mobile */
#define PCI_CHIP_HASWELL_ULT_M2      0x0A16 /* ULT GT2 mobile */
#define PCI_CHIP_HASWELL_ULT_M3      0x0A26 /* ULT GT3 mobile */
#define PCI_CHIP_HASWELL_ULT_B1      0x0A0B /* ULT GT1 */
#define PCI_CHIP_HASWELL_ULT_B2      0x0A1B /* ULT GT2 */
#define PCI_CHIP_HASWELL_ULT_B3      0x0A2B /* ULT GT3 */
#define PCI_CHIP_HASWELL_ULT_E1      0x0A0E /* ULT GT1 */
#define PCI_CHIP_HASWELL_ULT_E2      0x0A1E /* ULT GT2 */
#define PCI_CHIP_HASWELL_ULT_E3      0x0A2E /* ULT GT3 */
/* CRW */
#define PCI_CHIP_HASWELL_CRW_D1      0x0D02 /* CRW GT1 desktop */
#define PCI_CHIP_HASWELL_CRW_D2      0x0D12 /* CRW GT2 desktop */
#define PCI_CHIP_HASWELL_CRW_D3      0x0D22 /* CRW GT3 desktop */
#define PCI_CHIP_HASWELL_CRW_S1      0x0D0A /* CRW GT1 server */
#define PCI_CHIP_HASWELL_CRW_S2      0x0D1A /* CRW GT2 server */
#define PCI_CHIP_HASWELL_CRW_S3      0x0D2A /* CRW GT3 server */
#define PCI_CHIP_HASWELL_CRW_M1      0x0D06 /* CRW GT1 mobile */
#define PCI_CHIP_HASWELL_CRW_M2      0x0D16 /* CRW GT2 mobile */
#define PCI_CHIP_HASWELL_CRW_M3      0x0D26 /* CRW GT3 mobile */
#define PCI_CHIP_HASWELL_CRW_B1      0x0D0B /* CRW GT1 */
#define PCI_CHIP_HASWELL_CRW_B2      0x0D1B /* CRW GT2 */
#define PCI_CHIP_HASWELL_CRW_B3      0x0D2B /* CRW GT3 */
#define PCI_CHIP_HASWELL_CRW_E1      0x0D0E /* CRW GT1 */
#define PCI_CHIP_HASWELL_CRW_E2      0x0D1E /* CRW GT2 */
#define PCI_CHIP_HASWELL_CRW_E3      0x0D2E /* CRW GT3 */

#define IS_HASWELL(devid) (  \
	(devid) == PCI_CHIP_HASWELL_D1 || (devid) == PCI_CHIP_HASWELL_D2 || \
	(devid) == PCI_CHIP_HASWELL_D3 || (devid) == PCI_CHIP_HASWELL_S1 || \
	(devid) == PCI_CHIP_HASWELL_S2 || (devid) == PCI_CHIP_HASWELL_S3 || \
	(devid) == PCI_CHIP_HASWELL_M1 || (devid) == PCI_CHIP_HASWELL_M2 || \
	(devid) == PCI_CHIP_HASWELL_M3 || (devid) == PCI_CHIP_HASWELL_B1 || \
	(devid) == PCI_CHIP_HASWELL_B2 || (devid) == PCI_CHIP_HASWELL_B3 || \
	(devid) == PCI_CHIP_HASWELL_E1 || (devid) == PCI_CHIP_HASWELL_E2 || \
	(devid) == PCI_CHIP_HASWELL_E3 || (devid) == PCI_CHIP_HASWELL_SDV_D1 || \
	(devid) == PCI_CHIP_HASWELL_SDV_D2 || (devid) == PCI_CHIP_HASWELL_SDV_D3 || \
	(devid) == PCI_CHIP_HASWELL_SDV_S1 || (devid) == PCI_CHIP_HASWELL_SDV_S2 || \
	(devid) == PCI_CHIP_HASWELL_SDV_S3 || (devid) == PCI_CHIP_HASWELL_SDV_M1 || \
	(devid) == PCI_CHIP_HASWELL_SDV_M2 || (devid) == PCI_CHIP_HASWELL_SDV_M3 || \
	(devid) == PCI_CHIP_HASWELL_SDV_B1 || (devid) == PCI_CHIP_HASWELL_SDV_B2 || \
	(devid) == PCI_CHIP_HASWELL_SDV_B3 || (devid) == PCI_CHIP_HASWELL_SDV_E1 || \
	(devid) == PCI_CHIP_HASWELL_SDV_E2 || (devid) == PCI_CHIP_HASWELL_SDV_E3 || \
	(devid) == PCI_CHIP_HASWELL_ULT_D1 || (devid) == PCI_CHIP_HASWELL_ULT_D2 || \
	(devid) == PCI_CHIP_HASWELL_ULT_D3 || (devid) == PCI_CHIP_HASWELL_ULT_S1 || \
	(devid) == PCI_CHIP_HASWELL_ULT_S2 || (devid) == PCI_CHIP_HASWELL_ULT_S3 || \
	(devid) == PCI_CHIP_HASWELL_ULT_M1 || (devid) == PCI_CHIP_HASWELL_ULT_M2 || \
	(devid) == PCI_CHIP_HASWELL_ULT_M3 || (devid) == PCI_CHIP_HASWELL_ULT_B1 || \
	(devid) == PCI_CHIP_HASWELL_ULT_B2 || (devid) == PCI_CHIP_HASWELL_ULT_B3 || \
	(devid) == PCI_CHIP_HASWELL_ULT_E1 || (devid) == PCI_CHIP_HASWELL_ULT_E2 || \
	(devid) == PCI_CHIP_HASWELL_ULT_E3 || (devid) == PCI_CHIP_HASWELL_CRW_D1 || \
	(devid) == PCI_CHIP_HASWELL_CRW_D2 || (devid) == PCI_CHIP_HASWELL_CRW_D3 || \
	(devid) == PCI_CHIP_HASWELL_CRW_S1 || (devid) == PCI_CHIP_HASWELL_CRW_S2 || \
	(devid) == PCI_CHIP_HASWELL_CRW_S3 || (devid) == PCI_CHIP_HASWELL_CRW_M1 || \
	(devid) == PCI_CHIP_HASWELL_CRW_M2 || (devid) == PCI_CHIP_HASWELL_CRW_M3 || \
	(devid) == PCI_CHIP_HASWELL_CRW_B1 || (devid) == PCI_CHIP_HASWELL_CRW_B2 || \
	(devid) == PCI_CHIP_HASWELL_CRW_B3 || (devid) == PCI_CHIP_HASWELL_CRW_E1 || \
	(devid) == PCI_CHIP_HASWELL_CRW_E2 || (devid) == PCI_CHIP_HASWELL_CRW_E3)

#define IS_GEN75(devid)  IS_HASWELL(devid)

/* BRW */
#define PCI_CHIP_BROADWLL_M_GT1       0x1602 /* Intel(R) Broadwell Mobile - Halo (EDRAM) - GT1 */
#define PCI_CHIP_BROADWLL_D_GT1       0x1606 /* Intel(R) Broadwell U-Processor - GT1 */
#define PCI_CHIP_BROADWLL_S_GT1       0x160A /* Intel(R) Broadwell Server - GT1 */
#define PCI_CHIP_BROADWLL_W_GT1       0x160D /* Intel(R) Broadwell Workstation - GT1 */
#define PCI_CHIP_BROADWLL_U_GT1       0x160E /* Intel(R) Broadwell ULX - GT1 */
#define PCI_CHIP_BROADWLL_M_GT2       0x1612 /* Intel(R) Broadwell Mobile - Halo (EDRAM) - GT2 */
#define PCI_CHIP_BROADWLL_D_GT2       0x1616 /* Intel(R) Broadwell U-Processor - GT2 */
#define PCI_CHIP_BROADWLL_S_GT2       0x161A /* Intel(R) Broadwell Server - GT2 */
#define PCI_CHIP_BROADWLL_W_GT2       0x161D /* Intel(R) Broadwell Workstation - GT2 */
#define PCI_CHIP_BROADWLL_U_GT2       0x161E /* Intel(R) Broadwell ULX - GT2 */
#define PCI_CHIP_BROADWLL_M_GT3       0x1622 /* Intel(R) Broadwell Mobile - Halo (EDRAM) - GT3 */
#define PCI_CHIP_BROADWLL_D_GT3       0x1626 /* Intel(R) Broadwell U-Processor HD 6000 - GT3 */
#define PCI_CHIP_BROADWLL_UI_GT3      0x162B /* Intel(R) Broadwell U-Process Iris 6100 - GT3 */
#define PCI_CHIP_BROADWLL_S_GT3       0x162A /* Intel(R) Broadwell Server - GT3 */
#define PCI_CHIP_BROADWLL_W_GT3       0x162D /* Intel(R) Broadwell Workstation - GT3 */
#define PCI_CHIP_BROADWLL_U_GT3       0x162E /* Intel(R) Broadwell ULX - GT3 */

#define IS_BRW_GT1(devid)               \
  (devid == PCI_CHIP_BROADWLL_M_GT1 ||   \
   devid == PCI_CHIP_BROADWLL_D_GT1 || \
   devid == PCI_CHIP_BROADWLL_S_GT1 || \
   devid == PCI_CHIP_BROADWLL_W_GT1 || \
   devid == PCI_CHIP_BROADWLL_U_GT1)

#define IS_BRW_GT2(devid)               \
  (devid == PCI_CHIP_BROADWLL_M_GT2 ||   \
   devid == PCI_CHIP_BROADWLL_D_GT2 || \
   devid == PCI_CHIP_BROADWLL_S_GT2 || \
   devid == PCI_CHIP_BROADWLL_W_GT2 || \
   devid == PCI_CHIP_BROADWLL_U_GT2)

#define IS_BRW_GT3(devid)               \
  (devid == PCI_CHIP_BROADWLL_M_GT3 ||   \
   devid == PCI_CHIP_BROADWLL_D_GT3 || \
   devid == PCI_CHIP_BROADWLL_S_GT3 || \
   devid == PCI_CHIP_BROADWLL_W_GT3 || \
   devid == PCI_CHIP_BROADWLL_UI_GT3 || \
   devid == PCI_CHIP_BROADWLL_U_GT3)

#define IS_BROADWELL(devid) (IS_BRW_GT1(devid) || IS_BRW_GT2(devid) || IS_BRW_GT3(devid))

#define PCI_CHIP_CHV_0 0x22B0
#define PCI_CHIP_CHV_1 0x22B1
#define PCI_CHIP_CHV_2 0x22B2
#define PCI_CHIP_CHV_3 0x22B3
#define IS_CHERRYVIEW(devid) \
  (devid == PCI_CHIP_CHV_0 ||   \
   devid == PCI_CHIP_CHV_1 || \
   devid == PCI_CHIP_CHV_2 || \
   devid == PCI_CHIP_CHV_3)

#define IS_GEN8(devid)      (IS_BROADWELL(devid) || IS_CHERRYVIEW(devid))

/* SKL */
#define PCI_CHIP_SKYLAKE_ULT_GT1	0x1906   /* Intel(R) Skylake ULT - GT1 */
#define PCI_CHIP_SKYLAKE_ULT_GT2	0x1916   /* Intel(R) Skylake ULT - GT2 */
#define PCI_CHIP_SKYLAKE_ULT_GT3	0x1923   /* Intel(R) Skylake ULT - GT3 */
#define PCI_CHIP_SKYLAKE_ULT_GT3E1	0x1926   /* Intel(R) Skylake ULT - GT3E */
#define PCI_CHIP_SKYLAKE_ULT_GT3E2	0x1927   /* Intel(R) Skylake ULT - GT3E */
#define PCI_CHIP_SKYLAKE_ULT_GT2F	0x1921   /* Intel(R) Skylake ULT - GT2F */
#define PCI_CHIP_SKYLAKE_ULX_GT1	0x190E   /* Intel(R) Skylake ULX - GT1 */
#define PCI_CHIP_SKYLAKE_ULX_GT2	0x191E   /* Intel(R) Skylake ULX - GT2 */
#define PCI_CHIP_SKYLAKE_DT_GT1		0x1902   /* Intel(R) Skylake Desktop - GT1 */
#define PCI_CHIP_SKYLAKE_DT_GT2		0x1912   /* Intel(R) Skylake Desktop - GT2 */
#define PCI_CHIP_SKYLAKE_DT_GT4   0x1932   /* Intel(R) Skylake Desktop - GT4 */
#define PCI_CHIP_SKYLAKE_HALO_GT1 	0x190B   /* Intel(R) Skylake HALO - GT1 */
#define PCI_CHIP_SKYLAKE_HALO_GT2	0x191B   /* Intel(R) Skylake HALO - GT2 */
#define PCI_CHIP_SKYLAKE_HALO_GT3	0x192B   /* Intel(R) Skylake HALO - GT3 */
#define PCI_CHIP_SKYLAKE_HALO_GT4	0x193B   /* Intel(R) Skylake HALO - GT4 */
#define PCI_CHIP_SKYLAKE_SRV_GT1	0x190A   /* Intel(R) Skylake Server - GT1 */
#define PCI_CHIP_SKYLAKE_SRV_GT2	0x191A   /* Intel(R) Skylake Server - GT2 */
#define PCI_CHIP_SKYLAKE_SRV_GT3	0x192A   /* Intel(R) Skylake Server - GT3 */
#define PCI_CHIP_SKYLAKE_SRV_GT4	0x193A   /* Intel(R) Skylake Server - GT4 */
#define PCI_CHIP_SKYLAKE_WKS_GT2  0x191D   /* Intel(R) Skylake WKS - GT2 */
#define PCI_CHIP_SKYLAKE_MEDIA_SRV_GT3  0x192D /* Intel(R) Skylake Media Server - GT3 */
#define PCI_CHIP_SKYLAKE_WKS_GT4  0x193D   /* Intel(R) Skylake WKS - GT4 */

#define IS_SKL_GT1(devid)               \
  (devid == PCI_CHIP_SKYLAKE_ULT_GT1 ||   \
   devid == PCI_CHIP_SKYLAKE_ULX_GT1 || \
   devid == PCI_CHIP_SKYLAKE_DT_GT1 || \
   devid == PCI_CHIP_SKYLAKE_HALO_GT1 || \
   devid == PCI_CHIP_SKYLAKE_SRV_GT1)

#define IS_SKL_GT2(devid)               \
  (devid == PCI_CHIP_SKYLAKE_ULT_GT2 ||   \
   devid == PCI_CHIP_SKYLAKE_ULT_GT2F ||   \
   devid == PCI_CHIP_SKYLAKE_ULX_GT2 || \
   devid == PCI_CHIP_SKYLAKE_DT_GT2 || \
   devid == PCI_CHIP_SKYLAKE_HALO_GT2 || \
   devid == PCI_CHIP_SKYLAKE_SRV_GT2 || \
   devid == PCI_CHIP_SKYLAKE_WKS_GT2)

#define IS_SKL_GT3(devid)               \
  (devid == PCI_CHIP_SKYLAKE_ULT_GT3 ||   \
   devid == PCI_CHIP_SKYLAKE_ULT_GT3E1 ||   \
   devid == PCI_CHIP_SKYLAKE_ULT_GT3E2 ||   \
   devid == PCI_CHIP_SKYLAKE_HALO_GT3 || \
   devid == PCI_CHIP_SKYLAKE_SRV_GT3 || \
   devid == PCI_CHIP_SKYLAKE_MEDIA_SRV_GT3)

#define IS_SKL_GT4(devid)               \
  (devid == PCI_CHIP_SKYLAKE_DT_GT4 ||  \
   devid == PCI_CHIP_SKYLAKE_HALO_GT4 || \
   devid == PCI_CHIP_SKYLAKE_SRV_GT4 || \
   devid == PCI_CHIP_SKYLAKE_WKS_GT4)

#define IS_SKYLAKE(devid) (IS_SKL_GT1(devid) || IS_SKL_GT2(devid) || IS_SKL_GT3(devid) || IS_SKL_GT4(devid))

/* BXT */
#define PCI_CHIP_BROXTON_0	0x5A84
#define PCI_CHIP_BROXTON_1	0x5A85
#define PCI_CHIP_BROXTON_2	0x1A84
#define PCI_CHIP_BROXTON_3	0x1A85

#define IS_BROXTON(devid)               \
  (devid == PCI_CHIP_BROXTON_0 ||       \
   devid == PCI_CHIP_BROXTON_1 ||       \
   devid == PCI_CHIP_BROXTON_2 ||       \
   devid == PCI_CHIP_BROXTON_3)

#define PCI_CHIP_KABYLAKE_ULT_GT1     0x5906
#define PCI_CHIP_KABYLAKE_ULT_GT2     0x5916
#define PCI_CHIP_KABYLAKE_ULT_GT3     0x5926
#define PCI_CHIP_KABYLAKE_ULT_GT15    0x5913
#define PCI_CHIP_KABYLAKE_ULT_GT2_1   0x5921
#define PCI_CHIP_KABYLAKE_ULT_GT3_1   0x5923
#define PCI_CHIP_KABYLAKE_ULT_GT3_2   0x5927
#define PCI_CHIP_KABYLAKE_DT_GT1      0x5902
#define PCI_CHIP_KABYLAKE_DT_GT2      0x5912
#define PCI_CHIP_KABYLAKE_DT_GT15     0x5917
#define PCI_CHIP_KABYLAKE_HALO_GT1    0x590B
#define PCI_CHIP_KABYLAKE_HALO_GT2    0x591B
#define PCI_CHIP_KABYLAKE_HALO_GT4    0x593B
#define PCI_CHIP_KABYLAKE_HALO_GT15   0x5908
#define PCI_CHIP_KABYLAKE_ULX_GT1     0x590E
#define PCI_CHIP_KABYLAKE_ULX_GT2     0x591E
#define PCI_CHIP_KABYLAKE_ULX_GT15    0x5915
#define PCI_CHIP_KABYLAKE_SRV_GT1     0x590A
#define PCI_CHIP_KABYLAKE_SRV_GT2     0x591A
#define PCI_CHIP_KABYLAKE_WKS_GT2     0x591D

#define IS_KBL_GT1(devid)                 \
  (devid == PCI_CHIP_KABYLAKE_ULT_GT1 ||  \
   devid == PCI_CHIP_KABYLAKE_DT_GT1 ||   \
   devid == PCI_CHIP_KABYLAKE_HALO_GT1 || \
   devid == PCI_CHIP_KABYLAKE_ULX_GT1 ||  \
   devid == PCI_CHIP_KABYLAKE_SRV_GT1)

#define IS_KBL_GT15(devid)                 \
  (devid == PCI_CHIP_KABYLAKE_ULT_GT15 ||  \
   devid == PCI_CHIP_KABYLAKE_DT_GT15 ||   \
   devid == PCI_CHIP_KABYLAKE_HALO_GT15 || \
   devid == PCI_CHIP_KABYLAKE_ULX_GT15)

#define IS_KBL_GT2(devid)                  \
  (devid == PCI_CHIP_KABYLAKE_ULT_GT2 ||   \
   devid == PCI_CHIP_KABYLAKE_ULT_GT2_1 || \
   devid == PCI_CHIP_KABYLAKE_DT_GT2 ||    \
   devid == PCI_CHIP_KABYLAKE_HALO_GT2 ||  \
   devid == PCI_CHIP_KABYLAKE_ULX_GT2 ||   \
   devid == PCI_CHIP_KABYLAKE_SRV_GT2 ||   \
   devid == PCI_CHIP_KABYLAKE_WKS_GT2)

#define IS_KBL_GT3(devid)                  \
  (devid == PCI_CHIP_KABYLAKE_ULT_GT3 ||   \
   devid == PCI_CHIP_KABYLAKE_ULT_GT3_1 || \
   devid == PCI_CHIP_KABYLAKE_ULT_GT3_2)

#define IS_KBL_GT4(devid)               \
  (devid == PCI_CHIP_KABYLAKE_HALO_GT4)

#define IS_KABYLAKE(devid) (IS_KBL_GT1(devid) || IS_KBL_GT15(devid) || IS_KBL_GT2(devid) || IS_KBL_GT3(devid) || IS_KBL_GT4(devid))

#define PCI_CHIP_GLK_3x6     0x3184
#define PCI_CHIP_GLK_2x6     0x3185

#define IS_GEMINILAKE(devid)      \
  (devid == PCI_CHIP_GLK_3x6 ||   \
   devid == PCI_CHIP_GLK_2x6)

#define IS_GEN9(devid)     (IS_SKYLAKE(devid) || IS_BROXTON(devid) || IS_KABYLAKE(devid) || IS_GEMINILAKE(devid))

#define MAX_OCLVERSION(devid) (IS_GEN9(devid) ? 200 : 120)

#endif /* __CL_DEVICE_DATA_H__ */

