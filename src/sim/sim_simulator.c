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

#include "sim/sim_simulator.h"
#include "cl_utils.h"
#include "cl_alloc.h"
#include <stdlib.h>

/* Implement of the simulator interface */
struct _sim_simulator {
  struct _gbe_simulator internal; /* Contains the call backs */
  void *base_address;             /* Base address of the fake address space */
  void *curbe_address;            /* Curbe address */
  size_t curbe_size;              /* Curbe size */
};
typedef struct _sim_simulator *sim_simulator;

static void *sim_get_base_address(sim_simulator sim) {
  return sim->base_address;
}
static void sim_set_base_address(sim_simulator sim, void *addr) {
  sim->base_address = addr;
}
static void *sim_get_curbe_address(sim_simulator sim) {
  return sim->curbe_address;
}
static void sim_set_curbe_address(sim_simulator sim, void *addr) {
  sim->curbe_address = addr;
}
static size_t sim_get_curbe_size(sim_simulator sim) {
  return sim->curbe_size;
}
static void sim_set_curbe_size(sim_simulator sim, size_t sz) {
  sim->curbe_size = sz;
}

LOCAL void
sim_simulator_delete(gbe_simulator sim) {
  if (UNLIKELY(sim == NULL)) return;
  cl_free(sim);
}

LOCAL gbe_simulator
sim_simulator_new(void)
{
  sim_simulator sim;
  TRY_ALLOC_NO_ERR(sim, cl_calloc(1, sizeof(struct _sim_simulator)));
  sim->internal.get_base_address = (sim_get_base_address_cb*) sim_get_base_address;
  sim->internal.set_base_address = (sim_set_base_address_cb*) sim_set_base_address;
  sim->internal.get_curbe_address = (sim_get_curbe_address_cb*) sim_get_curbe_address;
  sim->internal.set_curbe_address = (sim_set_curbe_address_cb*) sim_set_curbe_address;
  sim->internal.get_curbe_size = (sim_get_curbe_size_cb*) sim_get_curbe_size;
  sim->internal.set_curbe_size = (sim_set_curbe_size_cb*) sim_set_curbe_size;

exit:
  return (gbe_simulator) sim;
error:
  sim_simulator_delete((gbe_simulator) sim);
  sim = NULL;
  goto exit;
}

