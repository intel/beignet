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

/**
 * \file simulator.h
 * \author Benjamin Segovia <benjamin.segovia@intel.com>
 *
 * C interface for the gen simulator
 */

#ifndef __GBE_SIMULATOR_H__
#define __GBE_SIMULATOR_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Gen simulator that runs the c++ produced by the back-end */
typedef struct _gbe_simulator *gbe_simulator;

/* Return the base address of the global / constant memory space */
typedef void *(sim_get_base_address_cb)(gbe_simulator);
/* Set the base address of the global / constant memory space */
typedef void (sim_set_base_address_cb)(gbe_simulator, void*);
/* Set the base address of the constant buffer */
typedef void *(sim_get_curbe_address_cb)(gbe_simulator);
/* Set the base address of the global / constant memory space */
typedef void (sim_set_curbe_address_cb)(gbe_simulator, void*);
struct _gbe_simulator {
  sim_set_base_address_cb *set_base_address;
  sim_get_base_address_cb *get_base_address;
  sim_set_curbe_address_cb *set_curbe_address;
  sim_get_curbe_address_cb *get_curbe_address;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GBE_SIMULATOR_H__ */

