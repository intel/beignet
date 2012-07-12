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

#ifndef __SIM_SIMULATOR_H__
#define __SIM_SIMULATOR_H__

#include "sim/simulator.h"

/* Allocate and initialize a new Gen simulator that run the c++ backend code */
extern gbe_simulator sim_simulator_new(void);
/* Destroy a Gen simulator */
extern void sim_simulator_delete(gbe_simulator);

#endif /* __SIM_GEN_SIMULATOR_H__ */

