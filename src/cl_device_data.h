/* 
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#define IS_IVB_GT1(devid)               \
  (devid == PCI_CHIP_IVYBRIDGE_GT1 ||   \
   devid == PCI_CHIP_IVYBRIDGE_M_GT1 || \
   devid == PCI_CHIP_IVYBRIDGE_S_GT1)

#define IS_IVB_GT2(devid)               \
  (devid == PCI_CHIP_IVYBRIDGE_GT2 ||   \
   devid == PCI_CHIP_IVYBRIDGE_M_GT2)

#define IS_IVYBRIDGE(devid) (IS_IVB_GT1(devid) || IS_IVB_GT2(devid))
#define IS_GEN7(devid)      IS_IVYBRIDGE(devid)

#define PCI_CHIP_HASWELL_M0          0x0094
#define PCI_CHIP_HASWELL_D0          0x0090
#define PCI_CHIP_HASWELL_M           0x0091
#define PCI_CHIP_HASWELL_L           0x0092

#define IS_HASWELL(devid) ((devid) == PCI_CHIP_HASWELL_M0 || \
                           (devid) == PCI_CHIP_HASWELL_D0 || \
                           (devid) == PCI_CHIP_HASWELL_M  || \
                           (devid) == PCI_CHIP_HASWELL_L)
#define IS_GEN75(devid)  IS_HASWELL(devid)

#endif /* __CL_DEVICE_DATA_H__ */

