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

#ifndef _GENX_DRIVER_H_
#define _GENX_DRIVER_H_

#include "intel_driver.h"
#include <stdint.h>

/* Provides more functionnalitites to setup gen state while using X */
typedef struct genx_driver
{
  intel_driver_t intel;
  Display *x11_display;
  struct dri_state *dri_ctx;
} genx_driver_t;

extern void genx_driver_init(genx_driver_t*);
extern void genx_driver_terminate(genx_driver_t*);

#endif /* _GENX_DRIVER_H_ */

