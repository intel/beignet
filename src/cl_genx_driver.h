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

#ifndef __CL_INTEL_DRIVER_H__
#define __CL_INTEL_DRIVER_H__

/* They are mostly wrapper around C++ delete / new to avoid c++ in c files */
struct intel_driver;

/* Allocate and initialize the gen driver */
extern struct intel_driver* cl_intel_driver_new(void);

/* Destroy and deallocate the gen driver */
extern void cl_intel_driver_delete(struct intel_driver*);

#endif /* __CL_INTEL_DRIVER_H__ */

